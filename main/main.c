#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <esp_camera.h>

#include <driver/uart.h>

const uart_port_t uart_num = UART_NUM_0;

#define DELAY(ms) vTaskDelay(ms / portTICK_PERIOD_MS);

void app_main(void)
{
    // Configure UART parameters
    int i = 0;
    uart_set_baudrate(uart_num, 921600);

    while (1) {
        printf("[%d] Hello world!\n", i);
        i++;
        DELAY(10);
    }
}
