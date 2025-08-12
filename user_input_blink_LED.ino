#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif
#include <string.h>

static const int led_pin = 32;

static TaskHandle_t task_1 = NULL;
static TaskHandle_t task_2 = NULL;
static int led_delay = 500;
// bool new_data = false;
const int num_chars = 32; //cap the input string at 32 characters
char received_char[num_chars];

void user_input(void * parameter) {
  int ndx = 0;
  char end_marker = '\n';
  char rc;
  bzero(received_char, num_chars);
  while (1) {
    if (Serial.available() > 0) {
      rc = Serial.read();
      if (rc != end_marker) {
          received_char[ndx] = rc;
          ndx++;
          if (ndx >= num_chars)
            ndx = num_chars - 1;
          
      }
      else {
        received_char[ndx] = '\0';
        // new_data = true;
        led_delay = strtol(received_char, NULL, 0);
        Serial.print("This is in..");
        Serial.println(led_delay);
        // if (new_data == true){
        bzero(received_char, num_chars);
         ndx = 0;
        
      }
    }
  }
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println("Arduino is Ready");
  pinMode(led_pin, OUTPUT);
  
  xTaskCreatePinnedToCore(user_input, "Take In Integer", 1024,\
      NULL, 1, NULL, app_cpu);
  xTaskCreatePinnedToCore(blink_led, "Blink led at interval", 1024,
      NULL, 2, NULL, app_cpu);
  vTaskDelete(NULL);

}
void blink_led(void * parameter){
  while (1) {
    digitalWrite(led_pin, HIGH);
    vTaskDelay(led_delay / portTICK_PERIOD_MS);
    digitalWrite(led_pin, LOW);
    vTaskDelay(led_delay / portTICK_PERIOD_MS);
  }
}
//program won't reach this point because control is passed to the scheduler
void loop() {
  // user_input();
  //show_input();
}
