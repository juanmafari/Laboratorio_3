#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "tomicro.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/timers.h"

static const int RX_BUF_SIZE = 1024;

#define TXD_PIN (UART_PIN_NO_CHANGE)
#define RXD_PIN (UART_PIN_NO_CHANGE)

#define UART UART_NUM_0

int num = 0;

int timeTimer = 500 ;


QueueHandle_t handleCola;
TimerHandle_t handleTimer;
//TimerHandle_t azulTimer;
//TimerHandle_t verdeTimer;

void * puntazo;

uint8_t* data;
uint8_t* dataTime;

char redVal[] = "FF0000";
char blueVal[] = "0000FF";
char greenVal[] = "00FF00";

char* rojo = redVal;
char* azul = blueVal;
char* verde = greenVal;
            
void init(void) 
{
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    // We won't use a buffer for sending data.
    uart_driver_install(UART, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART, &uart_config);
    uart_set_pin(UART, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

void sendQueue(TimerHandle_t xTimer ){
    static const char *SEND_TAG = "send_TASK";
    esp_log_level_set(SEND_TAG, ESP_LOG_INFO);
    ESP_LOGI(SEND_TAG, "llegue a aca");
    puntazo = pvTimerGetTimerID(xTimer); 
    xQueueSendToBack(handleCola, puntazo, 0);
    
    
}

void ATaskFunction( void *pvParameters ){
    int32_t lTime = 5000;

    for( ;; )
    {
        white();
        delay_ms(lTime);
        clearblack();
        delay_ms(lTime);
        vTaskDelete( NULL );
    }
    
        
}

static void BTaskFunction(void *arg)
{
    char* Txdatapru = (char*) malloc(100);
    static const char *RX_TASK_TAG = "RX_TASK";
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
    data = (uint8_t*) malloc(RX_BUF_SIZE+1);
    dataTime = (uint8_t*) malloc(RX_BUF_SIZE+1);
    while (1) {
        const int rxBytes = uart_read_bytes(UART, data, RX_BUF_SIZE, 1000 / portTICK_PERIOD_MS);
        if (rxBytes > 0) {
            data[rxBytes] = '\0';
            ESP_LOGI(RX_TASK_TAG, "Read %d bytes: '%s' ", rxBytes, data);
            const int timeBytes = uart_read_bytes(UART, dataTime, RX_BUF_SIZE, 1000 / portTICK_PERIOD_MS);
            if (timeBytes > 0)
            {
                dataTime[timeBytes] = '\0';
                ESP_LOGI(RX_TASK_TAG, "time: '%s' ms",  dataTime);
                //dataPointer = data;
                timeTimer = atoi((char*) dataTime);
                ESP_LOGI(RX_TASK_TAG, "time: '%d' ms",  timeTimer);
                if (strcmp((char*) data, "red") == 0)
                {
                    if (timeTimer == 0)
                    {
                        static const char *SEND_TAG = "send_TASK";
                        esp_log_level_set(SEND_TAG, ESP_LOG_INFO);
                        ESP_LOGI(SEND_TAG, "llegue a aca");
                        xQueueSendToBack(handleCola, rojo, 0);
                    }
                    else{
                        handleTimer = xTimerCreate("sendQueue", pdMS_TO_TICKS(timeTimer), pdFALSE, rojo, sendQueue);
                        xTimerStart(handleTimer, pdMS_TO_TICKS(0));
                    }
                }
                else if (strcmp((char*) data, "blue") == 0)
                {
                    if (timeTimer == 0)
                    {
                        static const char *SEND_TAG = "send_TASK";
                        esp_log_level_set(SEND_TAG, ESP_LOG_INFO);
                        ESP_LOGI(SEND_TAG, "llegue a aca");
                        xQueueSendToBack(handleCola, azul, 0);
                    }
                    else{
                        
                        handleTimer = xTimerCreate("sendQueue", pdMS_TO_TICKS(timeTimer), pdFALSE, azul, sendQueue);
                        xTimerStart(handleTimer, pdMS_TO_TICKS(0));
                    }
                }
                else if (strcmp((char*) data, "green") == 0)
                {
                    if (timeTimer == 0)
                    {
                        static const char *SEND_TAG = "send_TASK";
                        esp_log_level_set(SEND_TAG, ESP_LOG_INFO);
                        ESP_LOGI(SEND_TAG, "llegue a aca");
                        xQueueSendToBack(handleCola, verde, 0);
                    }
                    else{
                        
                        handleTimer = xTimerCreate("sendQueue", pdMS_TO_TICKS(timeTimer), pdFALSE, verde, sendQueue);
                        xTimerStart(handleTimer, pdMS_TO_TICKS(0));
                    }
                }
                else {
                    sprintf (Txdatapru, " %s no identificado como color \r\n",(data));
                    uart_write_bytes(UART, Txdatapru, strlen(Txdatapru));
                }   
            } 
        }
    }
    free(data);
    free(dataTime);
}



void CTaskFunction(void *pvParameters){
    char* Txdata = (char*) malloc(100);
    char* Txdatazo = (char*) malloc(1024);

    while (1) {
            if (xQueueReceive(handleCola, Txdatazo, (TickType_t) 0) == pdPASS) {
                
                sprintf (Txdata, " %s , son %d bytes \r\n",(Txdatazo),strlen(Txdatazo));
                uart_write_bytes(UART, Txdata, strlen(Txdata));
                
                hextoco(Txdatazo);
                memset(Txdatazo, 0, 1024);
                vTaskDelay(2000 / portTICK_PERIOD_MS);
            }
            else{
                vTaskDelay(2000 / portTICK_PERIOD_MS);
            }
    }
}


void app_main(void)
{
    configure();
    init();
    handleCola = xQueueCreate(10,1024*2);

    xTaskCreate( ATaskFunction, "waitingLED ", 1024*2, NULL, tskIDLE_PRIORITY, NULL );
    xTaskCreate(BTaskFunction, "uart_rx_task", 1024*2, NULL, configMAX_PRIORITIES-1, NULL);
    xTaskCreate(CTaskFunction, "uart_tx_task", 1024*2, NULL, configMAX_PRIORITIES-2, NULL);
}
