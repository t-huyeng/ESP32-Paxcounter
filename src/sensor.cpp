// Basic Config
#include "globals.h"
#include "sensor.h"

#include "gy21read.h"

// Local logging tag
static const char TAG[] = __FILE__;

#define SENSORBUFFER                                                           \
  10 // max. size of user sensor data buffer in bytes [default=20]

void sensor_init(void) {
  // this function is called during device startup
  // put your user sensor initialization routines here
  gy21_init(GY21_ADDRESS);
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
    temperature = gy21_read_temperature(GY21_ADDRESS);
    humidity = gy21_read_humidity(GY21_ADDRESS);
    ESP_LOGD(TAG, "GY21: Temperature: %f", temperature);
    ESP_LOGD(TAG, "GY21: Humidity: %f", humidity);
    // TODO send data
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
    buf[0] = length;
    buf[1] = 0x01;
    buf[2] = 0x02;
    buf[3] = 0x03;
    break;
  }

  return buf;
}