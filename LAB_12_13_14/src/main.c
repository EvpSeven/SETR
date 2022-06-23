/** \file main.c
 *  \brief Program implementing an application that controls the light intensity of a given region.
 * 
 *  The system implements a system to control the light intensity of a region. It operates in tow modes,
 * Manual and Automatic. On the Manual mode the system receives the light intensity by the buttons of the board,
 * one increases (button 3) and the other decreases (button 4). On the Automatic mode the system controls the light
 * intensity with a PI controller. A light sensor is sampled, the samples are filtered and the real light intensity
 * is computed and the controller is done. On the Automatic mode the light intensity to keep is given by the user
 * that sets the time (week day, hour and minute) and light intensity in the memory. To implement this function the
 * system as thread that implements a calendar (week da, hours and minutes). The modes of operation can be set by
 * the board buttons, button 1 sets the Automatic  mode and button 2 the manual mode.
 *  It was implemented recuuring to threads, shared-memory and semaphores.
 *  It was implemented using the board Nordic nrf52840-dk.
 * 
 *  \author André Brandão
 *  \author Emanuel Pereira
 *  \date 18/06/2022
 */

#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <drivers/pwm.h>
#include <sys/printk.h>
#include <sys/__assert.h>
#include <string.h>
#include <timing/timing.h>
#include <stdlib.h>
#include <stdio.h>
#include <console/console.h>

#include <ADC.h>
#include <PI_controller.h>

#define SAMP_PERIOD_MS  250    /**< Sample period (ms) */
#define TIMER_PERIOD_MS 60000  /**< Timer (calendar) thread period (ms) - 1 minute */

#define MANUAL 0    /**< Flag that indicates Manual mode is selected */ 
#define AUTOMATIC 1 /**< Flag that indicates Automatic mode is selected */ 

#define FILTER_SIZE 10  /**< Window Size of samples (digital filter) */
#define MEM_SIZE 10     /**< Schedule Memory Size */

#define STACK_SIZE 1024 /**< Size of stack area used by each thread */
    
// Address of Board buttons
#define BOARDBUT1 0xb   /**< Address of Board button 1 used to turn Manual mode ON */
#define BOARDBUT2 0xc   /**< Address of Board button 2 used to turn Manual mode OFF */
#define BOARDBUT3 0x18  /**< Address of Board button 3 used to increase light intensity on manual mode */
#define BOARDBUT4 0x19  /**< Address of Board button 4 used to decrease light intensity on manual mode */

#define thread_sampling_prio 1      /**< Scheduling priority of sampling thread */
#define thread_processing_prio 1    /**< Scheduling priority of processing thread */
#define thread_actuation_prio 1     /**< Scheduling priority of actuation thread */
#define thread_timer_prio 1         /**< Scheduling priority of timer thread */
#define thread_interface_prio 1     /**< Scheduling priority of interface thread */

#define GPIO0_NID DT_NODELABEL(gpio0)   /**< gpio0 Node Label from device tree (refer to dts file) */
#define PWM0_NID DT_NODELABEL(pwm0)     /**< pwm0 Node Label from device tree (refer to dts file) */

#define PWM_PIN 0x0e    /**< Address of the pin used to output pwm */ 

K_THREAD_STACK_DEFINE(thread_sampling_stack, STACK_SIZE);   /**< Create sampling thread stack space */
K_THREAD_STACK_DEFINE(thread_processing_stack, STACK_SIZE); /**< Create processing thread stack space */
K_THREAD_STACK_DEFINE(thread_actuation_stack, STACK_SIZE);  /**< Create actuation thread stack space */
K_THREAD_STACK_DEFINE(thread_timer_stack, STACK_SIZE);      /**< Create timer (calendar) thread stack space */
K_THREAD_STACK_DEFINE(thread_interface_stack, STACK_SIZE);  /**< Create interface thread stack space */

struct k_thread thread_sampling_data;   /**< Sampling thread data */
struct k_thread thread_processing_data; /**< Processing thread data */
struct k_thread thread_actuation_data;  /**< Actuation thread data */
struct k_thread thread_timer_data;      /**< Timer (calendar) thread data */
struct k_thread thread_interface_data;  /**< Interface thread data */

k_tid_t thread_sampling_tid;    /**< Sampling thread task ID */
k_tid_t thread_processing_tid;  /**< Processing thread task ID */
k_tid_t thread_actuation_tid;   /**< Actuation thread task ID */
k_tid_t thread_timer_tid;       /**< Timer thread task ID */
k_tid_t thread_interface_tid;   /**< Interface thread task ID */

static struct gpio_callback button_cb_data; /**< Buttons callback structures */

/** Circular array to store data */
typedef struct {
    int data[FILTER_SIZE];  /**< Array to store data*/
    int head;   /**< Index of next position to store data */   
}buffer;

/** Memory struct to store user data */
typedef struct {
    int week_day;   /**< Week day (0-Sunday .... 6-Saturday) */
    int hour;   /**< Hour */
    int minute; /**< Minutes */
    int intensity;  /**< Light intensity */
}memory;

/** Calendar struct */
typedef struct {
    int day;    /**< Week day (0-Sunday .... 6-Saturday) */
    int hour;   /**< Hour */
    int minute; /**< Minutes */
}Calendar;


// Global variables (shared memory) to communicate between tasks
buffer sample_buffer;   /**< Buffer to store samples to communicate between tasks*/ 

int mode = MANUAL;  /**< System operation mode (MANUAL or AUTOMATIC) */
int intensity = 0;  /**< Light intensity */ 
int dutycycle = 0;  /**< PWM dutycycle */

memory mem[MEM_SIZE];   /**< Memory to store user data */ 
int mem_idx;    /**< Memory index to store data */

Calendar calendar;  /**< Calendar */

/** Data to print week days - index 0 -> Sunday .... index 6 -> Saturday */
static char *week_days[7] = {"Domingo", "Segunda-feira", "Terça-feira", "Quarta-Feira", "Quinta-Feira",
                             "Sexta-Feira", "Sábado"};

// Semaphores for task synch
struct k_sem sem_adc;   /**< Semaphore to synch sample and actuation tasks (signals end of sampling)*/
struct k_sem sem_act;   /**< Semaphore to trigger actuation thread */
struct k_sem sem_mut;   /**< Semaphore to mutual exclusion on calendar */

// Thread code prototypes
void thread_sampling(void *argA, void *argB, void *argC);
void thread_processing(void *argA, void *argB, void *argC);
void thread_actuation(void *argA, void *argB, void *argC);
void thread_timer(void *argA, void *argB, void *argC);
void thread_interface(void *argA, void *argB, void *argC);

// Functions prototypes
int read_int();
int filter(int*);
void array_init(int*, int);
int array_average(int*, int);
void input_output_config(void);

/** \brief Callback function of the interrupt from the four board buttons
 * 
 *  Interrupt function of four buttons that allow to control the mode of the
 * system operation (MANUAL or AUTOMATIC) and to increase or decrease the light
 * intensity when on manual mode.
 */
void buttons_cbfunction(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{    
    if(BIT(BOARDBUT1) & pins)   // Button 1 - change mode to automatic
    {
        mode = AUTOMATIC;
        printk("\nChanged to Automatic mode\n");
    }

    if(BIT(BOARDBUT2) & pins)   // Button 2 - change mode to manual
    {
        mode = MANUAL;
        printk("\nChanged to Manual mode\n");
    }

    if((BIT(BOARDBUT3) & pins) && mode == MANUAL)   // Button 3 - increase light intensity (only on manual mode)
    {
        intensity++;    // increase light intensity

        if(intensity > 100) // light intensity upper limit
            intensity = 100;
            
        printk("intensity = %d\n", intensity);

        k_sem_give(&sem_act);  // Trigger actuation thread to update PWM dutycycle
    }

    if((BIT(BOARDBUT4) & pins) && mode == MANUAL)   // Button 4 - decrease light intensity (only on manual mode)
    {
        intensity--;    // decrease light intensity

         if(intensity < 0)  // light intensity lower limit
            intensity = 0;

        printk("intensity = %d\n", intensity);

        k_sem_give(&sem_act);  // Trigger actuation thread to update PWM dutycycle
    }
}

/** \brief Main function
 * 
 *  The main function initializes the semaphores and creates the threads
 */
void main(void)
{
    input_output_config();  // config input-output pins 
    
    // Create and init semaphores
    k_sem_init(&sem_adc, 0, 1);
    k_sem_init(&sem_act, 0, 1);
    k_sem_init(&sem_mut, 1, 1); // semaphore to implement mutual exclusion

    // Create tasks
    thread_sampling_tid = k_thread_create(&thread_sampling_data, thread_sampling_stack,
        K_THREAD_STACK_SIZEOF(thread_sampling_stack), thread_sampling,
        NULL, NULL, NULL, thread_sampling_prio, 0, K_NO_WAIT);

    thread_sampling_tid = k_thread_create(&thread_processing_data, thread_processing_stack,
        K_THREAD_STACK_SIZEOF(thread_processing_stack), thread_processing,
        NULL, NULL, NULL, thread_processing_prio, 0, K_NO_WAIT);

    thread_actuation_tid = k_thread_create(&thread_actuation_data, thread_actuation_stack,
        K_THREAD_STACK_SIZEOF(thread_actuation_stack), thread_actuation,
        NULL, NULL, NULL, thread_actuation_prio, 0, K_NO_WAIT);

    thread_timer_tid = k_thread_create(&thread_timer_data, thread_timer_stack,
        K_THREAD_STACK_SIZEOF(thread_timer_stack), thread_timer,
        NULL, NULL, NULL, thread_timer_prio, 0, K_NO_WAIT);
    
    thread_interface_tid = k_thread_create(&thread_interface_data, thread_interface_stack,
        K_THREAD_STACK_SIZEOF(thread_interface_stack), thread_interface,
        NULL, NULL, NULL, thread_interface_prio, 0, K_NO_WAIT);

    return;
}

/** \brief Sampling thread
 *  
 *  This thread implements the sampling task which only operates in Automatic mode.
 * It's a periodic thread that reads the adc with period SAMP_PERIOD_MS and stores it in the sample buffer.
 * After sampling triggers the processing task.
 * 
 * \pre adc_sample()
 * 
 * \see SAMP_PERIOD_MS
 */
void thread_sampling(void *argA , void *argB, void *argC)
{
    /* Timing variables to control task periodicity */
    int64_t fin_time=0, release_time=0;
    
    adc_config();   // Configure adc
    
    /* Compute next release instant */
    release_time = k_uptime_get() + SAMP_PERIOD_MS;

    while(1)
    {
        if(mode == AUTOMATIC)
        {
            sample_buffer.data[sample_buffer.head] = adc_sample();  // Get adc sample and store it in sample buffer

            sample_buffer.head = (sample_buffer.head + 1) % FILTER_SIZE;   // Increment position to store data

            k_sem_give(&sem_adc);    // Signal new sample
        }
        
        /* Wait for next release instant */ 
        fin_time = k_uptime_get();
        if(fin_time < release_time)
        {
            k_msleep(release_time - fin_time);
            release_time += SAMP_PERIOD_MS;
        }
    }
}

/** \brief Processing thread
 *  
 *  This thread implements the task of processing.
 * It filters the data samples and updates the average of the filtered data samples.
 * It's a sporadic thread triggered by the end of sampling (sampling thread) and as
 * such only operates on Automatic mode. After processing triggers the actuation task.
 * 
 * \see filter(uint16_t *data)
 * 
 */
void thread_processing(void *argA , void *argB, void *argC)
{
    int data=0; // filtered data
    int intensity_real=0;   // real light intensity
    
    PI_init(0.5, 0.5);    // PI controller initialization

    while(1)
    {
        k_sem_take(&sem_adc, K_FOREVER);    // Wait for new sample
        
        k_sem_take(&sem_mut, K_FOREVER);    // Mutual exclusion on calendar operations

        // Check if it's time to change the light intensity based on the user data memory
        for(unsigned int i=0; i < mem_idx; i++)
        {
            if(mem[i].week_day == calendar.day && mem[i].hour == calendar.hour && mem[i].minute == calendar.minute)
            {
                intensity = mem[i].intensity;
            }
        }

        k_sem_give(&sem_mut);   // Get out of critical section

        data = filter(sample_buffer.data);   // Filter data
    
        intensity_real = (data - 250)*100 / 350; // Compute the real light intensity
        
        dutycycle = PI_controller(intensity, intensity_real, dutycycle);    // PI controller algorithm
        
        k_sem_give(&sem_act);   // Trigger actuation thread to update PWM dutycycle
    }
}

/** \brief Actuation thread
 *  
 *  This thread implements the task of actuation, it's triggered by the change of
 * the required light intensity (buttons 3 or 4) on Manual mode or by the end of
 * the processing thread when on Automatic mode.
 * It computes and updates the pulse width of the pwm signal from the intensity
 * value (when on Manual mode) or from the dutycycle value (when on Automatic mode).
 * It's a sporadic thread triggered by the press of button 3 or button 4 or by the
 * end of processing (processing thread), based on the operation mode.
 */
void thread_actuation(void *argA , void *argB, void *argC)
{
    const struct device *pwm0_dev;          /* Pointer to PWM device structure */
    unsigned int pwmPeriod_us = 1000;       /* PWM period in us */
    
    int ton = 0;

    pwm0_dev = device_get_binding(DT_LABEL(PWM0_NID));
        
    while(1)
    {
        k_sem_take(&sem_act, K_FOREVER);   // Wait for actuation trigger

        /* Compute ton of PWM based on operation mode 
        considering that the OUTPUT pin is on negative logic */
        if(mode == MANUAL)
            ton = pwmPeriod_us - (intensity*pwmPeriod_us)/100;
        
        else if(mode == AUTOMATIC)
            ton = pwmPeriod_us - (dutycycle*pwmPeriod_us)/100;

        pwm_pin_set_usec(pwm0_dev, PWM_PIN, pwmPeriod_us, ton, PWM_POLARITY_NORMAL);   // Update PWM
    }
}

/** \brief Timer thread to implement a calendar 
 *  
 *  This thread is a periodic thread with period 1 minute (TIMER_PERIOD_MS),
 * to update the day, hour and minutes
 */
void thread_timer(void *argA, void *argB, void *argC){

    /* Timing variables to control task periodicity */
    int64_t fin_time=0, release_time=0;
    
    /* Compute next release instant */
    release_time = k_uptime_get() + TIMER_PERIOD_MS;

    while(1)
    { 
        k_sem_take(&sem_mut, K_FOREVER); // Mutual exclusion on calendar operations

        // increase minutes
        calendar.minute++;
 
        // update hour
        if(calendar.minute == 60)
        {
            calendar.hour += 1;
            calendar.minute = 0;
        }
        
        // update day
        if(calendar.hour == 24)
        {
            calendar.hour = 0;
            calendar.minute = 0;
            calendar.day = (calendar.day + 1) % 7;
        }
        
        k_sem_give(&sem_mut);   // Get out of critical section

        // print day and time in HH : MM format
        printk("\nDAY = %s , %02d h : %02d min ", week_days[calendar.day], calendar.hour, calendar.minute);
              
        /* Wait for next release instant */ 
        fin_time = k_uptime_get();
        if(fin_time < release_time)
        {
            k_msleep(release_time - fin_time);
            release_time += TIMER_PERIOD_MS;
        }
  }

}

/** \brief Thread to implement the user interface 
 *
 * This thread implements the user interface, let's the user add new schedules
 * configurations, check them and change the current date and hour.
 *
 */
void thread_interface(void *argA , void *argB, void *argC)
{    
    char command;

    console_init();
    
    printk("\nPress 1 to add schedule");
    printk("\nPress 2 to check schedules");
    printk("\nPress 3 to change current date and hour");
    printk("\nPress 4 to check system time");

    while(1)
    {
        printk("\n\rcommand: ");
        command = console_getchar();    // Get user command

        switch(command)
        {
            case '1':   // Get Schedules of User

                printk("\nSetting new schedules");
                printk("\nWeek Day: ");
                mem[mem_idx].week_day = read_int(); // Get week day
                            
                printk("\nHour: ");
                mem[mem_idx].hour = read_int(); // Get Hour

                printf("\nMinute: ");
                mem[mem_idx].minute = read_int();   // Get Minute

                printf("\nIntensity: ");
                mem[mem_idx].intensity = read_int();    // Get light intensity
                        
                mem_idx++;  // increase memory position to insert data

                break;
            case '2':   // Print Memory contents
                
                printk("\n");
                
                for(unsigned int i=0; i < mem_idx; i++)
                    printf("%d: %s, %02d:%02d, %d\n", i, week_days[mem[i].week_day], mem[i].hour, mem[i].minute, mem[i].intensity);

                break;

            case '3':   // Update Date and Hour
                k_sem_take(&sem_mut, K_FOREVER);    // Mutual exclusion on calendar operations
                
                printk("\nSetting New DATE");
                printk("\nWeek Day: ");
		        calendar.day = read_int();  // Get Week day
                
                printk("\nHour: ");
		        calendar.hour = read_int(); // Get Hour

		        printf("\nMinute: ");
		        calendar.minute = read_int();   // Get Minute 
                
                k_sem_give(&sem_mut);   // Get out of critical section
                break;
            case '4':   // print system day and time in HH : MM format
                k_sem_take(&sem_mut, K_FOREVER);    // Mutual exclusion on calendar operations
                printk("\nSystem Time");
                printk("\nDAY = %s , %02d h : %02d min ", week_days[calendar.day], calendar.hour, calendar.minute);
                k_sem_give(&sem_mut);   // Get out of critical section

                break;
            default:
                break;
        }
    }
}

/** \brief Function to read integer values from user.
 *  \pre Initialize console with console_init() 

 *  Reads and returns integer until enter key is pressed (carriage return - \\r)
 * It also makes the echo of the key pressed. Uses the console_getchar() function.
 * Limited to 6 digit number, increase the size of the char vector c, to allow numbers
 * with more digits.
 *
 * \see console_init() 
 * \see console_getchar()
 * 
 */
int read_int()
{
    char c[6];
    unsigned int i=0;

    do
    {
        c[i] = console_getchar();
        
        printf("%c", c[i]);
        i++;
    }while(c[i-1] != '\r');
    
    return atoi(c);
}

/** \brief Function to implement a digital filter
 *  
 *  This function implements a digital filter that removes the outliers
 * (10% or high deviation from average) from a set of data and computes the average
 * of the remaining samples.  
 *
 * \param[in] data pointer to array 
 * 
 * \returns average of the data without the outliers
 */
int filter(int *data)
{
    int i, j=0;
    int avg = 0, high_limit, low_limit;
    int new_data[FILTER_SIZE];
    
    // Array empty init
    array_init(new_data, FILTER_SIZE);

    avg = array_average(data, FILTER_SIZE);     // Get array average
    
    // Outliers Calculation
    high_limit = avg * 1.1;
    low_limit = avg * 0.9;
    
    for(i = 0; i < FILTER_SIZE; i++)
    {
        if(data[i] >= low_limit && data[i] <= high_limit)   // More then average*1.1 and less then average*0.9 IGNORE
        {
            new_data[j] = data[i];
            j++;
        }
    }

    // If empty data array set size to one to not generate error calculating average
    if(j == 0)
        j = 1;

    return array_average(new_data, j);  // return average of filtered data
}

/** \brief Function to initialize integer array
 * 
 *  This function fills an integer array with zeros
 *
 * \param[in] data pointer to array 
 * \param[in] size number of elements of the array
 */
void array_init(int *data, int size)
{
    for(unsigned int i = 0; i < size; i++)
        data[i] = 0;
}

/** \brief Function to compute the average of an array
 * 
 *  This function computes the average of an array of integers.
 *
 * \param[in] data pointer to array 
 * \param[in] size number of elements of the array
 * 
 * \return average of the array (integer type)
 */
int array_average(int *data, int size)
{
    int sum = 0;

    for(unsigned int i = 0; i < size; i++)
    {
        sum += data[i];
    }
    return sum / size;
}

/** \brief Configuration Function.
 * 
 *  This function makes all the hardware configuration. Configures the input and output pins and
 * the interruptions of the microcontroller.
 * 
 */
void input_output_config(void)
{
    const struct device *gpio0_dev;         /* Pointer to GPIO device structure */

    /* Bind to GPIO 0 */
    gpio0_dev = device_get_binding(DT_LABEL(GPIO0_NID));
    
    /* Configure BOARD BUTTON PINS */
    gpio_pin_configure(gpio0_dev, BOARDBUT1, GPIO_INPUT | GPIO_PULL_UP);
    gpio_pin_configure(gpio0_dev, BOARDBUT2, GPIO_INPUT | GPIO_PULL_UP);
    gpio_pin_configure(gpio0_dev, BOARDBUT3, GPIO_INPUT | GPIO_PULL_UP);
    gpio_pin_configure(gpio0_dev, BOARDBUT4, GPIO_INPUT | GPIO_PULL_UP);
    

    /* Set interrupt HW - which pin and event generate interrupt
    Board buttons configured to generate interrupt on rising-edge
    Control Panel buttons configured to generate interrupt on falling-edge */

    gpio_pin_interrupt_configure(gpio0_dev, BOARDBUT1, GPIO_INT_EDGE_TO_ACTIVE);
    gpio_pin_interrupt_configure(gpio0_dev, BOARDBUT2, GPIO_INT_EDGE_TO_ACTIVE);
    gpio_pin_interrupt_configure(gpio0_dev, BOARDBUT3, GPIO_INT_EDGE_TO_ACTIVE);
    gpio_pin_interrupt_configure(gpio0_dev, BOARDBUT4, GPIO_INT_EDGE_TO_ACTIVE);
  
    /* Set callbacks */
    gpio_init_callback(&button_cb_data, buttons_cbfunction, BIT(BOARDBUT1)| BIT(BOARDBUT2)| BIT(BOARDBUT3) | BIT(BOARDBUT4));
    gpio_add_callback(gpio0_dev, &button_cb_data);
}