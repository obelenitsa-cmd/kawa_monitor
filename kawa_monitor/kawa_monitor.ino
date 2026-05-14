//bool Warning_state = false;

TaskHandle_t Task1;
TaskHandle_t Task2;

void Task1code(void *parameter) {
  for (;;) {  // бесконечный цикл
   LoopKline(); 
   //LoopBLE_TPMS();
    delay(1);
  }
}

void Task2code(void *parameter) {
  for (;;) {  // бесконечный цикл
   //LoopKline(); 
   LoopBLE_TPMS();
    delay(1);
  }
}

void setup() {
  SetupDisplay();
  SetupBLE_TPMS();

  xTaskCreatePinnedToCore(Task1code, "Task1", 50000, NULL, 1, &Task1, 0 ); //Ядро 0 
  delay(1);
  xTaskCreatePinnedToCore(Task2code, "Task2", 50000, NULL, 1, &Task2, 1); //Ядро 1
}

 void loop() {
   //LoopKline(); 
   //LoopBLE_TPMS();
    //========================================================= независимый цикл предупреждения
    //unsigned long cur_millis = millis();
    //if (cur_millis - timer >= 500) {
    //  timer = cur_millis;        
    //  Warning_state = !Warning_state;
    //  Warning(Warning_state); 
    //}
    //=========================================================
   }



 

