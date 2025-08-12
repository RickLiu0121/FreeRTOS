
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
static const int led_pin = 32;
#endif     

void toggleLED(void * parameter){
  while (1){
    digitalWrite(led_pin, HIGH);
    vTaskDelay(200 / portTICK_PERIOD_MS);
     digitalWrite(led_pin, LOW);
    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}

void setup() {
  // put your setup code here, to run once:
  pinMode(led_pin, OUTPUT);
  xTaskCreatePinnedToCore(toggleLED, "Toggle LED", 1024,     NULL, 1, NULL, app_cpu); //run one core on
    // xTaskCreatePinnedToCore(toggleLED_2, "Toggle LED _2", 1024, NULL, 1, NULL, app_cpu); //run one core one

}

void loop() {
  // put your main code here, to run repeatedly:
  // digitalWrite(led_pin, HIGH);
  // delay(1000);
  // digitalWrite(led_pin, LOW);
  // delay(1000);
}
