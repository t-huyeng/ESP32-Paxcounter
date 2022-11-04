// Basic Config
#include "globals.h"
#include "sensor.h"

#if (HAS_GY21)
#include <HTU2xD_SHT2x_Si70xx.h>

float htValue; // to store T/RH result
byte error;
HTU2xD_SHT2x_SI70xx ht2x(HTU2xD_SENSOR,
                         HUMD_12BIT_TEMP_14BIT); // sensor type, resolution

#endif // HAS_GY21

// Local logging tag
static const char TAG[] = __FILE__;

#define SENSORBUFFER                                                           \
  10 // max. size of user sensor data buffer in bytes [default=20]

void sensor_init(void) {
  // this function is called during device startup
  // put your user sensor initialization routines here
  if (ht2x.begin() !=
      true) // reset sensor, set heater off, set resolution, check power
            // (sensor doesn't operate correctly if VDD < +2.25v)
  {
    ESP_LOGE(TAG, "HTU2xD/SHT2x not connected, fail or VDD < +2.25v");
  } else {
    ESP_LOGE(TAG, "HTU2xD/SHT2x/GY21 found");
  }
}

uint8_t sensor_mask(uint8_t sensor_no) {
  switch (sensor_no) {
  case 0:
    return (uint8_t)COUNT_DATA;
  case 1:
    return (uint8_t)SENSOR1_DATA;
  case 2:
    return (uint8_t)SENSOR2_DATA;
  case 3:
    return (uint8_t)SENSOR3_DATA;
  case 4:
    return (uint8_t)BATT_DATA;
  case 5:
    return (uint8_t)GPS_DATA;
  case 6:
    return (uint8_t)MEMS_DATA;
  case 7:
    return (uint8_t)RESERVED_DATA;
  default:
    return 0;
  }
}

uint8_t *sensor_read(uint8_t sensor) {
  static uint8_t buf[SENSORBUFFER] = {0};
  uint8_t length = 3;
  double temperature, humidity;
  switch (sensor) {
  case 1:
    // insert user specific sensor data frames here
    buf[0] = length;
    buf[1] = 0x01;
    buf[2] = 0x02;
    buf[3] = 0x03;
    break;
  case 2:

    buf[0] = length;
    buf[1] = 0x01;
    buf[2] = 0x02;
    buf[3] = 0x03;
    break;
  case 3:
    ESP_LOGE(TAG, "Reading Sensor 3, GY21");
    temperature =
        ht2x.readTemperature(); // accuracy +-0.3C in range 0C..60C at  14-bit
    delay(100);
    humidity =
        ht2x.readHumidity(); // accuracy +-2% in range 20%..80%/25C at 12-bit
    ESP_LOGE(TAG, "GY21: Temperature: %f", temperature);
    ESP_LOGE(TAG, "GY21: Humidity: %f", humidity);
    payload.addVoltage(temperature * 100);
    payload.addVoltage(humidity * 100);
    break;
  }

  return buf;
}