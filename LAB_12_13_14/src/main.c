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

#define MANUAL 0      /**< Flag that indicates Manual mode is selected */ 
#define AUTOMATIC 1   /**< Flag that indicates Automatic mode is selected */ 

#define SIZE 10 /**< Window Size of samples (digital filter) */

#define STACK_SIZE 1024 /**< Size of stack area used by each thread */

#define thread_sampling_prio 1      /**< Scheduling priority of sampling thread */
#define thread_actuation_prio 1     /**< Scheduling actuation of sampling thread */

#define PWM0_NID DT_NODELABEL(pwm0)     /**< pwm0 Node Label from device tree (refer to dts file) */
#define PWM_PIN 0x0e   /**< Address of the pin used to output pwm */ 

K_THREAD_STACK_DEFINE(thread_sampling_stack, STACK_SIZE);   /**< Create sampling thread stack space */
K_THREAD_STACK_DEFINE(thread_processing_stack, STACK_SIZE); /**< Create processing thread stack space */
K_THREAD_STACK_DEFINE(thread_actuation_stack, STACK_SIZE);  /**< Create actuation thread stack space */
  
struct k_thread thread_sampling_data;   /**< Sampling thread data */
struct k_thread thread_processing_data; /**< Processing thread data */
struct k_thread thread_actuation_data;  /**< Actuation thread data */

k_tid_t thread_sampling_tid;    /**< Sampling thread task ID */
k_tid_t thread_actuation_tid;   /**< Actuation thread task ID */

// Global variables (shared memory) to communicate between tasks
int sample = 0;   /**< Store samples to communicate between tasks*/ 

int mode = MANUAL;
int intensity = 0;

// Semaphores for task synch
struct k_sem sem_adc;   /**< Semaphore to synch sample and actuation tasks (signals end of sampling)*/

void thread_sampling(void *argA, void *argB, void *argC);
void thread_actuation(void *argA, void *argB, void *argC);

void butcoinpress_cbfunction(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
       
    if(BIT(BOARDBUT1) & pins)
        mode = AUTOMATIC;

    if(BIT(BOARDBUT2) & pins)
        mode = MANUAL;

    if(BIT(BOARDBUT3) & pins)
        intensity++;

    if(BIT(BOARDBUT4) & pins)
        intensity--;

}

void main(void)
{
    /* Create and init semaphores */
    k_sem_init(&sem_adc, 0, 1);

    /* Create tasks */
    thread_sampling_tid = k_thread_create(&thread_sampling_data, thread_sampling_stack,
        K_THREAD_STACK_SIZEOF(thread_sampling_stack), thread_sampling,
        NULL, NULL, NULL, thread_sampling_prio, 0, K_NO_WAIT);

    thread_actuation_tid = k_thread_create(&thread_actuation_data, thread_actuation_stack,
        K_THREAD_STACK_SIZEOF(thread_actuation_stack), thread_actuation,
        NULL, NULL, NULL, thread_actuation_prio, 0, K_NO_WAIT);

    return;
}

void thread_sampling(void *argA , void *argB, void *argC)
{
    /* Timing variables to control task periodicity */
    int64_t fin_time=0, release_time=0;
    
    adc_config();   // Configure adc
    
    /* Compute next release instant */
    release_time = k_uptime_get() + SAMP_PERIOD_MS;

    /* Thread loop */
    while(1)
    {
        sample = adc_sample();  // Get adc sample and store it in sample buffer
        
        printk("\n----------------------------\n");
        printk("\nsample = %d\n", sample);
        
        k_sem_give(&sem_adc);    // Signal new sample
        
        /* Wait for next release instant */ 
        fin_time = k_uptime_get();
        if(fin_time < release_time)
        {
            k_msleep(release_time - fin_time);
            release_time += SAMP_PERIOD_MS;
        }
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
        k_sem_take(&sem_adc, K_FOREVER);   // Wait for new sample
        
        // current = (VDD - sample) * R
        // luminance calc 
        // PI controller

        ton = ((average*1000)/3000);    // Compute ton of PWM
        
        printk("ton = %d\n",ton);
        
        pwm_pin_set_usec(pwm0_dev, PWM_PIN, pwmPeriod_us, ton, PWM_POLARITY_NORMAL);   // Update PWM
    }
}