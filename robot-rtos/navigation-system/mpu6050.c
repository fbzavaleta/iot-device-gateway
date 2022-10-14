/*
Drivers
*/
#include "driver/gpio.h"
#include <driver/i2c.h>
#include "esp_random.h"

/*
Freertos
*/
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "freertos/queue.h"
#include "freertos/event_groups.h"

/*
Wifi
*/
#include "esp_wifi.h"
#include "esp_event.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include <lwip/sockets.h>
#include "nvs_flash.h"

/*
Utilitarios
*/
#include <math.h>
#include <esp_log.h>
#include <errno.h>

#include "sdkconfig.h"

/*
Registradores
*/
#define PIN_SDA 21
#define PIN_CLK 22
#define BLINK_GPIO 2
#define I2C_ADDRESS 0x68 // I2C address of MPU6050 without AD0


#define MPU6050_ACCEL_XOUT_H 0x3B
#define MPU6050_PWR_MGMT_1   0x6B

#define SERVER_IP "184.106.153.149"
#define SERVER_PORT 80
#define EXAMPLE_ESP_WIFI_SSID 	"nac"
#define EXAMPLE_ESP_WIFI_PASS 	"nacwifi123"

static const char * TAG = "MAIN: ";

/*
 * The following registers contain the primary data we are interested in
 * 0x3B MPU6050_ACCEL_XOUT_H
 * 0x3C MPU6050_ACCEL_XOUT_L
 * 0x3D MPU6050_ACCEL_YOUT_H
 * 0x3E MPU6050_ACCEL_YOUT_L
 * 0x3F MPU6050_ACCEL_ZOUT_H
 * 0x50 MPU6050_ACCEL_ZOUT_L
 * 0x41 MPU6050_TEMP_OUT_H
 * 0x42 MPU6050_TEMP_OUT_L
 * 0x43 MPU6050_GYRO_XOUT_H
 * 0x44 MPU6050_GYRO_XOUT_L
 * 0x45 MPU6050_GYRO_YOUT_H
 * 0x46 MPU6050_GYRO_YOUT_L
 * 0x47 MPU6050_GYRO_ZOUT_H
 * 0x48 MPU6050_GYRO_ZOUT_L
 */

static char tag[] = "mpu6050";

#undef ESP_ERROR_CHECK
#define ESP_ERROR_CHECK(x)   do { esp_err_t rc = (x); if (rc != ESP_OK) { ESP_LOGE("err", "esp_err_t = %d", rc); assert(0 && #x);} } while(0);
#define DEBUG 		1

i2c_config_t conf;
i2c_cmd_handle_t cmd;

uint8_t data[14];
uint8_t data2[14];

uint32_t accel_x;
uint32_t accel_y;
uint32_t accel_z;


/*
queues
*/
static EventGroupHandle_t wifi_event_group;
const int WIFI_CONNECTED_BIT = BIT0;

typedef struct xData {
 	int sock; 
 	uint32_t x_accel; 
	uint32_t y_accel; 
	uint32_t z_accel; 	
} xSocket_t;

void Task_Socket ( void * pvParameter );
void Task_SendReceive ( void * pvParameter );

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
		case SYSTEM_EVENT_STA_START:
			esp_wifi_connect();
			break;
		case SYSTEM_EVENT_STA_GOT_IP:
			ESP_LOGI(TAG, "got ip:%s", ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
			xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
			break;
		case SYSTEM_EVENT_STA_DISCONNECTED:
			esp_wifi_connect();
			xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
			break;
		default:
			break;
    }
    return ESP_OK;
}
void wifi_init_sta( void )
{
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");
    ESP_LOGI(TAG, "connect to ap SSID:%s password:%s",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
}


void init_mpu6050()
{
	ESP_LOGD(tag, ">>Inicializando mpu6050");
	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = PIN_SDA;
	conf.scl_io_num = PIN_CLK;
	conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
	conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
	conf.master.clk_speed = 100000;	

	ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &conf));
	ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));	

	vTaskDelay(200/portTICK_PERIOD_MS);    

	cmd = i2c_cmd_link_create();
	ESP_ERROR_CHECK(i2c_master_start(cmd));
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_WRITE, 1));
	i2c_master_write_byte(cmd, MPU6050_ACCEL_XOUT_H, 1);
	ESP_ERROR_CHECK(i2c_master_stop(cmd));
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);

	cmd = i2c_cmd_link_create();
	ESP_ERROR_CHECK(i2c_master_start(cmd));
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_WRITE, 1));
	i2c_master_write_byte(cmd, MPU6050_PWR_MGMT_1, 1);
	i2c_master_write_byte(cmd, 0, 1);
	ESP_ERROR_CHECK(i2c_master_stop(cmd));
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);

	vTaskDelay(100/portTICK_PERIOD_MS);
}
void task_mpu6050() {

	for (;;)
	{
		cmd = i2c_cmd_link_create();
		ESP_ERROR_CHECK(i2c_master_start(cmd));
		ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_WRITE, 1));
		ESP_ERROR_CHECK(i2c_master_write_byte(cmd, MPU6050_ACCEL_XOUT_H, 1));
		ESP_ERROR_CHECK(i2c_master_stop(cmd));
		ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS));
		i2c_cmd_link_delete(cmd);

		cmd = i2c_cmd_link_create();
		ESP_ERROR_CHECK(i2c_master_start(cmd));
		ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_READ, 1));

		ESP_ERROR_CHECK(i2c_master_read_byte(cmd, data,   0));
		ESP_ERROR_CHECK(i2c_master_read_byte(cmd, data+1, 0));
		ESP_ERROR_CHECK(i2c_master_read_byte(cmd, data+2, 0));
		ESP_ERROR_CHECK(i2c_master_read_byte(cmd, data+3, 0));
		ESP_ERROR_CHECK(i2c_master_read_byte(cmd, data+4, 0));
		ESP_ERROR_CHECK(i2c_master_read_byte(cmd, data+5, 1));

		//i2c_master_read(cmd, data, sizeof(data), 1);
		ESP_ERROR_CHECK(i2c_master_stop(cmd));
		ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS));
		i2c_cmd_link_delete(cmd);

		accel_x = (data[0] << 8) | data[1];
		accel_y = (data[2] << 8) | data[3];
		accel_z = (data[4] << 8) | data[5];	

		vTaskDelay(500/portTICK_PERIOD_MS);		
	}

	vTaskDelete(NULL);
}



void Task_Socket ( void * pvParameter ) 
{
    int rc; 
	xSocket_t xSocket;

	if( DEBUG )
		ESP_LOGI(TAG, "Task_Socket run ...\r\n");

    for(;;) 
    {


		int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if( DEBUG )
			ESP_LOGI(TAG, "socket: rc: %d", sock);

		struct sockaddr_in serverAddress;
		serverAddress.sin_family = AF_INET;

		/**
		 * Registra o endereço IP e PORTA do servidor;
		 */
		inet_pton(AF_INET, SERVER_IP, &serverAddress.sin_addr.s_addr);
		serverAddress.sin_port = htons(SERVER_PORT);

		/**
		 * Tenta realiza a conexão socket com o servidor; 
		 * Caso a conexão ocorra com sucesso, será retornado 0, caso contrário, -1;
		 */
		rc = connect(sock, (struct sockaddr *)&serverAddress, sizeof(struct sockaddr_in));

		if( DEBUG )
			ESP_LOGI(TAG, "Status Socket: %d", rc);

		if( rc == -1 ) 
		{
			if( DEBUG )
				ESP_LOGI(TAG, "xiii Socket Error: %d", sock);

			/**
			 * Aguarda 5 segundos antes de abrir um novo socket;
			 */
			for( int i = 1; i <= 25; ++i )//rm_1
			{
				if( DEBUG )
					ESP_LOGI(TAG, "timeout: %d", 5-i);

				//Envio dos dados para a Quee
				if (xQueueSend(xQueue_timepout_x, &accel_x, (10/portTICK_PERIOD_MS)) == pdPASS)
				{
					ESP_LOGI(TAG, "enviado no buffer: %d", accel_x);
				}
				vTaskDelay(1000/portTICK_PERIOD_MS);
			}

			continue; 
		} 

		xSocket.sock = sock; 
		xSocket.x_accel = accel_x; 
		xSocket.y_accel = accel_y; 
		xSocket.z_accel = accel_z;

	    xTaskCreate( Task_SendReceive, "TaskSendReceive", 10000, (void*)&(xSocket), 5, NULL );

	}

	vTaskDelete( NULL );
}

void Task_SendReceive ( void * pvParameter ) 
{
	int rec_offset = 0; 
	int total =	1*1024; 
	char *buffer = pvPortMalloc( total );
	if( buffer == NULL ) 
	{
		if( DEBUG )
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
        "X-THINGSPEAKAPIKEY: 7Q0HCR62GSZSX0ET\n"
        "Content-Type: application/x-www-form-urlencoded\n"
        "content-length: ";

	//regate do buffer

	uint32_t data_package[25];

	uint32_t x_timeout_val;

	for (int i = 0; i < 25; i++)
	{
		xQueueReceive(xQueue_timepout_x, &x_timeout_val, portMAX_DELAY);
		data_package[i] = x_timeout_val;

	}

	//escolha o maior valor
	for (int i = 0; i < 25; i++)
	{
		if (data_package[0] < data_package[i])
		{
			data_package[0] =data_package[i];
		}

	}

	char databody[50];
  	sprintf( databody, "{7Q0HCR62GSZSX0ET&field1=%d&field2=%d&field3=%d}", data_package[0], xSocket->y_accel, xSocket->z_accel);
	sprintf( buffer , "%s%d\r\n\r\n%s\r\n\r\n", msg_post, strlen(databody), databody);


	int rc = send( xSocket->sock, buffer, strlen(buffer), 0 );

	if( DEBUG )
		ESP_LOGI(TAG, "Cabecalho HTTP Enviado? rc: %d", rc);

	for(;;)
	{
		ssize_t sizeRead = recv(xSocket->sock, buffer+rec_offset, total-rec_offset, 0);

		if ( sizeRead == -1 ) 
		{
			if( DEBUG )
				ESP_LOGI( TAG, "recv: %d", sizeRead );
			break;
		}



		if ( sizeRead == 0 ) 
		{
			break;
		}

		if( sizeRead > 0 ) 
		{	
			if( DEBUG ) 
		    	ESP_LOGI(TAG, "Socket: %d - Data read (size: %d) was: %.*s", xSocket->sock, sizeRead, sizeRead, buffer);

		   rec_offset += sizeRead; 
		 }

		vTaskDelay( 5/portTICK_PERIOD_MS );
	}

	rc = close(xSocket->sock);

	if( DEBUG )
		ESP_LOGI(TAG, "close: rc: %d", rc); 

	vPortFree( buffer );	

	vTaskDelete(NULL); 
}