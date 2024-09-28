#include "esp_log.h"
#include <lwip/sockets.h>
#include <esp_http_server.h>

/*
Debug
*/
#undef  ESP_ERROR_CHECK
#define ESP_ERROR_CHECK(x)   do { esp_err_t rc = (x); if (rc != ESP_OK) { ESP_LOGE("err", "esp_err_t = %d", rc); assert(0 && #x);} } while(0);

/*
API 
*/
#define SERVER_IP "192.168.0.12" 
#define SERVER_PORT 5000 

/*
Route172.16.54.150 - - [13/Sep/2022 21:52:54] "POST /connection-sensors?field1=34&shdjdj=gdhshs HTTP/1.1" 200 -r
*/
#define EXAMPLE_ESP_WIFI_SSID "luiz" 
#define EXAMPLE_ESP_WIFI_PASS "rm80721"   

extern void wifi_config( void );
extern void open_socket(int * sock_var, int * status_var);
extern void http_server_init();