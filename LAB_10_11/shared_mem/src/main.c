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

#include <ADC.h>

/* Sampling Thread periodicity (in ms)*/
#define SAMP_PERIOD_MS  1000

#define SIZE 10

/* Size of stack area used by each thread (can be thread specific, if necessary)*/
#define STACK_SIZE 1024

/* Thread scheduling priority */
#define thread_sampling_prio 1
#define thread_processing_prio 1
#define thread_actuation_prio 1

#define PWM0_NID DT_NODELABEL(pwm0) 
#define BOARDLED_PIN 0x0e

/* Create thread stack space */
K_THREAD_STACK_DEFINE(thread_sampling_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(thread_processing_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(thread_actuation_stack, STACK_SIZE);
  
/* Create variables for thread data */
struct k_thread thread_sampling_data;
struct k_thread thread_processing_data;
struct k_thread thread_actuation_data;

/* Create task IDs */
k_tid_t thread_sampling_tid;
k_tid_t thread_processing_tid;
k_tid_t thread_actuation_tid;

/* Global vars (shared memory between tasks A/B and B/C, resp) */
typedef struct {
    uint16_t data[SIZE];
    int head;
}buffer;

buffer sample_buffer;

int average;

/* Semaphores for task synch */
struct k_sem sem_adc;
struct k_sem sem_proc;

/* Thread code prototypes */
void thread_sampling(void *argA, void *argB, void *argC);
void thread_processing(void *argA, void *argB, void *argC);
void thread_actuation(void *argA, void *argB, void *argC);

int filter(uint16_t*);
float array_average(int*, int);


/* Main function */
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

/* Thread code implementation */
void thread_sampling(void *argA , void *argB, void *argC)
{
    /* Timing variables to control task periodicity */
    int64_t fin_time=0, release_time=0;
    
    adc_config();
    
    /* Compute next release instant */
    release_time = k_uptime_get() + SAMP_PERIOD_MS;

    /* Thread loop */
    while(1)
    {
        sample_buffer.data[sample_buffer.head] = adc_sample();
        
        printk("%d", sample_buffer.data[sample_buffer.head]);

        sample_buffer.head = (sample_buffer.head + 1) % SIZE;
        
        k_sem_give(&sem_adc);
        
        /* Wait for next release instant */ 
        fin_time = k_uptime_get();
        if( fin_time < release_time)
        {
            k_msleep(release_time - fin_time);
            release_time += SAMP_PERIOD_MS;
        }
    }

}

void thread_processing(void *argA , void *argB, void *argC)
{
    while(1)
    {
        k_sem_take(&sem_adc,  K_FOREVER);
        
        average = filter(sample_buffer.data);

        k_sem_give(&sem_proc);  
    }
}

void thread_actuation(void *argA , void *argB, void *argC)
{
    const struct device *pwm0_dev;          /* Pointer to PWM device structure */
    unsigned int pwmPeriod_us = 1000;       /* PWM priod in us */

    pwm0_dev = device_get_binding(DT_LABEL(PWM0_NID));

    while(1)
    {
        k_sem_take(&sem_proc, K_FOREVER);
        
        pwm_pin_set_usec(pwm0_dev, BOARDLED_PIN, pwmPeriod_us,(unsigned int)((pwmPeriod_us*average)/100), PWM_POLARITY_NORMAL);
    }
}

int filter(uint16_t *data)
{
    unsigned int i, j=0;
    float avg = 0, high_limit, low_limit;
    uint16_t new_data[BUFFER_SIZE];

    avg = array_average(data, BUFFER_SIZE);
    high_limit = avg * 1.1;
    low_limit = avg * 0.9;
    
    for(i = 0; i < BUFFER_SIZE; i++)
    {
        if(data[i] > low_limit && data[i] < high_limit)
        {
            new_data[j] = data[i];
            j++;
        }
    }
    
    return array_average(new_data, j); 
}

float array_average(int *data, int size)
{
    float sum = 0;

    for(unsigned int i = 0; i < size; i++)
        sum += data[i];

    return sum / size;
}