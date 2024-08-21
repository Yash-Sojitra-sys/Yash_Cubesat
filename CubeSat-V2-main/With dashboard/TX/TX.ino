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

  // Convert float values to integers
  int humidityInt = round(humidity);
  int temperatureInt = round(temperature);
  int pressureInt = round(pressure / 100); // Convert to hPa
  int altitudeInt = round(altitude);
  
  // Prepare data to send in chunks
  String data = String(temperatureInt) + "," + String(pressureInt) + "," + String(altitudeInt) + "," +
                String(ax) + "," + String(ay) + "," + String(az/100) + "," + String(humidityInt);

  // Split data into chunks if necessary
  int chunkSize = 27; // Adjust this size as needed
  for (int i = 0; i < data.length(); i += chunkSize) {
    String chunk = data.substring(i, i + chunkSize);
    sendString(chunk);
  }

  Serial.print("Sending: ");
  Serial.println(data);

  delay(2000); // Send data every 2 seconds
}
