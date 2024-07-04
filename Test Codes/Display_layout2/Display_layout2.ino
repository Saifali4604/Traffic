#include <Arduino.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>             // Arduino SPI library

#define TFT_CS        2
#define TFT_RST       5
#define TFT_DC        4

// Initialize Adafruit ST7789 TFT library
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

void setup() {
  Serial.begin(115200);
    
  tft.init(240, 240, SPI_MODE2);    // Init ST7789 display 240x240 pixels
  tft.setRotation(3);
  tft.fillScreen(ST77XX_BLACK);

  // Display initial air quality value and noise level indication
  displayAirQuality(random(0, 500));
}

void loop() {
  // Simulate changing air quality every few seconds
  int airQuality = random(0, 500);
  displayAirQuality(airQuality);
  delay(3000); // Adjust delay as needed
}

void displayAirQuality(int value) {
  int color;
  // Determine text color based on air quality level
  if (value < 50) {
    color = ST77XX_GREEN;
  } else if (value < 100) {
    color = ST77XX_YELLOW;
  } else if (value < 200) {
    color = ST77XX_ORANGE;
  } else {
    color = ST77XX_RED;
  }

  // Clear previous AQI display
  tft.fillRect(10, 60, 220, 60, ST77XX_BLACK);

  // Print air quality value
  tft.setTextColor(color, ST77XX_BLACK);
  tft.setTextSize(4);
  tft.setCursor(10 + 30, 60);
  if (value < 100) {
    tft.print(" ");
    tft.setCursor(30 + 30, 60); // Adjusted position for two-digit values
  }
  tft.print(value);

  // Display AQI text
  tft.setTextSize(4);
  tft.setCursor(100 + 30, 60); // Adjusted AQI text position
  tft.println("AQI");
  displayNoiseIndication(value);
}

void displayNoiseIndication(int noise) {
  // Calculate noise level percentage based on air quality
  int noiseLevel = map(noise, 0, 500, 0, 100); // Simulate noise level percentage (0-100%)
  int barWidth = map(noiseLevel, 0, 100, 0, 240); // Map noise level percentage to pixel width

  // Clear previous noise indication display
  tft.fillRect(0, 170, 240, 60, ST77XX_BLACK);

  // Draw noise level bar
  for (int i = 0; i < barWidth; ++i) {
    if (i < barWidth * 0.2) {
      tft.drawFastVLine(i, 200, 30, ST77XX_GREEN);
    } else if (i < barWidth * 0.5) {
      tft.drawFastVLine(i, 200, 30, ST77XX_YELLOW);
    } else if (i < barWidth * 0.8) {
      tft.drawFastVLine(i, 200, 30, ST77XX_ORANGE);
    } else {
      tft.drawFastVLine(i, 200, 30, ST77XX_RED);
    }
  }

  // Display noise level text
  tft.setTextSize(2.5);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setCursor(10, 170);
  tft.print("Noise Level: ");
  tft.print(noiseLevel);
  tft.println("%");
}

