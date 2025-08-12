#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif
bool volatile new_message = false;
const uint8_t num_chars = 255; //cap the input string at 32 characters

//pointer for msg
static char * ptr = NULL;
// static volatile uint8_t msg_flag
void user_input(void * parameter) {
  int ndx = 0;
  char end_marker = '\n';
  char rc;
  char received_char[num_chars];
  // char received_char[num_chars];
  // ptr = (char *)pvPortMalloc(sizeof(char) * num_chars);

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
        if (new_message == false) {
          received_char[ndx] = '\0';
          ptr = (char *)pvPortMalloc(sizeof(char) * (ndx+1));

          //error catching
          configASSERT(ptr);
          memcpy(ptr, received_char, ndx+1);
          Serial.println("New Message is In..");
          new_message = true;
          // if (new_data == true){
          bzero(received_char, num_chars);
          ndx = 0;
        }
      }
    }
  }
}
void show_message_task(void * parameter){
  while (1) {
    if (new_message == true) {
      Serial.println(ptr);
      Serial.print("Free heap(bytes): ");
      Serial.println(xPortGetFreeHeapSize());
      vPortFree(ptr);
      new_message = false;
      ptr = NULL;
      
    }
  }
}
void setup() {
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println("Arduino is Ready");
  // pinMode(led_pin, OUTPUT);
  
  xTaskCreatePinnedToCore(user_input, "Take In Integer", 2048,\
      NULL, 1, NULL, app_cpu);
  xTaskCreatePinnedToCore(show_message_task, "Show at Serial monitor", 2048,
      NULL, 1, NULL, app_cpu);
  vTaskDelete(NULL);
}

void loop() {
  // put your main code here, to run repeatedly:

}
