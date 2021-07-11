#include <M5Stack.h>
#include <Wire.h>
#include "Adafruit_Sensor.h"
#include "Adafruit_BMP280.h"
#include "Adafruit_SGP30.h"

Adafruit_SGP30 sgp30;
Adafruit_BMP280 bmp280;

unsigned int sgp30_tvoc = 0;
unsigned int sgp30_eco2 = 0;
float bmp280_pressure = 0.0;
float bmp280_temperature = 0.0;
float sht30_temperature = 0.0;
float sht30_humidity = 0.0;

const byte font_type = 4;

const unsigned int offset = 5;

const unsigned int coordinate_x_value = 140;
const unsigned int coordinate_x_unit = 240;

void setup() {
  M5.begin(true, false, true, true);
  init();
}

void init() {
  init_lcd();
  init_sgp30();
  init_bmp280();
}

void init_lcd() {
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Lcd.setTextDatum(TC_DATUM);
}

void init_sgp30() {
  if (!sgp30.begin()) {
    while (1) {
      // TODO
    }
  }
}

void wait_sgp30() {
  M5.Lcd.setTextDatum(MC_DATUM);
  M5.Lcd.drawString("Initialization...", 160, 120, font_type);

  unsigned long last_millis = 0;
  while (millis() - last_millis < 15000L) {
    delay(1); // avoid reset by esp32 watchdog timer
  }
  
  M5.Lcd.setTextDatum(TC_DATUM);
}

void init_bmp280() {
  while (!bmp280.begin(0x76)) {
    delay(1);
  }
}

void reset_display() {
  M5.Lcd.fillScreen(TFT_BLACK);

  M5.Lcd.drawString("TVOC:", 50,   0 + offset, font_type);
  M5.Lcd.drawString("eCO2:", 50,  40 + offset, font_type);
  M5.Lcd.drawString("PRES:", 50,  80 + offset, font_type);
  M5.Lcd.drawString("TMP1:", 50, 120 + offset, font_type);
  M5.Lcd.drawString("TMP2:", 50, 160 + offset, font_type);
  M5.Lcd.drawString("HUMD:", 50, 200 + offset, font_type);
}

bool initialized = false;

void loop() {
  if (!initialized) {
    wait_sgp30();
    reset_display();
    initialized = true;
  }

  // SGP30
  if (!sgp30.IAQmeasure()) {
    return;
  }
  sgp30_tvoc = sgp30.TVOC;
  sgp30_eco2 = sgp30.eCO2;

  // BMP280
  bmp280_pressure = bmp280.readPressure();
  bmp280_temperature = bmp280.readTemperature();

  // SHT30
  read_sht30();

  // Display
  draw_sampling_results();

  delay(3000);
}


void draw_sampling_results() {
  M5.Lcd.fillRect(100, 0, 220, 240, TFT_BLACK);

  // SGP30 TVOC
  M5.Lcd.drawNumber(sgp30_tvoc, coordinate_x_value, 0 + offset, font_type);
  M5.Lcd.drawString("ppb",      coordinate_x_unit,  0 + offset, font_type);

  // SGP30 CO2
  M5.Lcd.drawNumber(sgp30_eco2, coordinate_x_value, 40 + offset, font_type);
  M5.Lcd.drawString("ppm",      coordinate_x_unit,  40 + offset, font_type);

  // BMP280 Pressure
  char bmp280_pressure_buf[20];
  sprintf(bmp280_pressure_buf, "%0.2f", bmp280_pressure / 100.0);
  M5.Lcd.drawString(bmp280_pressure_buf, coordinate_x_value, 80 + offset, font_type);
  M5.Lcd.drawString("hPa",               coordinate_x_unit,  80 + offset, font_type);

  // BMP280 Temperature
  char bmp280_temperature_buf[20];
  sprintf(bmp280_temperature_buf, "%0.2f", bmp280_temperature);
  M5.Lcd.drawString(bmp280_temperature_buf, coordinate_x_value, 120 + offset, font_type);
  M5.Lcd.drawString("degrees",              coordinate_x_unit,  120 + offset, font_type);

  // SHT30 Temperature
  char sht30_temperature_buf[20];
  sprintf(sht30_temperature_buf, "%0.2f", sht30_temperature);
  M5.Lcd.drawString(sht30_temperature_buf, coordinate_x_value, 160 + offset, font_type);
  M5.Lcd.drawString("degrees",             coordinate_x_unit,  160 + offset, font_type);

  // SHT30 Humidity
  char sht30_humidity_buf[20];
  sprintf(sht30_humidity_buf, "%0.2f", sht30_humidity);
  M5.Lcd.drawString(sht30_humidity_buf, coordinate_x_value, 200 + offset, font_type);
  M5.Lcd.drawString("%",                coordinate_x_unit,  200 + offset, font_type);
}

void init_sht30() {
  // nothing to do
}

void read_sht30() {
  Wire.beginTransmission(0x44);
  Wire.write(0x2C);
  Wire.write(0x06);
  Wire.endTransmission();

  delay(500);

  Wire.requestFrom(0x44, 6);

  unsigned int data[6];
  for (int i = 0; i < 6; i++) {
    data[i] = Wire.read();
  }

  sht30_temperature = ((((data[0] * 256.0) + data[1]) * 175) / 65535.0) - 45;
  sht30_humidity = ((((data[3] * 256.0) + data[4]) * 100) / 65535.0);
}
