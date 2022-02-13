#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include <driver/uart.h>
#include <esp_camera.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_task_wdt.h>

#include "model.h"
#include "app_camera_esp.h"

#define WIDTH 96
#define HEIGHT 96

void rgb565_to_rgb888(uint8_t *buffer, int len);

const uart_port_t uart_num = UART_NUM_0;
static const char *TAG = "esp32cam_pet_feeder";

// TODO: Maybe use these as non-globals
tflite::ErrorReporter* error_reporter = nullptr;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
int inference_count = 0;
constexpr int tensor_arena_size = 2 * 1024 * 1024; // 2MB

#define DELAY(ms) vTaskDelay(ms / portTICK_PERIOD_MS);

char map_char(uint8_t val);

extern "C" void app_main(void) {
  // ------ ESP config ------
  // Configure UART parameters
  uart_set_baudrate(uart_num, 921600);
  // Disable watchdogs
  /* if (ESP_OK != esp_task_wdt_deinit()) { */
  /*   printf("Could not deinitialize watchdog\n"); */
  /*   return; */
  /* } */

  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  model = tflite::GetModel(g_model);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "Model provided is schema version %d not equal "
                         "to supported version %d.",
                         model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }

  // Allocate the arena
  uint8_t *tensor_arena = nullptr;
  tensor_arena = (uint8_t*) heap_caps_malloc(
    tensor_arena_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT
  );

  // Instantiate model
  static tflite::AllOpsResolver resolver;
  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, tensor_arena_size, error_reporter);
  interpreter = &static_interpreter;

  // Allocate tensors
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
    return;
  }

  if (app_camera_init() == -1) {
    return;
  }

  TfLiteTensor* input = interpreter->input(0);

  while (1) {
    uint64_t start = esp_timer_get_time();

    camera_fb_t *pic = esp_camera_fb_get();
    printf("model: %p, image: %d\n", g_model, pic->buf[10]);

    // https://stackoverflow.com/a/64304100
    uint32_t input_ix = 0; // index for the model input

    // tflite model has input shape [batch_size, height, width, channels]
    // which in turn is [1, HEIGHT, WIDTH, 3] three channels because RGB
    // but tflite micro expects flattened 1D array so you can just do this
    for (uint32_t pix = 0; pix < HEIGHT*WIDTH; pix++){
       // Convert from RGB55 to RGB888 and int8 range
       uint16_t color = pic->buf[pix];
       int16_t r = ((color & 0xF800) >> 11)*255/0x1F - 128;
       int16_t g = ((color & 0x07E0) >> 5)*255/0x3F - 128;
       int16_t b = ((color & 0x001F) >> 0)*255/0x1F - 128;

       input->data.int8[input_ix] =   (int8_t) r;
       input->data.int8[input_ix+1] = (int8_t) g;
       input->data.int8[input_ix+2] = (int8_t) b;

       input_ix += 3;
    }

    esp_camera_fb_return(pic);

    if (kTfLiteOk != interpreter->Invoke()) {
      TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed.");
    }

    TfLiteTensor* output = interpreter->output(0);
    printf("class1: %d, class2: %d, class3: %d\n", output->data.int8[0], output->data.int8[1], output->data.int8[2]);

    uint64_t end = esp_timer_get_time();
    printf("Prediction took %llu milliseconds\n", (end - start)/1000);

    /* DELAY(250); */
  }
}
