#include <Arduino.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>             // Arduino SPI library

#define TFT_CS   15  // Chip select control pin
#define TFT_DC    2  // Data Command control pin
#define TFT_RST   4  // Reset pin (could connect to RST pin)

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
  // Clear previous text and noise indication area
  tft.fillRect(0, 0, 240, 160, ST77XX_BLACK);

  // Display air quality text
  tft.setTextSize(3);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(10, 10);
  tft.println("Air Quality:");

  // Determine text color based on air quality level
  if (value < 50) {
    tft.setTextColor(ST77XX_GREEN);
  } else if (value < 100) {
    tft.setTextColor(ST77XX_YELLOW);
  } else if (value < 200) {
    tft.setTextColor(ST77XX_ORANGE);
  } else {
    tft.setTextColor(ST77XX_RED);
  }
  
  // Print air quality value
  tft.setTextSize(4);
  tft.setCursor(10, 60);
  if (value < 100) {
    tft.setCursor(30, 60); // Adjusted position for two-digit values
  }
  tft.print(value);

  // Display AQI text
  tft.setTextSize(2);
  tft.setCursor(10, 130); // Adjusted AQI text position
  tft.println("AQI");

  // Display noise level indication bar
  displayNoiseIndication(value);
}

void displayNoiseIndication(int airQuality) {
  // Calculate noise level percentage based on air quality
  int noiseLevel = map(airQuality, 0, 500, 0, 100); // Simulate noise level percentage (0-100%)
  int barHeight = map(noiseLevel, 0, 100, 0, 160); // Map noise level percentage to pixel height

  // Draw noise level indication bar with gradient colors
  for (int y = 0; y < barHeight; ++y) {
    if (y < barHeight * 0.2) {
      tft.drawFastVLine(220, 160 - y, 50, ST77XX_GREEN); // Increased thickness to 5 pixels
    } else if (y < barHeight * 0.5) {
      tft.drawFastVLine(220, 160 - y, 50, ST77XX_YELLOW);
    } else if (y < barHeight * 0.8) {
      tft.drawFastVLine(220, 160 - y,50, ST77XX_ORANGE);
    } else {
      tft.drawFastVLine(220, 160 - y, 50, ST77XX_RED);
    }
  }

  // Display noise level text
  tft.setTextSize(2.6);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(10, 210);
  tft.print("Noise Level: ");
  tft.print(noiseLevel);
  tft.println("%");
}
