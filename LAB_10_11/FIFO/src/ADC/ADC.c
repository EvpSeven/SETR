/** \file ADC.c
 * 	\brief Module that implements ADC operations in nRF52840DK_nRF52840 board
 * 
 * \author André Brandão
 * \author Emanuel Pereira
 * \date 29/05/2022
 */
#include "ADC.h"

/* Global vars */
const struct device *adc_dev = NULL; /**< Pointer to ADC device structure */
static uint16_t adc_sample_buffer[BUFFER_SIZE]; /**< Buffer to store the adc samples */

/** \brief Function to configure ADC
 *   
 *  ADC driver's configuration
 * 
 *  \see adc_sample()
*/
void adc_config()
{
	int err;

	adc_dev = device_get_binding(DT_LABEL(ADC_NID));
	if (!adc_dev) {
        printk("ADC device_get_binding() failed\n");
    } 
    err = adc_channel_setup(adc_dev, &my_channel_cfg);
    if (err) {
        printk("adc_channel_setup() failed with error code %d\n", err);
    }

	/* It is recommended to calibrate the SAADC at least once before use, and whenever the ambient temperature has changed by more than 10 °C */
    NRF_SAADC->TASKS_CALIBRATEOFFSET = 1;
}

/** \brief Function to get samples from ADC
 *   
 *  This function performs an ADC conversion and converts it to the corresponfing tension.
 *
 *  \pre adc_read() has been called
 *
 *  \returns tension in milivolts from ADC.
 * 
 *  \see adc_config()
*/
uint16_t adc_sample(void)
{
	int ret;
	const struct adc_sequence sequence = {
		.channels = BIT(ADC_CHANNEL_ID),
		.buffer = adc_sample_buffer,
		.buffer_size = sizeof(adc_sample_buffer),
		.resolution = ADC_RESOLUTION,
	};

	if (adc_dev == NULL) {
            printk("adc_sample(): error, must bind to adc first \n\r");
            return -1;
	}

	ret = adc_read(adc_dev, &sequence);

	if (ret) {
            printk("adc_read() failed with code %d\n", ret);
	}	
	
         if(adc_sample_buffer[0] > 1023)
                adc_sample_buffer[0] = 0;

	return (uint16_t)(1000*adc_sample_buffer[0]*((float)3/1023));
}
