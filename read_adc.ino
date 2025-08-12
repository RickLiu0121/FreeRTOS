#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

static float average = 0;
static const int adc_pin = 32;
static volatile int val = 0;
static int num_sample = 10;

static uint64_t frequency = 10000000;
static uint64_t timer_max_count = 1000000;
static hw_timer_t *timer = NULL;
static const uint16_t buf_size = 255;
static volatile int adc_buf[buf_size];

static uint8_t head = 0;
static uint8_t tail = 0;
//one binary semaphore handler for buffer access
static SemaphoreHandle_t buf_sem = NULL;
//one binary semaphore handler for average value access
static SemaphoreHandle_t avg_mutex = NULL;


void IRAM_ATTR onTimer() {
  BaseType_t task_woken = pdFALSE;
  val = analogRead(adc_pin);
  adc_buf[head] = val;
  head = (head + 1) % buf_size;
  if (head >= num_sample) {
    xSemaphoreGiveFromISR(buf_sem, &task_woken);
    if (task_woken) {
      portYIELD_FROM_ISR();
    }
  }

}

void compute_avg(void * pmtr) {
  // int samples[num_samples];
  float sum;
  while (1) {
    sum = 0;
    xSemaphoreTake(buf_sem, portMAX_DELAY);
    for (int i = 0; i < num_sample; i++) {
      // samples[i] = adc_buf[tail]
      sum += adc_buf[tail];
      tail = (tail + 1) % buf_size;
    }
    xSemaphoreTake(avg_mutex, portMAX_DELAY);
    average = sum / num_sample;
    xSemaphoreGive(avg_mutex);
  }
}
void cmdline(void * pmtr) {
  char received_char[buf_size];
  char rc;
  uint8_t ndx = 0;
  char avg[] = "avg";
  bzero(received_char, buf_size);

  for (;;) {
    if (Serial.available() > 0) {
      rc = Serial.read();
      if (rc != '\n') {
        if (ndx < buf_size) {
          received_char[ndx] = rc;
          ndx++;
          Serial.print(rc);
        }
      }
      //logic when user type in "\n"
      else {
        received_char[ndx] = '\0';
        Serial.println();
        //echo the input;
        // Serial.println(received_char);
        //check if "avg" is the input
        if (strncmp(received_char, avg, 3) == 0){
          xSemaphoreTake(avg_mutex, portMAX_DELAY);
          Serial.println(average);
          xSemaphoreGive(avg_mutex);
        }
        bzero(received_char, buf_size);
        ndx = 0;
      }
    }
  }
}
void setup() {
  Serial.begin(115200);
  // Wait a moment to start (so we don't miss Serial output)
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("---FreeRTOS ISR Buffer Demo---");
  xTaskCreatePinnedToCore(cmdline, "Take in user input", 2048, NULL, 1, NULL, app_cpu);
  xTaskCreatePinnedToCore(compute_avg, "Average every 10 samples", 2048, NULL, 1, NULL, app_cpu);

  buf_sem = xSemaphoreCreateBinary();
  avg_mutex = xSemaphoreCreateMutex();
  timer = timerBegin(10000000);
  timerAttachInterrupt(timer, &onTimer);
  timerAlarm(timer, 1000000, true, 0); 
  timerStart(timer);

  // Delete "setup and loop" task
  vTaskDelete(NULL);
}

void loop() {
  // put your main code here, to run repeatedly:

}
