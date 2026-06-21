TaskHandle_t Task1;
TaskHandle_t Task2;

void Task1code(void *parameter) {
  for (;;) {  // бесконечный цикл 
   LoopBLE_TPMS();
   LoopOTAWiFi();
   delay(1);

    // Проверка свободного стека задачи Task1
    //UBaseType_t freeStack = uxTaskGetStackHighWaterMark(NULL); //Оптимальный «запас безопасности» — около 500–1000 байт.
    //Serial.print("Task1 Free Stack (bytes): ");
    //Serial.println(freeStack);
    //vTaskDelay(pdMS_TO_TICKS(200));
  }
}

void Task2code(void *parameter) {
  for (;;) {  // бесконечный цикл
   LoopKline(); 
   delay(1);
   // Тут второй стек проверить не получится - нет продолжения петли после LoopKline(); 
  }
}

void setup() {
  Serial.begin(115200);
  SetupDisplay();
  SetupBLE_TPMS();
  SetupOTAWiFi();

  xTaskCreatePinnedToCore(Task1code, "Task1", 4096, NULL, 1, &Task1, 0 ); //Ядро 0 
  delay(1);
  xTaskCreatePinnedToCore(Task2code, "Task2", 6144, NULL, 1, &Task2, 1); //Ядро 1  
}

void loop() {       
   //LoopPHOTO();
   LoopDisplay();
   //printMemoryStatus();
}

//void printMemoryStatus() {  // вставить в любой loop через паузу
  //Serial.print("Current Free Heap: ");
  //Serial.print(ESP.getFreeHeap()); // Текущий доступный объем оперативной памяти. Если значение постоянно уменьшается со временем: В коде есть утечка памяти
  //Serial.print(" bytes | Historical Min Free Heap: ");
  //Serial.print(ESP.getMinFreeHeap()); // показывает «критическую точку»: самый низкий уровень свободной памяти с момента включения платы. Для отслеживания утечек памяти
  //Serial.println(" bytes");
//}
