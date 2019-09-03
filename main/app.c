#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <esp_log.h>
#include <esp_log_internal.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "sdkconfig.h"

#include "sdkconfig.h"
#include "driver/gpio.h"
#include "mfrc522.h"
#include "spi.h"

#define PIN_NUM_MISO GPIO_NUM_19
#define PIN_NUM_MOSI GPIO_NUM_23
#define PIN_NUM_CLK  GPIO_NUM_18
#define PIN_NUM_CS   GPIO_NUM_5
#define PIN_NUM_RST  GPIO_NUM_4

static unsigned char CardID[5];

void nfc_task(void *pvParameter)
{
    spi_init(PIN_NUM_CLK, PIN_NUM_MOSI, PIN_NUM_MISO);  // Init Driver SPI
    MFRC522_Init(PIN_NUM_RST, PIN_NUM_CS); // Init MFRC522

    while (1)
    {
        if (MFRC522_Check(CardID) == MI_OK)
        {
            ESP_LOGI("MFRC", "[%02x-%02x-%02x-%02x-%02x] \r\n",
                     CardID[0], CardID[1], CardID[2], CardID[3], CardID[4]);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void app_main()
{
    xTaskCreate(&nfc_task, "nfc_task", 4096, NULL, 4, NULL);
}
