idf_component_register(
    SRCS
        "main.cc"
        "model.cc"
        "app_camera_esp.c"
        "constants.cc"
    PRIV_REQUIRES tflite-lib esp32-camera spi_flash
    INCLUDE_DIRS "")

