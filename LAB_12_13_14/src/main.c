#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <drivers/pwm.h>
#include <drivers/uart.h>
#include <sys/printk.h>
#include <sys/__assert.h>
#include <string.h>
#include <timing/timing.h>
#include <stdlib.h>
#include <stdio.h>

/* import ADC file */
#include <ADC.h>

#define SAMP_PERIOD_MS  1000 /**< Sample period (ms) */
#define TIMER_PERIOD_MS 60000 /**< Timer thread period - 1 minute */

#define MANUAL 0      /**< Flag that indicates Manual mode is selected */ 
#define AUTOMATIC 1   /**< Flag that indicates Automatic mode is selected */ 

#define SIZE 10 /**< Window Size of samples (digital filter) */

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

/* Therad periodicity (in ms)*/
#define thread_timer_period 60000

#define GPIO0_NID DT_NODELABEL(gpio0)   /**< gpio0 Node Label from device tree (refer to dts file) */
#define PWM0_NID DT_NODELABEL(pwm0)     /**< pwm0 Node Label from device tree (refer to dts file) */
#define PWM_PIN 0x0e   /**< Address of the pin used to output pwm */ 

#define UART_NID DT_NODELABEL(uart0)    /* UART Node label, see dts */
#define RXBUF_SIZE 60                   /* RX buffer size */
#define TXBUF_SIZE 60                   /* TX buffer size */
#define RX_TIMEOUT 1000                  /* Inactivity period after the instant when last char was received that triggers an rx event (in us) */

K_THREAD_STACK_DEFINE(thread_sampling_stack, STACK_SIZE);   /**< Create sampling thread stack space */
K_THREAD_STACK_DEFINE(thread_processing_stack, STACK_SIZE); /**< Create processing thread stack space */
K_THREAD_STACK_DEFINE(thread_actuation_stack, STACK_SIZE);  /**< Create actuation thread stack space */
K_THREAD_STACK_DEFINE(thread_timer_stack, STACK_SIZE);      /**< Create timer (calendar) thread stack space */
K_THREAD_STACK_DEFINE(thread_interface_stack, STACK_SIZE);      /**< Create interface thread stack space */

  
struct k_thread thread_sampling_data;   /**< Sampling thread data */
struct k_thread thread_processing_data; /**< Processing thread data */
struct k_thread thread_actuation_data;  /**< Actuation thread data */
struct k_thread thread_timer_data;      /**< Timer thread data */
struct k_thread thread_interface_data;  /**< Interface thread data */

k_tid_t thread_sampling_tid;    /**< Sampling thread task ID */
k_tid_t thread_processing_tid;  /**< Processing thread task ID */
k_tid_t thread_actuation_tid;   /**< Actuation thread task ID */
k_tid_t thread_timer_tid;       /**< Timer thread task ID */
k_tid_t thread_interface_tid;   /**< Interface thread task ID */

// Buttons callback structures
static struct gpio_callback button_cb_data;

/* Struct for UART configuration (if using default valuies is not needed) */
const struct uart_config uart_cfg = {
		.baudrate = 115200,
		.parity = UART_CFG_PARITY_NONE,
		.stop_bits = UART_CFG_STOP_BITS_1,
		.data_bits = UART_CFG_DATA_BITS_8,
		.flow_ctrl = UART_CFG_FLOW_CTRL_NONE
};

/* UART related variables */
const struct device *uart_dev;          /* Pointer to device struct */ 
static uint8_t rx_buf[RXBUF_SIZE];      /* RX buffer, to store received data */
static uint8_t rx_chars[RXBUF_SIZE];    /* chars actually received  */

/** Circular array to store data */
typedef struct {
    int data[SIZE];    /**< Array to store data*/
    int head;    /**< Index of next position to store data */   
}buffer;

// Global variables (shared memory) to communicate between tasks
buffer sample_buffer;   /**< Buffer to store samples to communicate between tasks*/ 

int mode = MANUAL;
int intensity = 1;

// Semaphores for task synch
struct k_sem sem_adc;   /**< Semaphore to synch sample and actuation tasks (signals end of sampling)*/
struct k_sem sem_proc;  
struct k_sem sem_uart;

/* Thread code prototypes */
void thread_sampling(void *argA, void *argB, void *argC);
void thread_processing(void *argA, void *argB, void *argC);
void thread_actuation(void *argA, void *argB, void *argC);
void thread_timer(void *argA, void *argB, void *argC);
void thread_interface(void *argA, void *argB, void *argC);

int filter(int*);
void array_init(int*, int);
int array_average(int*, int);
void input_output_config(void);
void uart_config(void);

void buttons_cbfunction(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
       
    if(BIT(BOARDBUT1) & pins)
    {
        mode = AUTOMATIC;
        printk("\nChanged to Automatic mode\n");
    }

    if(BIT(BOARDBUT2) & pins)
    {
        mode = MANUAL;
        printk("\nChanged to Manual mode\n");
    }

    if((BIT(BOARDBUT3) & pins) && mode == MANUAL)
    {
        intensity++;

        if(intensity > 100)
            intensity = 100;
            
        printk("intensity = %d\n", intensity);

        k_sem_give(&sem_proc);
    }

    if((BIT(BOARDBUT4) & pins) && mode == MANUAL)
    {
        intensity--;

         if(intensity < 0)
            intensity = 0;

        printk("intensity = %d\n", intensity);

        k_sem_give(&sem_proc);
    }
}

static void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data)
{
    int err;

    switch (evt->type) {
	
        case UART_TX_DONE:
		printk("UART_TX_DONE event \n\r");
                break;

	case UART_TX_ABORTED:
		printk("UART_TX_ABORTED event \n\r");
		break;
		
	case UART_RX_RDY:
		printk("UART_RX_RDY event \n\r");
                /* Just copy data to a buffer. Usually it is preferable to use e.g. a FIFO to communicate with a task that shall process the messages*/
                memcpy(rx_chars,&(rx_buf[evt->data.rx.offset]),evt->data.rx.len); 
                rx_chars[evt->data.rx.len]=0; /* Terminate the string */
                k_sem_give(&sem_uart);
		break;

	case UART_RX_BUF_REQUEST:
		printk("UART_RX_BUF_REQUEST event \n\r");
		break;

	case UART_RX_BUF_RELEASED:
		printk("UART_RX_BUF_RELEASED event \n\r");
		break;
		
	case UART_RX_DISABLED: 
                /* When the RX_BUFF becomes full RX is is disabled automaticaly.  */
                /* It must be re-enabled manually for continuous reception */
                printk("UART_RX_DISABLED event \n\r");
		err =  uart_rx_enable(uart_dev ,rx_buf,sizeof(rx_buf),RX_TIMEOUT);
                if (err) {
                    printk("uart_rx_enable() error. Error code:%d\n\r",err);
                    //exit(FATAL_ERR);                
                }
		break;

	case UART_RX_STOPPED:
		printk("UART_RX_STOPPED event \n\r");
		break;
		
	default:
                printk("UART: unknown event \n\r");
		break;
    }

}

void main(void)
{
    input_output_config();
    uart_config();
    
    /* Create and init semaphores */
    k_sem_init(&sem_adc, 0, 1);
    k_sem_init(&sem_proc, 0, 1);
    k_sem_init(&sem_uart, 0, 1);

    /* Create tasks */
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
        
            printk("\n----------------------------\n");
            printk("\nsample = %d\n", sample_buffer.data[sample_buffer.head]);

            sample_buffer.head = (sample_buffer.head + 1) % SIZE;   // Increment position to store data

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

void thread_processing(void *argA , void *argB, void *argC)
{
    int R=20;
    
    int data=0;
    int current=0;
    int lum=0;

    while(1)
    {
        k_sem_take(&sem_adc, K_FOREVER);   // Wait for new sample
        
        data = filter(sample_buffer.data);   // Filter data
        
        current = data / R;
        
        lum = (669*current + 173)/1000;

        printk("avg: %d", data);
        printk("\nCurrent: %d", current);
        printk("\nlum: %d", lum);

        // PI controller

        k_sem_give(&sem_proc);
    }
}

void thread_actuation(void *argA , void *argB, void *argC)
{
    const struct device *pwm0_dev;          /* Pointer to PWM device structure */
    unsigned int pwmPeriod_us = 1000;       /* PWM period in us */
    
    int ton=0;  // PWM ton

    pwm0_dev = device_get_binding(DT_LABEL(PWM0_NID));
    
    while(1)
    {
        k_sem_take(&sem_proc, K_FOREVER);   // Wait for new sample

        ton = pwmPeriod_us - (intensity*pwmPeriod_us)/100;    // Compute ton of PWM
        
        pwm_pin_set_usec(pwm0_dev, PWM_PIN, pwmPeriod_us, ton, PWM_POLARITY_NORMAL);   // Update PWM
    }
}

void thread_timer(void *argA, void *argB, void *argC){

    /* Timing variables to control task periodicity */
    int64_t fin_time=0, release_time=0;
    
    static char *week_days[7] = {"Domingo", "Segunda-feira", "Terça-feira", "Quarta-Feira", "Quinta-Feira",
                                 "Sexta-Feira", "Sábado"};

    int hour=0, minute=0, day=0;
    
    /* Compute next release instant */
    release_time = k_uptime_get() + TIMER_PERIOD_MS;

    while(1)
    {
        /*if(day==0)
          printk("Domingo ");
        if(day==1)
          printk("Segunda-Feira ");
        if(day==2)
          printk("Terça-Feira ");
        if(day==3)
          printk("Quarta-Feira ");          
        if(day==4)
          printk("Quinta-Feira ");
        if(day==5)
          printk("Sexta-Feira ");          
        if(day==6)
          printk("Sabado-Feira ");*/
                                          
        //print day and time in HH : MM format
        printk("DAY = %s , %02d h : %02d min ", week_days[day], hour, minute);
        
        //increase minutes
        minute++;
 
        //update hour, minute and second
        if(minute == 60)
        {
            hour += 1;
            minute = 0;
        }
        
        if(hour == 24)
        {
            hour = 0;
            minute = 0;
            day = (day + 1) % 7;
        }
                  
        /* Wait for next release instant */ 
        fin_time = k_uptime_get();
        if(fin_time < release_time)
        {
            k_msleep(release_time - fin_time);
            release_time += TIMER_PERIOD_MS;
        }
  }

}

void thread_interface(void *argA , void *argB, void *argC)
{
    while(1)
    {
        k_sem_take(&sem_uart, K_FOREVER);   // Wait for new sample

        printf("type: %s", rx_chars);
        // INTERFACE
    }
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
    int new_data[SIZE];
    
    // Array empty init
    array_init(new_data, SIZE);

    //printk("samples: ");
    avg = array_average(data, SIZE);     // Get array average
    
    //printk("\navg = %d\n",avg);
    
    // Outliers Calculation
    high_limit = avg * 1.1;
    low_limit = avg * 0.9;
    
    for(i = 0; i < SIZE; i++)
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
    
    //printk("filtered: ");

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
        //printk("%d ", data[i]);
    }
    return sum / size;
}

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

void uart_config(void)
{
    /* Bind to UART */
    uart_dev = device_get_binding(DT_LABEL(UART_NID));

    /* Configure UART */
    uart_configure(uart_dev, &uart_cfg);

    /* Register callback */
    uart_callback_set(uart_dev, uart_cb, NULL);

    /* Enable data reception */
    uart_rx_enable(uart_dev ,rx_buf,sizeof(rx_buf),RX_TIMEOUT);
}