#include <ArduinoJson.h>

#define LED_pin 2

void setup() {
    Serial.begin(115200); // Initialize serial communication for debugging
    Serial2.begin(9600);  // Initialize UART for communication with another ESP32
    pinMode(LED_pin, OUTPUT);
    digitalWrite(LED_pin, LOW);
}

void blinkLED(int times, int interval) {
    for (int i = 0; i < times; i++) {
        digitalWrite(LED_pin, HIGH);
        delay(interval);
        digitalWrite(LED_pin, LOW);
        delay(interval);
    }
}

void loop() {
    if (Serial.available() > 0) {
        String msg = Serial.readStringUntil('\n'); // Read serial data until newline
        msg.trim(); // Remove any trailing newline or space characters
        Serial.println("Received: " + msg); // Debugging

        // Parse JSON
        DynamicJsonDocument doc(1024); // Adjust size according to your JSON complexity
        DeserializationError error = deserializeJson(doc, msg);

        if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            return;
        }

        // Retrieve total count from JSON
        int total_count = doc["Total"];
        Serial.println("Total count: " + String(total_count)); // Debugging

        // Control LED based on total_count
        if (total_count > 10) {
            blinkLED(5, 500); // Blink LED 5 times with 500ms interval
        } else if (total_count > 0) {
            digitalWrite(LED_pin, HIGH);
        } else {
            digitalWrite(LED_pin, LOW);
        }

        // Send total_count to another ESP32 using UART
        Serial2.println(total_count);
    }
}
