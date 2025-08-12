// Read ADC values in ISR once per second
// You'll likely need this on vanilla FreeRTOS
//#include <semphr.h>

// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// Pins
static const int adc_pin = 32;

// Globals
static hw_timer_t *timer = NULL;
static volatile uint16_t val;
static SemaphoreHandle_t bin_sem = NULL;

//*****************************************************************************
// Interrupt Service Routines (ISRs)

// This function executes when timer reaches max (and resets)
void IRAM_ATTR onTimer() {

  BaseType_t task_woken = pdFALSE;

  // Perform action (read from ADC)
  val = analogRead(adc_pin);

  // Give semaphore to tell task that new value is ready
  xSemaphoreGiveFromISR(bin_sem, &task_woken);

  // Exit from ISR (Vanilla FreeRTOS)
  //portYIELD_FROM_ISR(task_woken);

  // Exit from ISR (ESP-IDF)
  if (task_woken) {
    portYIELD_FROM_ISR();
  }
}

//*****************************************************************************
// Tasks

// Wait for semaphore and print out ADC value when received
void printValues(void *parameters) {

  // Loop forever, wait for semaphore, and print value
  while (1) {
    xSemaphoreTake(bin_sem, portMAX_DELAY);
    Serial.println(val); 
  }
}

//*****************************************************************************
// Main (runs as its own task with priority 1 on core 1)

void setup() {

  // Configure Serial
  Serial.begin(115200);

  // Wait a moment to start (so we don't miss Serial output)
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("---FreeRTOS ISR Buffer Demo---");

  // Create semaphore before it is used (in task or ISR)
  bin_sem = xSemaphoreCreateBinary();

  // Force reboot if we can't create the semaphore
  if (bin_sem == NULL) {
    Serial.println("Could not create semaphore");
    ESP.restart();
  }

  // Start task to print out results (higher priority!)
  xTaskCreatePinnedToCore(printValues,
                          "Print values",
                          1024,
                          NULL,
                          2,
                          NULL,
                          app_cpu);
  timer = timerBegin(10000000);
  timerAttachInterrupt(timer, &onTimer);
  timerAlarm(timer, 10000000, true, 0); 
  timerStart(timer);
  // Delete "setup and loop" task
  vTaskDelete(NULL);
}

void loop() {
  // Do nothing, forever
}