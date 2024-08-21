#include <VirtualWire.h>

#define RECEIVE_PIN 11

String receivedData = "";

void setup() {
  Serial.begin(9600);
  vw_set_rx_pin(RECEIVE_PIN);
  vw_setup(2000); // Bits per second
  vw_rx_start(); // Start the receiver PLL running
}

void loop() {
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;

  if (vw_get_message(buf, &buflen)) {
    // Message with a good checksum received
    String chunk = "";
    for (int i = 0; i < buflen; i++) {
      chunk += (char)buf[i];
    }

    // Accumulate received data
    receivedData += chunk;

    // Assuming a complete message is received when the buffer is full or contains end characters
    // Print the full received message when you detect a full message
    // You can add a specific end character to mark the end of a message if needed

    Serial.println(receivedData);
    receivedData = ""; // Clear the buffer for the next message
  }
}