// Basic Config
#include "globals.h"
#include "sensor.h"

#define SENSORBUFFER                                                           \
  10 // max. size of user sensor data buffer in bytes [default=20]

#if (HAS_SENSOR_2)
uint16_t duration;
uint16_t distance;
#endif

void sensor_init(void) {
  // this function is called during device startup
  // put your user sensor initialization routines here
  ESP_LOGI(TAG, "sensor_init() called");
  pinMode(TRIGPIN, OUTPUT); // Sets the trigPin as an Output
  pinMode(ECHOPIN, INPUT);  // Sets the echoPin as an Input
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
  hcsrStatus_t hcsrStatus;
  switch (sensor) {
  case 1:
    // insert user specific sensor data frames here
    buf[0] = length;
    buf[1] = 0x01;
    buf[2] = 0x02;
    buf[3] = 0x03;
    break;
  case 2:
    ESP_LOGI(TAG, "Inside Sensor 2");
    digitalWrite(TRIGPIN, LOW);
    delayMicroseconds(2);

    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(TRIGPIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGPIN, LOW);

    // Reads the echoPin, returns the sound wave travel time in microseconds
    duration = pulseIn(ECHOPIN, HIGH);
    ESP_LOGI(TAG, "Measure duration: %d", duration);

    // Calculating the distance
    distance = duration * 0.034 / 2;
    ESP_LOGI(TAG, "Measure distance: %d", distance);

    hcsrStatus = {duration, distance};
    payload.addHCSR(hcsrStatus);
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