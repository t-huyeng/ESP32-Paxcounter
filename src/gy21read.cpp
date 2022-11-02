// routines for fetching data from the GY21-sensor

#if (HAS_GY21)

// Local logging tag
static const char TAG[] = __FILE__;

#include "gy21read.h"
#include <Wire.h>

double temperature, humidity;
byte error;

// init
bool gy21_init(const int addr) {
  // TODO check PINS
  Wire.begin(21, 22);
  delay(100);
  Wire.beginTransmission(addr);
  error = Wire.endTransmission();
  if (error != 0) {
    ESP_LOGE(TAG, "GY21 not found");
    return false;
  }
  return true;
}

double gy21_read_temperature(const int addr) {
  ESP_LOGD(TAG, "GY21: Reading temperature");
  double gy21_temp;
  int low_byte, high_byte,
      raw_data; /**Send command of initiating temperature measurement**/
  Wire.beginTransmission(addr);
  Wire.write(0xE3);
  Wire.endTransmission(); /**Read data of temperature**/
  Wire.requestFrom(addr, 2);
  if (Wire.available() <= 2) {
    high_byte = Wire.read();
    low_byte = Wire.read();
    high_byte = high_byte << 8;
    raw_data = high_byte + low_byte;
  }
  temperature = (175.72 * raw_data) / 65536;
  temperature = temperature - 46.85;
  return temperature;
}

double gy21_read_humidity(const int addr) {
  ESP_LOGD(TAG, "GY21: Reading humidity");
  double humidity, raw_data_1, raw_data_2;
  int low_byte, high_byte,
      container; /**Send command of initiating relative humidity measurement**/
  Wire.beginTransmission(addr);
  Wire.write(0xE5);
  Wire.endTransmission(); /**Read data of relative humidity**/
  Wire.requestFrom(addr, 2);
  if (Wire.available() <= 2) {
    high_byte = Wire.read();
    container = high_byte / 100;
    high_byte = high_byte % 100;
    low_byte = Wire.read();
    raw_data_1 = container * 25600;
    raw_data_2 = high_byte * 256 + low_byte;
  }
  raw_data_1 = (125 * raw_data_1) / 65536;
  raw_data_2 = (125 * raw_data_2) / 65536;
  humidity = raw_data_1 + raw_data_2;
  humidity = humidity - 6;
  return humidity;
}

#endif // HAS_GY21