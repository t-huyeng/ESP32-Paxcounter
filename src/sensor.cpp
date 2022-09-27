// Basic Config
#include "globals.h"
#include "sensor.h"

#if (COUNT_ENS)
#include "payload.h"
#include "corona.h"
extern PayloadConvert payload;
#endif

#if (HAS_SENSORS)
long duration;
int distance;
uint8_t sensorVal;
#endif

// Local logging tag
static const char TAG[] = __FILE__;

#define SENSORBUFFER                                                           \
  10 // max. size of user sensor data buffer in bytes [default=20]

void sensor_init(void) {

  // this function is called during device startup
  // put your user sensor initialization routines here

// pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
// pinMode(echoPin, INPUT); // Sets the echoPin as an Input

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

const int dry = 950; // value for dry sensor
const int wet = 360; // value for wet sensor

uint8_t readHumiditySoil(uint8_t pin)
{
  uint8_t sensorVal = analogRead(pin);

  // Sensor has a range of 239 to 595
  // We want to translate this to a scale or 0% to 100%
  // More info: https://www.arduino.cc/reference/en/language/functions/math/map/
  int percentageHumidity = map(sensorVal, wet, dry, 100, 0);
  return sensorVal;
  // return percentageHumidity;
}


uint8_t *sensor_read(uint8_t sensor) {

  static uint8_t buf[SENSORBUFFER] = {0};
  uint8_t length = 3;

  switch (sensor) {

  case 1:

    // insert user specific sensor data frames here
    // note: Sensor1 fields are used for ENS count, if ENS detection enabled
#if (COUNT_ENS)
    if (cfg.enscount)
      payload.addCount(cwa_report(), MAC_SNIFF_BLE_ENS);
#else
    buf[0] = length;
    buf[1] = 0x01;
    buf[2] = 0x02;
    buf[3] = 0x03;
#endif
    break;

  case 2:


#if (HAS_SENSOR_2)
    // digitalWrite(trigPin, LOW);
    // delayMicroseconds(2);

    // // Sets the trigPin on HIGH state for 10 micro seconds
    // digitalWrite(trigPin, HIGH);
    // delayMicroseconds(10);
    // digitalWrite(trigPin, LOW);

    // // Reads the echoPin, returns the sound wave travel time in microseconds
    // duration = pulseIn(echoPin, HIGH);
    // ESP_LOGI(TAG, "Measure duration: %d", duration);

    // // Calculating the distance
    // distance= duration*0.034/2;
    sensorVal = readHumiditySoil(SOIL_SENSOR_PIN);

    ESP_LOGI(TAG, "Measure Soil moisture: %d", sensorVal);
    payload.addByte(sensorVal);
#else
    buf[0] = length;
    buf[1] = 0x01;
    buf[2] = 0x02;
    buf[3] = 0x03;
#endif
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
