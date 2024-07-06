#include <Arduino.h>
#include <ArduinoJson.h>
#include <esp_now.h> 
#include <WiFi.h> 
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#define green1 15
#define green2 13
#define green3 4
#define green4 16
#define yellow1 18
#define yellow2 19
#define yellow3 21
#define yellow4 22
#define d2 2
int a= 0;
int R = 1;
int Traffic_delay = 1000;
int espnow_connect = 0;
int exp_execute = 0;

String recv_jsondata;  
StaticJsonDocument<256> doc;

void Traffic_light(void *pvParameters);
TaskHandle_t Traffic_light_handler;

void ESP_NOW_Task(void *pvParameters);
TaskHandle_t ESP_NOW_Task_Handler;

void Traffic_time_control(void *pvParameters);
TaskHandle_t Traffic_time_control_handler;

void Traffic_time_control2(void *pvParameters);
TaskHandle_t Traffic_time_control_handler2;

EventGroupHandle_t Traffic_lights_time_signal;
EventGroupHandle_t Traffic_lights_time_signal2;

const int BIT_0 = (1 << 0);
const int BIT_1 = (1 << 1);

void setup() {
  Serial.begin(115200);
  pinMode(green1, OUTPUT);
  pinMode(green2, OUTPUT);
  pinMode(green3, OUTPUT);
  pinMode(green4, OUTPUT);
  pinMode(yellow1, OUTPUT);
  pinMode(yellow2, OUTPUT);
  pinMode(yellow3, OUTPUT);
  pinMode(yellow4, OUTPUT);
  pinMode(d2, OUTPUT);

  Traffic_lights_time_signal = xEventGroupCreate();
  Traffic_lights_time_signal2 = xEventGroupCreate();
  xTaskCreatePinnedToCore(ESP_NOW_Task, "ESP NOW", 2048, NULL, 1, &ESP_NOW_Task_Handler, 0);
  xTaskCreate(Traffic_light, "Traffic Light", 2048, NULL, 1, &Traffic_light_handler);
  xTaskCreate(Traffic_time_control, "Traffic Time Control", 2048, NULL, 1, &Traffic_time_control_handler);
  xTaskCreate(Traffic_time_control2, "Traffic Time Control", 2048, NULL, 1, &Traffic_time_control_handler2);
  xEventGroupSetBits(Traffic_lights_time_signal, BIT_1);
}

void Traffic_time_control(void *pvParameters) {
  for (;;) {
    EventBits_t xBits = xEventGroupWaitBits(
      Traffic_lights_time_signal,
      BIT_0,
      pdTRUE,
      pdFALSE,
      portMAX_DELAY
    );
    Serial.println("Updating New Traffic Delay...");
    xEventGroupSetBits(Traffic_lights_time_signal, BIT_1);
  }
}

void Traffic_time_control2(void *pvParameters) {
  for (;;) {
    EventBits_t xBits = xEventGroupWaitBits(
      Traffic_lights_time_signal2,
      BIT_0,
      pdTRUE,
      pdFALSE,
      portMAX_DELAY
    );
    // Placeholder for future functionality
    Serial.println("Fetching the No. of cars");
  }
}

void ESP_NOW_Task(void *pvParameters) {
  for (;;) {
    if(exp_execute > 0) {
      digitalWrite(d2, HIGH);
      delay(1000);
      digitalWrite(d2, LOW);
      delay(1000);
    }
    if(a == 0) {
      a = 1;
      WiFi.mode(WIFI_STA);
      if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        ESP.restart();
        return;
      }
      esp_now_register_recv_cb(OnDataRecv);
    }
  }
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  char* buff = (char*) incomingData;
  recv_jsondata = String(buff);
  DeserializationError error = deserializeJson(doc, recv_jsondata);
  if (!error) {
    exp_execute = doc["EXP"];
  }
  recv_jsondata = ""; 
}

void Traffic_light(void *pvParameters) {
  for (;;) {
    EventBits_t xBits = xEventGroupWaitBits(
      Traffic_lights_time_signal,
      BIT_1,
      pdTRUE,
      pdFALSE,
      portMAX_DELAY
    );

    switch (R) {
      case 1:
        digitalWrite(green1, HIGH);
        xEventGroupSetBits(Traffic_lights_time_signal2, BIT_0);
        vTaskDelay(pdMS_TO_TICKS(Traffic_delay / 2));
        digitalWrite(green1, LOW);
        digitalWrite(yellow1, HIGH);
        vTaskDelay(pdMS_TO_TICKS(Traffic_delay / 2));
        xEventGroupSetBits(Traffic_lights_time_signal, BIT_0);
        digitalWrite(yellow1, LOW);
        R = 2;
        break;
      case 2:
        digitalWrite(green2, HIGH);
        xEventGroupSetBits(Traffic_lights_time_signal2, BIT_0);
        vTaskDelay(pdMS_TO_TICKS(Traffic_delay / 2));
        digitalWrite(green2, LOW);
        digitalWrite(yellow2, HIGH);
        vTaskDelay(pdMS_TO_TICKS(Traffic_delay / 2));
        xEventGroupSetBits(Traffic_lights_time_signal, BIT_0);
        digitalWrite(yellow2, LOW);
        R = 3;
        break;
      case 3:
        digitalWrite(green3, HIGH);
        xEventGroupSetBits(Traffic_lights_time_signal2, BIT_0);
        vTaskDelay(pdMS_TO_TICKS(Traffic_delay / 2));
        digitalWrite(green3, LOW);
        digitalWrite(yellow3, HIGH);
        vTaskDelay(pdMS_TO_TICKS(Traffic_delay / 2));
        xEventGroupSetBits(Traffic_lights_time_signal, BIT_0);
        digitalWrite(yellow3, LOW);
        R = 4;
        break;
      case 4:
        digitalWrite(green4, HIGH);
        xEventGroupSetBits(Traffic_lights_time_signal2, BIT_0);
        vTaskDelay(pdMS_TO_TICKS(Traffic_delay / 2));
        digitalWrite(green4, LOW);
        digitalWrite(yellow4, HIGH);
        vTaskDelay(pdMS_TO_TICKS(Traffic_delay / 2));
        xEventGroupSetBits(Traffic_lights_time_signal, BIT_0);
        digitalWrite(yellow4, LOW);
        R = 1;
        break;
      default:
        break;
    }
  }
}

void loop() {}
