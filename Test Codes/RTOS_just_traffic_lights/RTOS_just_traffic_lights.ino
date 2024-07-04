#define green1 2
#define green2 4
#define green3 5
#define green4 16
#define yellow1 18
#define yellow2 19
#define yellow3 21
#define yellow4 22

int R = 1;
int Traffic_delay = 1000;

void Traffic_light(void *pvParameters); 
TaskHandle_t Traffic_light_handler; 

void Traffic_time_control(void *pvParameters); 
TaskHandle_t Traffic_time_control_handler; 

EventGroupHandle_t Traffic_lights_time_signal; 
 
void setup(){ 
  Serial.begin(115200); 
  pinMode(green1,OUTPUT);
  pinMode(green2,OUTPUT);
  pinMode(green3,OUTPUT);
  pinMode(green4,OUTPUT);
  pinMode(yellow1,OUTPUT);
  pinMode(yellow2,OUTPUT);
  pinMode(yellow3,OUTPUT);
  pinMode(yellow4,OUTPUT);
 
  Traffic_lights_time_signal = xEventGroupCreate(); 
  xTaskCreate(Traffic_light, "Blink Led", 2048, NULL, 1 , &Traffic_light_handler);
  xTaskCreate(Traffic_time_control, "Blink Led", 2048, NULL, 1 , &Traffic_time_control_handler);
  xEventGroupSetBits(Traffic_lights_time_signal, 1); 
  xEventGroupSetBits(Traffic_lights_time_signal, 0);
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