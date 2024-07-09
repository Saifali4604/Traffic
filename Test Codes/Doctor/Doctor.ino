#include <Arduino.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <esp_now.h> 
#include <WiFi.h> 
#include <MAX30100_PulseOximeter.h>
#define REPORTING_PERIOD_MS  500
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);

// Define button pins
const int buttonHigh = 23;
const int buttonMedium = 18;
const int buttonLow = 5;
const int buttonYes = 15;
const int buttonNo = 4;

//PulseOximeter pox;
int b=0;
int P=0;
int Path =0;
int periority = 0;
int path_to_choose = 0;
int heart,spo2;
uint32_t tsLastReport = 0;
int espnow_connect = 0;
int exp_execute = 0;

String send_jsondata;
String recv_jsondata;  
StaticJsonDocument<256> doc;

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; //Master ESP (motor module) MAC address

void Heart_rate(void *pvParameters); 
TaskHandle_t Heart_rate_handler;  
void Questions_task(void *pvParameters);
TaskHandle_t Questions_task_handler;

void askHighSeverityQuestions() {
  Serial.println("1: Life-threatening?");
  lcd.clear();         
  lcd.setCursor(0,0);   //Set cursor to character 2 on line 0
  lcd.print("1: Life-threatening?");
  lcd.setCursor(0,1);   //Set cursor to character 2 on line 0
  lcd.print("Yes   No");
  if (getYesNoResponse()) {
    Serial.println("Yes");
    periority = 1; // Update priority based on the question
  } else {
    Serial.println("No");
  }
  delay(1000);

  Serial.println("2: Immediate intervention needed?");
  lcd.clear();         
  lcd.setCursor(0,0);   //Set cursor to character 2 on line 0
  lcd.print("2: Immediate intervention needed?");
  lcd.setCursor(0,1);   //Set cursor to character 2 on line 0
  lcd.print("Yes   No");
  if (getYesNoResponse()) {
    Serial.println("Yes");
    periority = 1; // Update priority based on the question
  } else {
    Serial.println("No");
  }
  delay(1000);

  Serial.println("3: Unconscious?");
  lcd.clear();         
  lcd.setCursor(0,0);   //Set cursor to character 2 on line 0
  lcd.print("3: Unconscious?");
  lcd.setCursor(0,1);   //Set cursor to character 2 on line 0
  lcd.print("Yes   No");
  if (getYesNoResponse()) {
    Serial.println("Yes");
    periority = 1; // Update priority based on the question
  } else {
    Serial.println("No");
  }
  delay(1000);
}

void askMediumSeverityQuestions() {
  Serial.println("1: Stable but prompt attention needed?");
  lcd.clear();         
  lcd.setCursor(0,0);   //Set cursor to character 2 on line 0
  lcd.print("1: Stable but prompt attention needed?");
  lcd.setCursor(0,1);   //Set cursor to character 2 on line 0
  lcd.print("Yes   No");
  if (getYesNoResponse()) {
    Serial.println("Yes");
    periority = 2; // Update priority based on the question
  } else {
    Serial.println("No");
  }
  delay(1000);

  Serial.println("2: Severe injuries?");
  lcd.clear();         
  lcd.setCursor(0,0);   //Set cursor to character 2 on line 0
  lcd.print("2: Severe injuries?");
  lcd.setCursor(0,1);   //Set cursor to character 2 on line 0
  lcd.print("Yes   No");
  if (getYesNoResponse()) {
    Serial.println("Yes");
    periority = 2; // Update priority based on the question
  } else {
    Serial.println("No");
  }
  delay(1000);

  Serial.println("3: Significant pain?");
  lcd.clear();         
  lcd.setCursor(0,0);   //Set cursor to character 2 on line 0
  lcd.print("3: Significant pain?");
  lcd.setCursor(0,1);   //Set cursor to character 2 on line 0
  lcd.print("Yes   No");
  if (getYesNoResponse()) {
    Serial.println("Yes");
    periority = 2; // Update priority based on the question
  } else {
    Serial.println("No");
  }
  delay(1000);
}

void askLowSeverityQuestions() {
  Serial.println("1: Minor injuries?");
  lcd.clear();         
  lcd.setCursor(0,0);   //Set cursor to character 2 on line 0
  lcd.print("1: Minor injuries?");
  lcd.setCursor(0,1);   //Set cursor to character 2 on line 0
  lcd.print("Yes   No");
  if (getYesNoResponse()) {
    Serial.println("Yes");
    periority = 3; // Update priority based on the question
  } else {
    Serial.println("No");
  }
  delay(1000);

  Serial.println("2: Stable with no danger?");
  lcd.clear();         
  lcd.setCursor(0,0);   //Set cursor to character 2 on line 0
  lcd.print("2: Stable with no danger?");
  lcd.setCursor(0,1);   //Set cursor to character 2 on line 0
  lcd.print("Yes   No");
  if (getYesNoResponse()) {
    Serial.println("Yes");
    periority = 3; // Update priority based on the question
  } else {
    Serial.println("No");
  }
  delay(1000);

  Serial.println("3: Can wait?");
  lcd.clear();         
  lcd.setCursor(0,0);   //Set cursor to character 2 on line 0
  lcd.print("3: Can wait?");
  lcd.setCursor(0,1);   //Set cursor to character 2 on line 0
  lcd.print("Yes   No");
  if (getYesNoResponse()) {
    Serial.println("Yes");
    periority = 3; // Update priority based on the question
  } else {
    Serial.println("No");
  }
  delay(1000);
}

bool getYesNoResponse() {
  while (true) {
    if (digitalRead(buttonYes) == LOW) {
      delay(500); // Debounce delay
      return true;
    }
    if (digitalRead(buttonNo) == LOW) {
      delay(500); // Debounce delay
      return false;
    }
  }
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  if (status == ESP_NOW_SEND_SUCCESS ){
    Serial.println("Delivery success");
    espnow_connect=1;
  }
  else{
    Serial.println("Delivery fail");
    espnow_connect=0;
  }
}

void esp_now_send(){
  doc["P"] = periority;
  doc["Path"] = path_to_choose;
  serializeJson(doc, send_jsondata); // converting it to JSON
  // Sending it to Master ESP (motor module)
  esp_now_send(broadcastAddress, (uint8_t *) send_jsondata.c_str(), send_jsondata.length());
  Serial.println(send_jsondata); // printing sent data
  lcd.clear();         
  lcd.backlight();      // Make sure backlight is on
  
  // Print a message on both lines of the LCD.
  lcd.setCursor(0,0);   //Set cursor to character 2 on line 0
  lcd.print(send_jsondata);
  delay(2000);
  send_jsondata = "";
  lcd.clear();         
  lcd.backlight();      // Make sure backlight is on
  
  // Print a message on both lines of the LCD.
  lcd.setCursor(0,0);   //Set cursor to character 2 on line 0
  lcd.print("High  Mid  Low");
}

void setup(){ 
  Serial.begin(115200); 
  pinMode(buttonHigh, INPUT_PULLUP);
  pinMode(buttonMedium, INPUT_PULLUP);
  pinMode(buttonLow, INPUT_PULLUP);
  pinMode(buttonYes, INPUT_PULLUP);
  pinMode(buttonNo, INPUT_PULLUP);
  lcd.init();
  lcd.clear();         
  lcd.backlight();      // Make sure backlight is on
  
  // Print a message on both lines of the LCD.
  lcd.setCursor(0,0);   //Set cursor to character 2 on line 0
  lcd.print("High  Mid  Low");
  WiFi.mode(WIFI_STA);
  if (esp_now_init() !=  ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    ESP.restart();
    return;
  }
  //esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  for (int ii = 0; ii < 6; ++ii )
  {
    peerInfo.peer_addr[ii] = (uint8_t) broadcastAddress[ii];
  }
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  // Serial.print("Initializing pulse oximeter..");
  // if (!pox.begin()) {
  //   Serial.println("FAILED");
  //   //ESP.restart();
  // } 
  // else {
  //   Serial.println("SUCCESS");
  // }
  // pox.setIRLedCurrent(MAX30100_LED_CURR_14_2MA);//increase the intensity of IR light,  in this code current is 14.2 mA
  //   // Register a callback for the beat detection
  
  // pox.shutdown();

  //xTaskCreate(Heart_rate, "health monitoring", 2048, NULL, 1 , &Heart_rate_handler);
  xTaskCreate(Questions_task, "question asking", 2048, NULL, 1, &Questions_task_handler);
}

// void Heart_rate(void *pvParameters){  
//   pox.resume(); // Start the Pulse Oximeter sensor
//   for(;;){
//     pox.update();
//     if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
//       heart = pox.getHeartRate();
//       spo2 = pox.getSpO2();
//       // Serial.print("Heart rate:");
//       // Serial.print(heart);
//       // Serial.print("bpm / SpO2:");
//       // Serial.print(spo2);
//       // Serial.println("%");
      
//       tsLastReport = millis();
//     }
//     vTaskDelay(1);
//   }
// }

void Questions_task(void *pvParameters){
  for(;;){
    if (digitalRead(buttonHigh) == LOW) {
      Serial.println("High Severity");
      lcd.clear();         
      lcd.setCursor(0,0);   //Set cursor to character 2 on line 0
      lcd.print("High Severity");
      delay(2000);
      askHighSeverityQuestions();
      path_to_choose = 1;
      esp_now_send();
    } else if (digitalRead(buttonMedium) == LOW) {
      Serial.println("Medium Severity");
      lcd.clear();         
      lcd.setCursor(0,0);   //Set cursor to character 2 on line 0
      lcd.print("Medium Severity");
      delay(2000);
      askMediumSeverityQuestions();
      path_to_choose = 2;
      esp_now_send();
    } else if (digitalRead(buttonLow) == LOW) {
      Serial.println("Low Severity");
      lcd.clear();         
      lcd.setCursor(0,0);   //Set cursor to character 2 on line 0
      lcd.print("Low Severity");
      delay(2000);
      askLowSeverityQuestions();
      path_to_choose = 3;
      esp_now_send();
    }
    vTaskDelay(1);
  }
}

void loop(){} 
