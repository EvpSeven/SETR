/** \file ADC.h
 * 	\brief Header of Program that implements ADC to nRF52840DK_nRF52840 board
 *
 *  
 * 
 * \author André Brandão
 * \author Emanuel Pereira
 * \date 26/05/2022
 */
 
#ifndef _ADC_H
#define _ADC_H

#include <drivers/adc.h> //Import ADC Drivers definitions

/*ADC definitions and includes*/
#include <hal/nrf_saadc.h>

#define ADC_NID DT_NODELABEL(adc) /**<  */
#define ADC_RESOLUTION 10 /**< ADC Resolution */
#define ADC_GAIN ADC_GAIN_1_4 /**< ADC Gain */
#define ADC_REFERENCE ADC_REF_VDD_1_4 /**< ADC Reference Tension */
#define ADC_ACQUISITION_TIME ADC_ACQ_TIME(ADC_ACQ_TIME_MICROSECONDS, 40) /**< ADC Reference Tension */
#define ADC_CHANNEL_ID 1 /**< ADC Channel ID */

/* This is the actual nRF ANx input to use. Note that a channel can be assigned to any ANx. In fact a channel can */
/*    be assigned to two ANx, when differential reading is set (one ANx for the positive signal and the other one for the negative signal) */  
/* Note also that the configuration of differnt channels is completely independent (gain, resolution, ref voltage, ...) */
#define ADC_CHANNEL_INPUT NRF_SAADC_INPUT_AIN1 /**< ADC INPUT PIN */

#define BUFFER_SIZE 1 /**< Buffer Size */

/* ADC channel configuration */
/**
 *
 * Struct to ADC configuration
 *
*/
static const struct adc_channel_cfg my_channel_cfg = {
	.gain = ADC_GAIN,
	.reference = ADC_REFERENCE,
	.acquisition_time = ADC_ACQUISITION_TIME,
	.channel_id = ADC_CHANNEL_ID,
	.input_positive = ADC_CHANNEL_INPUT
};

void adc_config(void);
uint16_t adc_sample(void);

#endif // _ADC_H