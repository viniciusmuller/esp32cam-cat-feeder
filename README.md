# Dependencies
- Python 3
- ESP IDF v4.3.2 folder at `$HOME/esp-idf-v4.3.2`

# Setup
- `idf.py menuconfig`
- Enable PSRAM support (*Component Config -> ESP32-specific -> Support for
    external, SPI-connected RAM*)
- Go under the *Camera Pins* section and select the camera that you're using
- Increase flash size from 2MB to 4MB (*Serial flasher config -> Flash size*)
- Enable usage of custom partition table (*Partition Table -> Custom partition
    table CSV*)
<!-- - Set compiler optimization mode to speed -->

# Usage
- `idf.py build`
- `idf.py flash`
- `idf.py monitor`
