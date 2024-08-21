#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085.h>
#include <DHT.h>
#include <VirtualWire.h>
#include <MPU6050.h>

#define DHTPIN 2       // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11  // DHT 11
#define TRANSMIT_PIN 12

DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085 bmp;
MPU6050 mpu;

void setup() {
  Serial.begin(9600);
  dht.begin();
  
  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    while (1) {}
  }

  Wire.begin();
  mpu.initialize();
  
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed!");
    while (1) {}
  }

  vw_set_tx_pin(TRANSMIT_PIN);
  vw_setup(2000); // Bits per second
}

void sendString(String data) {
  char charBuf[data.length() + 1];
  data.toCharArray(charBuf, data.length() + 1);
  vw_send((uint8_t *)charBuf, strlen(charBuf));
  vw_wait_tx(); // Wait until the whole message is gone
  delay(100); // Small delay to ensure the receiver has time to process the message
}

void loop() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  float pressure = bmp.readPressure();
  float altitude = bmp.readAltitude();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // Prepare data to send in chunks
  String data1 = "T:" + String(temperature) + ",H:" + String(humidity);
  String data2 = "P:" + String(pressure/100) + ",A:" + String(altitude);
  String data3 = "AX:" + String(ax) + ",AY:" + String(ay) + ",AZ:" + String(az);
  String data4 = "GX:" + String(gx) + ",GY:" + String(gy) + ",GZ:" + String(gz);

  // Send data chunks
  sendString(data1);
  sendString(data2);
  sendString(data3);
  sendString(data4);

  Serial.println("Sending:");
  Serial.println(data1);
  Serial.println(data2);
  Serial.println(data3);
  Serial.println(data4);

  delay(2000); // Send data every 2 seconds
}