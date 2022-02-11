/* sail_adc.h
 * Header file for the ADC driver module for the autonomous sailboat project.
 * Created on August 11, 2016.
 * Created by Thomas Gwynne-Timothy & Julia Sarty.
 */

#ifndef SAIL_ADC_H_
#define SAIL_ADC_H_

typedef enum ADC_ChannelIDs {
	ADC_SAIL,
	ADC_RUDDER,
	ADC_NUM_CHANNELS
} ADC_ChannelID;

/* ADC_Init
 * Initialize the ADC module for reading voltage at each potentiometer.
 * Status:
 *   - STATUS_OK - the ADC module was initialized successfully
 *   - STATUS_ERR_DENIED - the ADC module could not be initialized
 */
enum status_code ADC_Init(void);


/* ADC_GetReading
 * Get a voltage reading from the specified ADC channel.
 * Input:
 *   - id - the channel from which to get the reading
 *   - reading - pointer to the location at which the reading should be stored
 * Status:
 *   - STATUS_OK - the reading was successful
 *   - STATUS_ERR_NOT_INITIALIZED - the ADC module was not initialized 
 *   - STATUS_ERR_BAD_ADDRESS - the provided pointer was NULL
 */
enum status_code ADC_GetReading(ADC_ChannelID id, double *reading);

#endif // SAIL_ADC_H_
