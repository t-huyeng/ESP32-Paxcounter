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

#define SENSORBUFFER                                                           \
  10 // max. size of user sensor data buffer in bytes [default=20]

#if (HAS_SENSOR_2)
uint16_t duration;
float distance;
const int NUM_MEASUREMENTS = 20;
float TEMPERATURE_COMPENSATION = 0.01;
float measurements[NUM_MEASUREMENTS];
int trimPercent = 10;
long MAX_DISTANCE = 400;
#endif

uint16_t get_duration() {
  digitalWrite(TRIGPIN, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(TRIGPIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGPIN, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  // duration = pulseIn(ECHOPIN, HIGH);
  long duration = pulseIn(ECHOPIN, HIGH, MAX_DISTANCE * 58);
  return duration;
}

hcsrStatus_t read_ultrasonic() {
  hcsrStatus_t hcsrStatus;
  // Temperature compensation
  // temperature = getTemperature();
  // TODO use real temperature
  float temperature = 20;
  for (int i = 0; i < NUM_MEASUREMENTS; i++) {
    duration = get_duration();
    // Calculate distance
    float measurement = duration / 58.2;
    // measurement = measurement / (1 + (temperature *
    // TEMPERATURE_COMPENSATION)); ESP_LOGI(TAG, "Measure measurement: %f",
    // measurement);
    measurements[i] = measurement;
  }
  // Sort measurements in ascending order
  std::sort(measurements, measurements + NUM_MEASUREMENTS);

  // Calculate number of measurements to trim
  int trimCount = (trimPercent / 100.0) * NUM_MEASUREMENTS;

  // Trim measurements
  float sum = 0;
  for (int i = trimCount; i < NUM_MEASUREMENTS - trimCount; i++) {
    sum += measurements[i];
  }

  distance = sum / (NUM_MEASUREMENTS - (2 * trimCount));
  ESP_LOGI(TAG, "Measure distance: %f", distance);

  hcsrStatus = {duration, distance};
  return hcsrStatus;
}

void sensor_init(void) {
  // this function is called during device startup
  // put your user sensor initialization routines here
  ESP_LOGI(TAG, "sensor_init() called");
  pinMode(TRIGPIN, OUTPUT); // Sets the trigPin as an Output
  pinMode(ECHOPIN, INPUT);  // Sets the echoPin as an Input
  #if (HAS_GY21)
  if (ht2x.begin() !=
      true) // reset sensor, set heater off, set resolution, check power
            // (sensor doesn't operate correctly if VDD < +2.25v)
  {
    ESP_LOGE(TAG, "HTU2xD/SHT2x not connected, fail or VDD < +2.25v");
  } else {
    ESP_LOGE(TAG, "HTU2xD/SHT2x/GY21 found");
  }
  #endif // HAS_GY21
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
  hcsrStatus_t hcsrStatus;  double temperature, humidity;
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
    hcsrStatus = read_ultrasonic();
    payload.addHCSR(hcsrStatus);
    break;
  case 3:
    #if (HAS_GY21)
    ESP_LOGE(TAG, "Reading Sensor 3, GY21");
    temperature =
        ht2x.readTemperature(); // accuracy +-0.3C in range 0C..60C at  14-bit
    delay(100);
    humidity =
        ht2x.readHumidity(); // accuracy +-2% in range 20%..80%/25C at 12-bit
    ESP_LOGE(TAG, "GY21: Temperature: %f", temperature);
    ESP_LOGE(TAG, "GY21: Humidity: %f", humidity);
    payload.addTempHum(temperature, humidity);
    #endif // HAS_GY21
    break;
  }

  return buf;
}