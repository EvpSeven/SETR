/** \file main.c
 * 	\brief Program implementing an application using shared memory and semaphores for processing input values from ADC
 * 
 *  The system implements a basic processing of an analog signal. It reads the input voltage from
 * an analog sensor (emulated by a 10 kΩ potentiometer), digitally filters the signal and outputs it (PWM signal).
 * The Digital filter consists in a moving average filter that removes the outliers (10% or high deviation from average)
 * and outputs the average of the remaining samples. This average is computed into a pulse width of a pwm signal that is 
 * applied to one of the DevKit leds. It was implemented recuuring to threads, shared-memory and semaphores.
 *  It was implemented using the board Nordic nrf52840-dk.
 * 
 * \author André Brandão
 * \author Emanuel Pereira
 * \date 26/05/2022
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

/* import ADC file */
#include <ADC.h>

#define SAMP_PERIOD_MS  1000 /**< Sample period (ms) */

#define SIZE 10 /**< Window Size of samples (digital filter) */

#define STACK_SIZE 1024 /**< Size of stack area used by each thread */

#define thread_sampling_prio 1      /**< Scheduling priority of sampling thread */
#define thread_processing_prio 1    /**< Scheduling priority of processing thread */
#define thread_actuation_prio 1     /**< Scheduling actuation of sampling thread */

#define PWM0_NID DT_NODELABEL(pwm0)     /**< pwm0 Node Label from device tree (refer to dts file) */
#define BOARDLED_PIN 0x0e   /**< Address of the LED pin used to output pwm */ 

K_THREAD_STACK_DEFINE(thread_sampling_stack, STACK_SIZE);   /**< Create sampling thread stack space */
K_THREAD_STACK_DEFINE(thread_processing_stack, STACK_SIZE); /**< Create processing thread stack space */
K_THREAD_STACK_DEFINE(thread_actuation_stack, STACK_SIZE);  /**< Create actuation thread stack space */
  
struct k_thread thread_sampling_data;   /**< Sampling thread data */
struct k_thread thread_processing_data; /**< Processing thread data */
struct k_thread thread_actuation_data;  /**< Actuation thread data */

k_tid_t thread_sampling_tid;    /**< Sampling thread task ID */
k_tid_t thread_processing_tid;  /**< Processing thread task ID */
k_tid_t thread_actuation_tid;   /**< Actuation thread task ID */

/** Circular array to store data */
typedef struct {
    uint16_t data[SIZE];    /**< Array to store data*/
    int head;    /**< Index of next position to store data */   
}buffer;

// Global variables (shared memory) to communicate between tasks
buffer sample_buffer;   /**< Buffer to store samples to communicate between tasks*/ 
int average;    /**< Store filter output (communicate between tasks) */

// Semaphores for task synch
struct k_sem sem_adc;   /**< Semaphore to synch sample and processing tasks (signals end of sampling)*/
struct k_sem sem_proc;  /**< Semaphore to synch processing and actuation tasks (signals end of processing)*/

void thread_sampling(void *argA, void *argB, void *argC);
void thread_processing(void *argA, void *argB, void *argC);
void thread_actuation(void *argA, void *argB, void *argC);

int filter(uint16_t*);
void array_init(uint16_t*, int);
int array_average(uint16_t*, int);


/** \brief Main function
 * 
 *  The main function initializes the semaphores and creates the threads
 */
void main(void) {
    
    /* Create and init semaphores */
    k_sem_init(&sem_adc, 0, 1);
    k_sem_init(&sem_proc, 0, 1);
    
    /* Create tasks */
    thread_sampling_tid = k_thread_create(&thread_sampling_data, thread_sampling_stack,
        K_THREAD_STACK_SIZEOF(thread_sampling_stack), thread_sampling,
        NULL, NULL, NULL, thread_sampling_prio, 0, K_NO_WAIT);

    thread_processing_tid = k_thread_create(&thread_processing_data, thread_processing_stack,
        K_THREAD_STACK_SIZEOF(thread_processing_stack), thread_processing,
        NULL, NULL, NULL, thread_processing_prio, 0, K_NO_WAIT);

    thread_actuation_tid = k_thread_create(&thread_actuation_data, thread_actuation_stack,
        K_THREAD_STACK_SIZEOF(thread_actuation_stack), thread_actuation,
        NULL, NULL, NULL, thread_actuation_prio, 0, K_NO_WAIT);

    return;
}

/** \brief Sampling thread
 *  
 *  This thread implements the sampling task.
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
    
    array_init(sample_buffer.data, SIZE);   // Initialize array with zeros
    adc_config();   // Configure adc
    
    /* Compute next release instant */
    release_time = k_uptime_get() + SAMP_PERIOD_MS;

    /* Thread loop */
    while(1)
    {
        sample_buffer.data[sample_buffer.head] = adc_sample();  // Get adc sample and store it in sample buffer
        
        printk("\n----------------------------\n");
        printk("\nsample = %d\n", sample_buffer.data[sample_buffer.head]);

        sample_buffer.head = (sample_buffer.head + 1) % SIZE;   // Increment position to store data
        
       k_sem_give(&sem_adc);    // Signal new sample
        
        /* Wait for next release instant */ 
        fin_time = k_uptime_get();
        if( fin_time < release_time)
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
 * It's a sporadic thread triggered by the end of sampling (sampling thread). After processing
 * triggers the actuation task.
 * 
 * \see filter(uint16_t *data)
 * 
 */
void thread_processing(void *argA , void *argB, void *argC)
{
    while(1)
    {
        k_sem_take(&sem_adc,  K_FOREVER);   // Wait for new sample
        
        average = filter(sample_buffer.data);   // Filter data
        
        printk("\nnew average = %d\n",average);

        k_sem_give(&sem_proc);  // Signal new processed data
    }
}

/** \brief Actuation thread
 *  
 *  This thread implements the task of actuation.
 * It computes the pulse width of the pwm signal from the average (computed on the processing task)
 * and updates the pwm signal. It's a sporadic thread triggered by the end of processing (processing thread).
 */
void thread_actuation(void *argA , void *argB, void *argC)
{
    const struct device *pwm0_dev;          /* Pointer to PWM device structure */
    unsigned int pwmPeriod_us = 1000;       /* PWM period in us */

    int ton=0;  // PWM ton

    pwm0_dev = device_get_binding(DT_LABEL(PWM0_NID));
    

    while(1)
    {
        k_sem_take(&sem_proc, K_FOREVER);   // Wait for new processed data
        
        ton = ((average*1000)/3000);    // Compute ton of PWM
        
        printk("ton = %d\n",ton);
        
        pwm_pin_set_usec(pwm0_dev, BOARDLED_PIN, pwmPeriod_us, ton, PWM_POLARITY_NORMAL);   // Update PWM
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
int filter(uint16_t *data)
{
    int i, j=0;
    int avg = 0, high_limit, low_limit;
    uint16_t new_data[SIZE];
    
    // Array empty init
    array_init(new_data, SIZE);

    printk("samples: ");
    avg = array_average(data, SIZE);     // Get array average
    
    printk("\navg = %d\n",avg);
    
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
    
    printk("filtered: ");

    return array_average(new_data, j);  // return average of filtered data
}

/** \brief Function to initialize integer array
 * 
 *  This function fills an integer array with zeros
 *
 * \param[in] data pointer to array 
 * \param[in] size number of elements of the array
 */
void array_init(uint16_t *data, int size)
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
int array_average(uint16_t *data, int size)
{
    int sum = 0;

    for(unsigned int i = 0; i < size; i++)
    {
        sum += data[i];
        printk("%d ", data[i]);
    }
    return sum / size;
}