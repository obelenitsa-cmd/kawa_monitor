TaskHandle_t Task1;
TaskHandle_t Task2;

void Task1code(void *parameter) {
  for (;;) {  // бесконечный цикл
   LoopKline(); 
    delay(1);
  }
}

void Task2code(void *parameter) {
  for (;;) {  // бесконечный цикл 
   LoopBLE_TPMS();
    delay(1);
  }
}

void setup() {
  SetupDisplay();
  SetupBLE_TPMS();

  xTaskCreatePinnedToCore(Task1code, "Task1", 6144, NULL, 1, &Task1, 0 ); //Ядро 0 
  delay(1);
  xTaskCreatePinnedToCore(Task2code, "Task2", 8192, NULL, 1, &Task2, 1); //Ядро 1
}

 void loop() {
}

/*
#include <WiFi.h>
#include <ArduinoOTA.h>

const char* ssid = "Ваш_SSID";
const char* password = "Ваш_Пароль";

void setup() {
  Serial.begin(115200);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWi-Fi Подключен!");
  Serial.print("IP адрес: ");
  Serial.println(WiFi.localIP());

  // Настройка OTA-портов и пароля (опционально)
  ArduinoOTA.setPort(8266);
  ArduinoOTA.setHostname("esp32s3-lcd-147");
  // ArduinoOTA.setPassword("admin"); // Раскомментируйте для защиты паролем

  // Обработчики событий OTA (вывод статуса)
  ArduinoOTA.onStart([]() {
    String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
    Serial.println("Начало обновления: " + type);
  });
  
  ArduinoOTA.onEnd([]() {
    Serial.println("\nОбновление завершено!");
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Прогресс: %u%%\r", (progress / (total / 100)));
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Ошибка [%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Ошибка авторизации");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Ошибка начала");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Ошибка подключения");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Ошибка получения");
    else if (error == OTA_END_ERROR) Serial.println("Ошибка завершения");
  });

  ArduinoOTA.begin();
}

void loop() {
  ArduinoOTA.handle(); // Важно! Метод должен вызываться постоянно
  
  // Остальной код loop() не должен содержать долгих delay()

 */

