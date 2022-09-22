#include "esp_wifi.h"
#include "esp_event.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "freertos/event_groups.h"
#include <string.h>
#include "driver/gpio.h"

#include <esp_http_server.h>

#include "wifi_robot.h"
#include "html.h"
static const char * TAG = "MAIN-WIFI: ";

static EventGroupHandle_t wifi_event_group;
const int WIFI_CONNECTED_BIT = BIT0;

static esp_err_t event_handler(void *ctx, system_event_t *event)

#define BLINK_GPIO 15
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


void wifi_config()
{
    wifi_event_group = xEventGroupCreate();
    wifi_init_sta();
}

void open_socket(int * sock_var, int * status_var)
{
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in serverAddress;

    serverAddress.sin_family = AF_INET;
    inet_pton(AF_INET, SERVER_IP, &serverAddress.sin_addr.s_addr);
    serverAddress.sin_port = htons(SERVER_PORT);

    * status_var =  connect(sock, (struct sockaddr *)&serverAddress, sizeof(struct sockaddr_in));
    * sock_var = sock;
}


static  esp_err_t robot_get_handler(httpd_req_t *req)
{
    char*  buf;
    size_t buf_len;

    buf_len = httpd_req_get_url_query_len(req) + 1;

    if (buf_len > 1) 
    {
        buf = malloc(buf_len);

        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) 
        {
            ESP_LOGI(TAG, "Query Parameter found %s", buf);
            char query_param[32];
            if (httpd_query_key_value(buf, "LED1", query_param, sizeof(query_param)) == ESP_OK) 
            {
                ESP_LOGI(TAG, "Value of query parameter => LED1=%s", query_param);
                gpio_pad_select_gpio( BLINK_GPIO );
                gpio_set_direction( BLINK_GPIO, GPIO_MODE_OUTPUT );

                if (strcmp(query_param, "ON") == 0)
                {
                    gpio_set_level( BLINK_GPIO, 1 );
                }
                else
                {
                    gpio_set_level( BLINK_GPIO, 0 );
                }
                
                
            } 
        }
        free(buf);
    }
    const char* resp_str = (const char*) req->user_ctx;

    httpd_resp_send(req, resp_str, strlen(resp_str));

    if (httpd_req_get_hdr_value_len(req, "Host") == 0) 
    {
        ESP_LOGI(TAG, "Request headers lost");
    }
    return ESP_OK;
}

httpd_uri_t robot = 
{
    .uri        = "/",
    .method     = HTTP_GET,
    .handler    = robot_get_handler,
    .user_ctx   = HTML_PAGE 
};

static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) 
    {
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &robot);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

void http_server_init()
{
    start_webserver();
}