TaskHandle_t Task1;
TaskHandle_t Task2;

void Task1code(void *parameter) {
  for (;;) {  // бесконечный цикл 
   LoopOTAWiFi();
   LoopBLE_TPMS();
   vTaskDelay(1);
  }
}

void Task2code(void *parameter) {
  for (;;) {  // бесконечный цикл
   LoopKline(); 
   vTaskDelay(1);
  }
}

void setup() {
  //Serial.begin(115200);
  SetupDisplay();
  SetupOTAWiFi();
  SetupBLE_TPMS();

  xTaskCreatePinnedToCore(Task1code, "Task1", 4096, NULL, 1, &Task1, 0 ); //Ядро 0 
  delay(1);
  xTaskCreatePinnedToCore(Task2code, "Task2", 6144, NULL, 1, &Task2, 1); //Ядро 1  
}

void loop() {       
   LoopDisplay();
   delay(1);
}

