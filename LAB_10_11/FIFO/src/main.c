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
#define SAMP_PERIOD_MS  5

#define SIZE 10

/* Size of stack area used by each thread (can be thread specific, if necessary)*/
#define STACK_SIZE 1024

/* Thread scheduling priority */
#define thread_sampling_prio 1
#define thread_processing_prio 1
#define thread_actuation_prio 1

#define GPIO0_NID DT_NODELABEL(gpio0) 
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

/* Create fifos*/
struct k_fifo fifo_sample;
struct k_fifo fifo_average;

/* Create fifo data structure and variables */
struct data_item_t {
    void *fifo_reserved;    /* 1st word reserved for use by FIFO */
    uint16_t data;          /* Actual data */
};

typedef struct {
    uint16_t data[SIZE];
    int head;
}buffer;

/* Thread code prototypes */
void thread_sampling(void *argA, void *argB, void *argC);
void thread_processing(void *argA, void *argB, void *argC);
void thread_actuation(void *argA, void *argB, void *argC);

int filter(uint16_t*);
void array_init(uint16_t*, int);
int array_average(uint16_t*, int);


/* Main function */
void main(void) {
    
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
    struct data_item_t sample;

    /* Create/Init fifos */
    k_fifo_init(&fifo_sample);
    
    adc_config();
    
    /* Compute next release instant */
    release_time = k_uptime_get() + SAMP_PERIOD_MS;

    /* Thread loop */
    while(1)
    {
        sample.data = adc_sample();
        
        printk("\n----------------------------\n");
        printk("\nsample = %d\n", sample.data);

        k_fifo_put(&fifo_sample, &sample);        
       
        
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
    struct data_item_t *data_samples;
    struct data_item_t average;
    buffer buffer;
    
    /* Create/Init fifos */
    k_fifo_init(&fifo_average);
    
    while(1)
    {
       data_samples = k_fifo_get(&fifo_sample, K_FOREVER);
       buffer.data[buffer.head] = data_samples->data;
       
       buffer.head = (buffer.head + 1) % SIZE;
        
        average.data = filter(buffer.data);
        
        printk("\nnew average = %d\n",average.data);
        k_fifo_put(&fifo_average, &average);
         
    }
}

void thread_actuation(void *argA , void *argB, void *argC)
{
    const struct device *pwm0_dev;          /* Pointer to PWM device structure */
    unsigned int pwmPeriod_us = 1000;       /* PWM priod in us */
    struct data_item_t *average;

    pwm0_dev = device_get_binding(DT_LABEL(PWM0_NID));
    int ton=0;

    while(1)
    {
         average = k_fifo_get(&fifo_average, K_FOREVER);
        
        ton = ((average->data*1000)/3000);
        
        printk("ton = %d\n",ton);
        
        pwm_pin_set_usec(pwm0_dev, BOARDLED_PIN, pwmPeriod_us, ton, PWM_POLARITY_NORMAL);
    }
}

int filter(uint16_t *data)
{
    int i, j=0;
    int avg = 0, high_limit, low_limit;
    uint16_t new_data[SIZE];
    
    array_init(new_data, SIZE);

    printk("samples: ");
    avg = array_average(data, SIZE);
    
    printk("\navg = %d\n",avg);
    
    high_limit = avg * 1.1;
    low_limit = avg * 0.9;
    
    for(i = 0; i < SIZE; i++)
    {
        if(data[i] >= low_limit && data[i] <= high_limit)
        {
            new_data[j] = data[i];
            j++;
        }
    }

    if(j == 0)
        j = 1;
    
    printk("filtered: ");

    return array_average(new_data, j);;
}

void array_init(uint16_t *data, int size)
{
    for(unsigned int i = 0; i < size; i++)
        data[i] = 0;
}

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