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
#include "wifi_robot.h"
#include "nvs_flash.h"
#include "qre1113.h"

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

void http_Socket ( void * pvParameter );
void http_SendReceive ( void * pvParameter );

#define CMD_MEASURE	300
#define MAX_DISTANCE_CM 500 // 5m max
#define DEGUG 1

/*
 * Quees
 */
QueueHandle_t XQuee_ultrasonic;
QueueHandle_t XQuee_comunications;
QueueHandle_t XQuee_navigation;


typedef struct {
	uint16_t command;
	uint32_t distance;
	TaskHandle_t taskHandle;
} CMD_t;

typedef struct xData {
 	int sock; 
 	uint32_t distance;
} xSocket_t;


void ultrasonic()
{
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

void http_Socket(void * pvParameter)
{
	int rc; 
	xSocket_t xSocket;
	uint32_t displacement_x;
	
	for (;;)
	{
		int sock;

		xQueueReceive( XQuee_comunications, &displacement_x, portMAX_DELAY ); 
		ESP_LOGI(TAG, "Comunications recibe: %d", displacement_x);

		open_socket(&sock, &rc);
		ESP_LOGI(TAG, "Status Socket: %d", rc);

		if (rc == -1)
		{
			ESP_LOGI(TAG, "error on Socket: %d", rc);
			for( int i = 1; i <= 5; ++i )
			{	
				ESP_LOGI(TAG, "timeout: %d", 5-i);
				vTaskDelay( 1000/portTICK_PERIOD_MS );
			}
			continue;			
		}

		xSocket.sock = sock;
		xSocket.distance = 10;

		xTaskCreate( http_SendReceive, "http_SendReceive", 10000, (void*)&(xSocket), 5, NULL );
	}
	vTaskDelete(NULL);
	
}

void http_SendReceive(void * pvParameter)
{
	int rec_offset = 0; 
	int total =	1*1024; 
	char *buffer = pvPortMalloc( total );
	if( buffer == NULL ) 
	{
		
		ESP_LOGI(TAG, "pvPortMalloc Error\r\n");
		vTaskDelete(NULL); 	  
		return;
	 }
	 
	/**
	 * Recebe o Socket da conexão com o servidor web;
	 */
    xSocket_t * xSocket = (xSocket_t*) pvParameter;
	
	const char * msg_post = \

        "POST /update HTTP/1.1\n"
        "Host: api.thingspeak.com\n"
        "Connection: close\n"
        "X-THINGSPEAKAPIKEY: XNLVSMMPW8LO2M7I\n"
        "Content-Type: application/x-www-form-urlencoded\n"
        "content-length: ";
		
	char databody[50];
  	sprintf( databody, "{XNLVSMMPW8LO2M7I&field1=%d}", xSocket->distance);
	sprintf( buffer , "%s%d\r\n\r\n%s\r\n\r\n", msg_post, strlen(databody), databody);

  
	int rc = send( xSocket->sock, buffer, strlen(buffer), 0 );

	ESP_LOGI(TAG, "HTTP Enviado? rc: %d", rc);
	
	for(;;)
	{
		ssize_t sizeRead = recv(xSocket->sock, buffer+rec_offset, total-rec_offset, 0);
		
		if ( sizeRead == -1 ) 
		{
			ESP_LOGI( TAG, "recv: %d", sizeRead );
		}

		if ( sizeRead == 0 ) 
		{
			break;
		}

		if( sizeRead > 0 ) 
		{	
			ESP_LOGI(TAG, "Socket: %d - Data read (size: %d) was: %.*s", xSocket->sock, sizeRead, sizeRead, buffer);
		   
		   rec_offset += sizeRead;
		 }

		vTaskDelay( 5/portTICK_PERIOD_MS );
	}
	
	rc = close(xSocket->sock);
	
	ESP_LOGI(TAG, "close: rc: %d", rc); 
	
	vPortFree( buffer );	

	vTaskDelete(NULL); 	
}

void setup_sensor()
{
	uint32_t displacement_x;

	bool utrasonic_sensor;
	bool reflex_sensor_left;
	bool reflex_sensor_rigth;

	for (;;)
	{
		displacement_x = 20;
		xQueueSend(XQuee_comunications, &displacement_x, portMAX_DELAY);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
	vTaskDelete(NULL);
}

/*
checkpoint 2C - adicionar suporte para o chanel3
*/

void read_qre()
{
	ad_struct reflex_channel;

	for (;;)
	{
		reflex_channel = analogic_readx();
		xQueueSend(XQuee_navigation, &reflex_channel, portMAX_DELAY);
		ESP_LOGI(TAG, "Read analogic send to quee mV %d and %d", reflex_channel.adc0_chars, reflex_channel.adc3_chars);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
	vTaskDelete(NULL);
}

void drive()
{
	ad_struct reflex_channel;

	for (;;)
	{
		xQueueReceive( XQuee_navigation, &reflex_channel, portMAX_DELAY ); 
		ESP_LOGI(TAG, "Read analogic recibe mV %d and %d", reflex_channel.adc0_chars, reflex_channel.adc3_chars);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
	vTaskDelete(NULL);
}

void app_main(void)
{
	esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) 
	{
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
	ESP_ERROR_CHECK(ret);
	wifi_config();
	http_server_init();
	analogic_setupx();
	wifi_event_group = xEventGroupCreate();
	wifi_init_sta();
	init_mpu6050();

	if ( (xQueue_timepout_x = xQueueCreate(25, sizeof(uint32_t))) ==NULL)
	{
         ESP_LOGI( TAG, "error - Nao foi possivel alocar a quee.\r\n" );  
         return;  
	}

	if( (XQuee_navigation = xQueueCreate( 10, sizeof(ad_struct)) ) == NULL )
	{
		ESP_LOGI( TAG, "error - nao foi possivel alocar XQuee_navigation.\n" );
		return;
	} 

	if( (XQuee_ultrasonic = xQueueCreate( 10, sizeof(CMD_t)) ) == NULL )
	{
		ESP_LOGI( TAG, "error - nao foi possivel alocar XQuee_ultrasonic.\n" );
		return;
	} 

	if( (XQuee_comunications = xQueueCreate( 10, sizeof(uint32_t)) ) == NULL )
	{
		ESP_LOGI( TAG, "error - nao foi possivel alocar XQuee_comunications.\n" );
		return;
	} 	

    if( ( xTaskCreate( drive, "drive", 2048, NULL, 5, NULL )) != pdTRUE )
	{
		ESP_LOGI( TAG, "error - nao foi possivel alocar drive.\n" );	
		return;		
	}  

    if( ( xTaskCreate( ultrasonic, "ultrasonic", 2048, NULL, 5, NULL )) != pdTRUE )
	{
		ESP_LOGI( TAG, "error - nao foi possivel alocar ultrasonic.\n" );	
		return;		
	}  

    if( ( xTaskCreate( read_qre, "read_qre", 2048, NULL, 5, NULL )) != pdTRUE )
	{
		ESP_LOGI( TAG, "error - nao foi possivel alocar read_qre.\n" );	
		return;		
	}    

    if( ( xTaskCreate( collision, "collision", 2048, NULL, 5, NULL )) != pdTRUE )
	{
		ESP_LOGI( TAG, "error - nao foi possivel alocar collision.\n" );	
		return;		
	}      

    if( ( xTaskCreate( http_Socket, "http_Socket", 2048, NULL, 5, NULL )) != pdTRUE )
	{
		ESP_LOGI( TAG, "error - nao foi possivel alocar http_Socket.\n" );	
		return;		
	}       

    if( ( xTaskCreate( setup_sensor, "setup_sensor", 2048, NULL, 5, NULL )) != pdTRUE )
	{
		ESP_LOGI( TAG, "error - nao foi possivel alocar setup_sensor.\n" );	
		return;		
	}

	if( xTaskCreate( Task_Socket, "task_socket", 4048, NULL, 5, NULL ) != pdTRUE )
	{
		ESP_LOGI( TAG, "error - nao foi possivel alocar Task_Socket.\n" );
		return;
	}

	if( xTaskCreate( task_mpu6050, "task_mpu6050", 4048, NULL, 5, NULL ) != pdTRUE )
	{
		ESP_LOGI( TAG, "error - nao foi possivel alocar task_mpu6050.\n" );
		return;
	}       

}