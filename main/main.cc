#include <driver/uart.h>
#include <esp_camera.h>
#include <esp_log.h>
#include <esp_system.h>

#include "tensorflow/lite/micro/micro_error_reporter.h"

#include "app_camera_esp.h"

const uart_port_t uart_num = UART_NUM_0;
static const char *TAG = "esp32cam_pet_feeder";

#define DELAY(ms) vTaskDelay(ms / portTICK_PERIOD_MS);

char map_char(uint8_t val);

extern "C" void app_main(void) {
  // Configure UART parameters
  int i = 0;
  uart_set_baudrate(uart_num, 921600);

  if (app_camera_init() == -1) {
    printf("I could not start the camera :sob:\n");
    return;
  }

  while (1) {
    camera_fb_t *pic = esp_camera_fb_get();

    int len = pic->width * pic->height;
    for (int i = 0; i < len; i++) {
      printf("%c", map_char(pic->buf[i]));

      if (i % pic->width == 0) {
        puts("");
      }
    }
    puts("\n\n\n\n\n\n");

    esp_camera_fb_return(pic);
    DELAY(250);
  }
}

char map_char(uint8_t val) {
  /* uint8_t pallete_size = 10; */
  char c;

  if (val > 229) {
    c = ' ';
  } else if (val > 204) {
    c = '.';
  } else if (val > 178) {
    c = ':';
  } else if (val > 153) {
    c = '-';
  } else if (val > 127) {
    c = '=';
  } else if (val > 102) {
    c = '+';
  } else if (val > 76) {
    c = '*';
  } else if (val > 51) {
    c = '#';
  } else if (val > 25) {
    c = '%';
  } else {
    c = '@';
  }

  return c;
}
