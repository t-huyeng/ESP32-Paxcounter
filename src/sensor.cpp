// Basic Config
#include "globals.h"
#include "sensor.h"
#include <math.h>

#if (HAS_SENSOR_1)

const int threshold = 600;
#endif

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
const int NUM_MEASUREMENTS = 50;
float TEMPERATURE_COMPENSATION = 0.01;
float measurements[NUM_MEASUREMENTS];
float measurements_duration[NUM_MEASUREMENTS];
int trimPercent = 10;
long MAX_DISTANCE = 400;
#endif

#include <algorithm>

void eliminateOutliersIQR(float array[], int &n, float lowerThreshold,
                          float upperThreshold) {
  // sort the array
  std::sort(array, array + n);

  // calculate the quartiles
  float q1 = array[n / 4];
  float q3 = array[n * 3 / 4];
  float iqr = q3 - q1;

  // calculate the lower and upper bounds
  float lowerBound = q1 - lowerThreshold * iqr;
  float upperBound = q3 + upperThreshold * iqr;

  // remove values that fall outside of the bounds
  int j = 0;
  for (int i = 0; i < n; i++) {
    if (array[i] >= lowerBound && array[i] <= upperBound) {
      array[j++] = array[i];
    }
  }
  n = j;
}

double calculateMean(float array[], int n) {
  double sum = 0;
  double mean = 0;

  // calculate mean
  for (int i = 0; i < n; i++) {
    sum += array[i];
  }
  mean = sum / n;
  return mean;
}

double calculateVariance(float array[], int n) {
  double sum = 0;
  double mean = 0;
  double variance = 0;

  mean = calculateMean(array, n);

  // calculate variance
  for (int i = 0; i < n; i++) {
    variance += pow((array[i] - mean), 2);
  }
  variance = variance / n;
  return variance;
}

double calculateStandardDeviation(float array[], int n) {
  double variance = 0;
  variance = calculateVariance(array, n);

  // calculate standard deviation
  double standardDeviation = sqrt(variance);

  return standardDeviation;
}

uint16_t get_duration() {
  digitalWrite(TRIGPIN, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(TRIGPIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGPIN, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(ECHOPIN, HIGH);
  return duration;
}

hcsrStatus_t read_ultrasonic() {
  hcsrStatus_t hcsrStatus;
// Temperature compensation
#if (HAS_GY21)
  float temperature = ht2x.readTemperature();
  ESP_LOGI(TAG, "Measure temp: %f", temperature);
#else
  float temperature = 20;
#endif
  for (int i = 0; i < NUM_MEASUREMENTS; i++) {
    duration = get_duration();
    measurements_duration[i] = duration;
    // ESP_LOGI(TAG, "Measure measurement: %d", duration);
    // Calculate distance
    float measurement = (duration * 0.03314 / 2);
    measurement = ((331.4 + (0.606 * temperature)) * duration / 20000);

    // ESP_LOGI(TAG, "Measure measurement: %f", measurement);
    measurements[i] = measurement;
    delay(50);
  }
  // Sort measurements in ascending order
  std::sort(measurements, measurements + NUM_MEASUREMENTS);

  int n = NUM_MEASUREMENTS;
  // call eliminateOutliersIQR
  eliminateOutliersIQR(measurements, n, 1.5, 1.5);
  // ESP_LOGI(TAG, "Measure n: %d", n);
  // print mean of measurements
  double mean3 = calculateMean(measurements, n);
  distance = mean3;
  // ESP_LOGI(TAG, "Measure distance: %f", distance);

  // duration
  std::sort(measurements_duration, measurements_duration + NUM_MEASUREMENTS);
  int n_duration = NUM_MEASUREMENTS;
  // call eliminateOutliersIQR
  eliminateOutliersIQR(measurements_duration, n_duration, 1.5, 1.5);
  int mean3_duration = calculateMean(measurements_duration, n_duration);
  duration = mean3_duration;
  // ESP_LOGI(TAG, "Measure duration: %d", duration);

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
// boolean to check that reading is called only twice
bool run_once = false;

hcsrStatus_t read_ultrasonic_sensor() {
  hcsrStatus_t hcsrStatus;
  hcsrStatus_t hcsrStatus_1;
  hcsrStatus_t hcsrStatus_2;

  hcsrStatus_1 = read_ultrasonic();
  delay(100);
  hcsrStatus_2 = read_ultrasonic();
  // take the average of both measurements
  hcsrStatus.distance = (hcsrStatus_1.distance + hcsrStatus_2.distance) / 2;
  hcsrStatus.duration = (hcsrStatus_1.duration + hcsrStatus_2.duration) / 2;
  ESP_LOGI(TAG, "Measurements 1: %f", hcsrStatus_1.distance);
  ESP_LOGI(TAG, "Measurements 2: %f", hcsrStatus_2.distance);
  // check if both measurements are in 10 % range of each other
  if (abs(hcsrStatus_1.distance - hcsrStatus_2.distance) >
      (hcsrStatus_1.distance * 0.05)) {
    // check if the function has been called before
    // LOG that the measurements are not in range
    ESP_LOGI(TAG, "Measurements are not in range");
    if (run_once) {
      // if yes, then return the average of both measurements
      return hcsrStatus;
    }

    // if not, then set the run_once boolean to true
    run_once = true;
    // rerun the function
    return hcsrStatus = read_ultrasonic_sensor();
  }
  return hcsrStatus;
}

uint8_t *sensor_read(uint8_t sensor) {
  static uint8_t buf[SENSORBUFFER] = {0};
  uint8_t length = 3;
  hcsrStatus_t hcsrStatus;
  double temperature, humidity;
  int sensorValue;
  switch (sensor) {
  case 1:
    ESP_LOGI(TAG, "Inside Sensor 1");
    sensorValue = analogRead(SENSORPIN);
    ESP_LOGI(TAG, "Sensor Value: %d", sensorValue);
    if (sensorValue > threshold) {
      ESP_LOGI(TAG, "Sensor Value > threshold");
    }
    payload.addCount(sensorValue, 1);
    break;
  case 2:
    run_once = false;
    ESP_LOGI(TAG, "Inside Sensor 2");
    hcsrStatus = read_ultrasonic_sensor();
    ESP_LOGI(TAG, "Sensor 2: Distance: %f", hcsrStatus.distance);
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