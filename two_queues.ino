#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

static const uint8_t msg_queue_len = 5;
static const int led_pin = 32;
const int num_chars = 32;
static QueueHandle_t que_hand_1;
static QueueHandle_t que_hand_2;


struct msg_from_B{
  int num_blinks;
  char blink_100[8];
};

void user_input_task_A(void * pmtr){
  char received_char[num_chars];
  const char end_marker = '\n'; 
  bzero(received_char, num_chars);
  struct msg_from_B msg;
  int ndx = 0;
  char rc;
  while (1){
    if (xQueueReceive(que_hand_2, (void *)&msg, 0) == pdTRUE){
      Serial.print("The LED ");
      Serial.println(msg.blink_100);

      Serial.print("The LED has blinked in total ");
      Serial.print(msg.num_blinks);
      Serial.println(" times");
    }
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
        Serial.println(received_char);
        char delay[] = "delay ";
        if (strncmp(received_char,delay, 6) == 0) {
          int new_delay = strtol(&received_char[6],NULL,0);
          if (xQueueSend(que_hand_1, (void *)&new_delay, 5) != pdTRUE)
            Serial.println("Queue Full");
          else
            Serial.print("Copied value to queue 1: ");
            Serial.println(new_delay);
        }
        bzero(received_char, num_chars);
        ndx = 0;
      }
    }
  }
}
void control_LED_task_B(void * pmtr){
  struct msg_from_B msg;
  int received_delay;
  int blink_count;
  int led_delay = 500;
  while (1) {
    if (xQueueReceive(que_hand_1, (void *)&received_delay, 0) == pdTRUE){
      led_delay = received_delay;
      Serial.print("Received new delay interval from queue 1: ");
      Serial.println(led_delay);
    }
    digitalWrite(led_pin, HIGH);
    vTaskDelay(led_delay / portTICK_PERIOD_MS);
    digitalWrite(led_pin, LOW);
    vTaskDelay(led_delay / portTICK_PERIOD_MS);
    blink_count++;
    if (blink_count % 100 == 0) {
      msg.num_blinks = blink_count;
      strcpy(msg.blink_100, "Blinked");
      if (xQueueSend(que_hand_2, (void *)&msg, 5) != pdTRUE)
        Serial.println("Queue Full");
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(led_pin, OUTPUT);
  que_hand_1 = xQueueCreate(msg_queue_len, sizeof(int));
  que_hand_2 = xQueueCreate(msg_queue_len, sizeof(msg_from_B));
  xTaskCreatePinnedToCore(user_input_task_A, "Take in user input", 4000, NULL, 1, NULL, app_cpu);
  xTaskCreatePinnedToCore(control_LED_task_B, "Task B", 4000, NULL, 1, NULL, app_cpu);


}

void loop() {
  // put your main code here, to run repeatedly:

}
