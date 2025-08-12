//check to see if a character is in serial buffer
//to keep the indication LED on for 5s
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

const int led_pin = 32;
static TimerHandle_t led_timer = NULL;
const int num_chars = 255;
void user_input(void * pmtr) {
  int ndx = 0;
  char end_marker = '\n';
  char rc;
  char received_char[num_chars];
  bzero(received_char, num_chars);

  while (1) {
    if (Serial.available() > 0) {
      rc = Serial.read();
      digitalWrite(led_pin, HIGH);
      //insert timer logic to turn off LED
      // Serial.println("Start Timer");
      xTimerStart(led_timer, portMAX_DELAY);
      if (rc != end_marker) {
        received_char[ndx] = rc;
        Serial.print(received_char[ndx]);
        ndx++;
        if (ndx >= num_chars)
          ndx = num_chars - 1;
      }
      else {
        received_char[ndx] = '\0';
        // Serial.println(received_char);
        bzero(received_char, num_chars);
        ndx = 0;
      }
    }
  }
}
void led_call_back(TimerHandle_t xTimer) {
  digitalWrite(led_pin, LOW);
  Serial.println("No input within last 5 seconds");
}
void setup() {
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println("Arduino is Ready");
  pinMode(led_pin, OUTPUT);
  xTaskCreatePinnedToCore(user_input, "Take In message", 1200,\
      NULL, 1, NULL, app_cpu);
  led_timer = xTimerCreate("LED timer", 5000 / portTICK_PERIOD_MS, pdFALSE, (void *) 0, \
      led_call_back);
  if (led_timer == NULL)
    Serial.println("Timer failed to create");
}

void loop() {
  // put your main code here, to run repeatedly:

}
