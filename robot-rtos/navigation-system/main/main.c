/*
 * Lib C
 */
#include <stdio.h>
#include <stdint.h>  
#include <string.h>

/*
 * FreeRTOS
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

/*
 * Drivers
 */
#include "driver/gpio.h"
#include "ultrasonic.h"

/*
 * logs
 */
#include "esp_log.h"

/*
 * definiçãos de GPIOs
 */

#define LED_GPIO    15  //Led_uso_geral D15
#define ECHO_GPIO   26  //S1 D26
#define TRIGG_GPIO  25  //S2 D25

/*
 * definições Gerais
 */
static const char * TAG = "MAIN-ROBOT: ";

#define CMD_MEASURE	300
#define MAX_DISTANCE_CM 500 // 5m max
#define DEGUG 1

/*
 * Quees
 */
QueueHandle_t XQuee_ultrasonic;

typedef struct {
	uint16_t command;
	uint32_t distance;
	TaskHandle_t taskHandle;
} CMD_t;

typedef struct {
	uint32_t dis_meters;
	uint32_t dis_cent;
	TaskHandle_t taskHandle;
} dis_ultrasonico;


void ultrasonic()
{
    	dis_ultrasonico dist_sensor;
	CMD_t cmdBuf;
	cmdBuf.command = CMD_MEASURE;
	cmdBuf.taskHandle = xTaskGetCurrentTaskHandle();

	ultrasonic_sensor_t sensor = {
		.trigger_pin = TRIGG_GPIO,
		.echo_pin = ECHO_GPIO
	};

	ultrasonic_init(&sensor);    

	while (true) {
		uint32_t distance;
		esp_err_t res = ultrasonic_measure_cm(&sensor, MAX_DISTANCE_CM, &distance);
		if (res != ESP_OK) {
			printf("Error: ");
			switch (res) {
				case ESP_ERR_ULTRASONIC_PING:
					printf("Cannot ping (device is in invalid state)\n");
					break;
				case ESP_ERR_ULTRASONIC_PING_TIMEOUT:
					printf("Ping timeout (no device found)\n");
					break;
				case ESP_ERR_ULTRASONIC_ECHO_TIMEOUT:
					printf("Echo timeout (i.e. distance too big)\n");
					break;
				default:
					printf("%d\n", res);
			}
		} else {
			ESP_LOGI(TAG,"Send Distance: %d cm, %.02f m\n", distance, distance / 100.0);
			cmdBuf.distance = distance;
			xQueueSend(XQuee_ultrasonic, &cmdBuf, portMAX_DELAY);

            		dist_sensor.dis_cent = distance;
            		dist_sensor.dis_meters = distance / 100.0;
		}
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}    
}

void collision()
{
	CMD_t cmdBuf;
	uint8_t ascii[30];

		for(;;)
		{
			xQueueReceive( XQuee_ultrasonic, &cmdBuf, portMAX_DELAY ); 	

			strcpy((char*)ascii, "Ultrasonic DISTANCE");
			sprintf((char*)ascii, "%d cm",cmdBuf.distance );
			
			
			if( DEGUG ) 
			{
				ESP_LOGI(TAG,"\n\nDistance msg \ 
									value: %s", ascii);
			}

			vTaskDelay( 10/portTICK_PERIOD_MS );	
		}
	vTaskDelete(NULL);
}

void app_main(void)
{

	if( (XQuee_ultrasonic = xQueueCreate( 10, sizeof(CMD_t)) ) == NULL )
	{
		ESP_LOGI( TAG, "error - nao foi possivel alocar XQuee_ultrasonic.\n" );
		return;
	} 

    if( ( xTaskCreate( ultrasonic, "ultrasonic", 2048, NULL, 5, NULL )) != pdTRUE )
	{
		ESP_LOGI( TAG, "error - nao foi possivel alocar ultrasonic.\n" );	
		return;		
	}    

    if( ( xTaskCreate( collision, "collision", 2048, NULL, 5, NULL )) != pdTRUE )
	{
		ESP_LOGI( TAG, "error - nao foi possivel alocar collision.\n" );	
		return;		
	}       

}
