#include <Arduino.h>
#include <driver/i2s.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>             // Arduino SPI library

#define TFT_CS  2
#define TFT_RST 5
#define TFT_DC 4
#define I2S_WS 15    // I2S word select pin
#define I2S_SCK 14   // I2S clock pin
#define I2S_SD 32    // I2S data pin

#define green1 27
#define green2 26
#define green3 34
#define green4 16
#define yellow1 33
#define yellow2 19
#define yellow3 21
#define yellow4 22

// I2S configuration
const i2s_config_t i2s_config = {
  .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX), // Receive mode
  .sample_rate = 44100,                             // Sample rate
  .bits_per_sample = i2s_bits_per_sample_t(16),     // Bits per sample
  .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,      // Only left channel
  .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S),
  .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,         // Interrupt level 1
  .dma_buf_count = 8,                               // Number of DMA buffers
  .dma_buf_len = 1024,                              // DMA buffer length
  .use_apll = false,
  .tx_desc_auto_clear = false,
  .fixed_mclk = 0
};

// I2S pin configuration
const i2s_pin_config_t pin_config = {
  .bck_io_num = I2S_SCK,
  .ws_io_num = I2S_WS,
  .data_out_num = I2S_PIN_NO_CHANGE,
  .data_in_num = I2S_SD
};

// Initialize Adafruit ST7789 TFT library
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// Mutex for TFT display access
SemaphoreHandle_t tftMutex ;

int16_t i2sData[1024];
int32_t sum = 0;
int R = 1;
int Traffic_delay = 1000;
int sensitivity = 70; // Sensitivity variable (0 to 100)

void Traffic_light(void *pvParameters); 
TaskHandle_t Traffic_light_handler; 

void Traffic_time_control(void *pvParameters); 
TaskHandle_t Traffic_time_control_handler; 

void Traffic_display_airquality(void *pvParameters); 
TaskHandle_t Traffic_display_airquality_handler; 

void Traffic_display_noise(void *pvParameters); 
TaskHandle_t Traffic_display_noise_handler;

EventGroupHandle_t Traffic_lights_time_signal; 
 
void setup(){ 
  Serial.begin(115200); 
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  i2s_set_clk(I2S_NUM_0, 44100, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
  pinMode(green1,OUTPUT);
  pinMode(green2,OUTPUT);
  pinMode(green3,OUTPUT);
  pinMode(green4,OUTPUT);
  pinMode(yellow1,OUTPUT);
  pinMode(yellow2,OUTPUT);
  pinMode(yellow3,OUTPUT);
  pinMode(yellow4,OUTPUT);

  tft.init(240, 240, SPI_MODE2);    // Init ST7789 display 240x240 pixels
  tft.setRotation(3);
  tft.fillScreen(ST77XX_BLACK);
  tft.fillRect(0, 0, 240, 240, ST77XX_BLACK);
  tft.setTextSize(3);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(10, 10);
  tft.println("Air Quality:");

  // Create mutex for TFT display access
  tftMutex = xSemaphoreCreateMutex();

  Traffic_lights_time_signal = xEventGroupCreate(); 
  xTaskCreate(Traffic_light, "Blink Led", 2048, NULL, 1 , &Traffic_light_handler);
  xTaskCreate(Traffic_display_airquality, "display", 2048, NULL, 1 , &Traffic_display_airquality_handler);
  xTaskCreate(Traffic_display_noise, "display", 2048, NULL, 1 , &Traffic_display_noise_handler);
  xTaskCreate(Traffic_time_control, "Blink Led", 2048, NULL, 1 , &Traffic_time_control_handler);
  xEventGroupSetBits(Traffic_lights_time_signal, 1); 
  xEventGroupSetBits(Traffic_lights_time_signal, 0);
} 

void displayAirQuality(int value) {
  // Acquire mutex to access TFT display
  if (xSemaphoreTake(tftMutex, portMAX_DELAY) == pdTRUE) {
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
    // Release mutex after TFT display access
    xSemaphoreGive(tftMutex);
  }
}

void displayNoiseIndication(int noiseLevel) {
  // Acquire mutex to access TFT display
  if (xSemaphoreTake(tftMutex, portMAX_DELAY) == pdTRUE) {
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
    // Release mutex after TFT display access
    xSemaphoreGive(tftMutex);
  }
}


void Traffic_display_airquality(void *pvParameters){  
  for(;;){
    int airQuality = random(0, 500);
    displayAirQuality(airQuality);
    vTaskDelay(pdMS_TO_TICKS(1100));
  }
}

void Traffic_display_noise(void *pvParameters){  
  for(;;){
    size_t bytesRead;
    i2s_read(I2S_NUM_0, i2sData, sizeof(i2sData), &bytesRead, portMAX_DELAY);
    // Calculate RMS value of the audio signal
    sum = 0;
    for (int i = 0; i < bytesRead / 2; ++i) {
      sum += abs(i2sData[i]); // Use absolute value for higher sensitivity to small sounds
    }
    float rms = sum / (bytesRead / 2);
    // Convert RMS to a percentage for noise level (adjusted for sensitivity)
    int maxRMS = map(sensitivity, 0, 100, 1000, 100); // Adjust maxRMS based on sensitivity
    int noiseLevel = map(rms, 0, maxRMS, 0, 100); // Adjust scale as needed
    noiseLevel = constrain(noiseLevel, 0, 100); // Constrain the noise level to 100%
    displayNoiseIndication(noiseLevel); // Display noise level as a bar on the TFT display
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void Traffic_time_control(void *pvParameters){  
  for(;;){
    EventBits_t xBits = xEventGroupWaitBits( 
                 Traffic_lights_time_signal,    // The event group being tested. 
                 1 | 2 | 3 | 4,  // The bits within the event group to wait for. 
                 pdTRUE,         // BIT_0 and BIT_4 should be cleared before returning. 
                 pdTRUE,        // Don't wait for both bits, either bit will do. 
                 portMAX_DELAY);  //ms 
    int r = xBits;
    // fetch the No. of cars 
    // calculate the time for r road by comparing with others roads
    // max time should be standard time.
    EventBits_t xBit = xEventGroupWaitBits( 
                 Traffic_lights_time_signal,    // The event group being tested. 
                 0,  // The bits within the event group to wait for. 
                 pdTRUE,         // BIT_0 and BIT_4 should be cleared before returning. 
                 pdFALSE,        // Don't wait for both bits, either bit will do. 
                 portMAX_DELAY);  //ms 
    // load the time Traffic_delay
    xEventGroupSetBits(Traffic_lights_time_signal, 5);
  }
}

void Traffic_light(void *pvParameters) {
  for (;;) {
    EventBits_t xBits = xEventGroupWaitBits(
      Traffic_lights_time_signal,    // The event group being tested.
      5,  // The bits within the event group to wait for.
      pdTRUE,         // Clear bits before returning.
      pdFALSE,        // Wait for any bit.
      portMAX_DELAY); // Wait indefinitely

    switch (R) {
      case 1:
        digitalWrite(green1, HIGH);
        xEventGroupSetBits(Traffic_lights_time_signal, 2);
        vTaskDelay(pdMS_TO_TICKS(Traffic_delay/2));
        digitalWrite(green1, LOW);
        digitalWrite(yellow1, HIGH);
        vTaskDelay(pdMS_TO_TICKS(Traffic_delay/2));
        xEventGroupSetBits(Traffic_lights_time_signal, 0);
        digitalWrite(yellow1, LOW);
        R = 2;
        break;
      case 2:
        digitalWrite(green2, HIGH);
        xEventGroupSetBits(Traffic_lights_time_signal, 3);
        vTaskDelay(pdMS_TO_TICKS(Traffic_delay/2));
        digitalWrite(green2, LOW);
        digitalWrite(yellow2, HIGH);
        vTaskDelay(pdMS_TO_TICKS(Traffic_delay/2));
        xEventGroupSetBits(Traffic_lights_time_signal, 0);
        digitalWrite(yellow2, LOW);
        R = 3;
        break;
      case 3:
        digitalWrite(green3, HIGH);
        xEventGroupSetBits(Traffic_lights_time_signal, 4);
        vTaskDelay(pdMS_TO_TICKS(Traffic_delay/2));
        digitalWrite(green3, LOW);
        digitalWrite(yellow3, HIGH);
        vTaskDelay(pdMS_TO_TICKS(Traffic_delay/2));
        xEventGroupSetBits(Traffic_lights_time_signal, 0);
        digitalWrite(yellow3, LOW);
        R = 4;
        break;
      case 4:
        digitalWrite(green4, HIGH);
        xEventGroupSetBits(Traffic_lights_time_signal, 1);
        vTaskDelay(pdMS_TO_TICKS(Traffic_delay/2));
        digitalWrite(green4, LOW);
        digitalWrite(yellow4, HIGH);
        vTaskDelay(pdMS_TO_TICKS(Traffic_delay/2));
        xEventGroupSetBits(Traffic_lights_time_signal, 0);
        digitalWrite(yellow4, LOW);
        R = 1;
        break;
      default:
        break;
    }
  }
}

void loop(){} 

