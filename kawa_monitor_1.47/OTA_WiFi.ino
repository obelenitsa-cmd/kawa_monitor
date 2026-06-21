#include <WiFi.h>          
#include <ArduinoOTA.h>

const char* ssid     = "Update";  // Сеть
const char* password = "12345678"; // Пароль

// Таймаут для поиска сети при старте
const unsigned long WIFI_TIMEOUT = 30000; // 30 секунд

unsigned long startTime = 0;       
bool isTimeoutTimerActive = true;  
bool isOtaStarted = false;     

void SetupOTAWiFi() {
  //Serial.println("\n[Система] Запуск.Поиск Wi-Fi для OTA в фоне...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password); 

  startTime = millis(); // Фиксируем время старта
}

void LoopOTAWiFi() {
  
  // 1. ЗАЩИТНЫЙ ТАЙМЕР НА 30 СЕКУНД (если роутер не найден)
  if (isTimeoutTimerActive) {
    if (millis() - startTime >= WIFI_TIMEOUT) {
      //Serial.println("\n[Wi-Fi] Нет сети. Режим OTA отменен.");
      
      WiFi.disconnect(true); // Отключаемся от сети
      WiFi.mode(WIFI_OFF); // Полностью выключаем радиомодуль Wi-Fi
      isTimeoutTimerActive = false; // Отключаем таймер навсегда
    }
  }

  // 2. ИНИЦИАЛИЗАЦИЯ ARDUINO OTA ПРИ УСПЕШНОМ ПОДКЛЮЧЕНИИ
  if (WiFi.status() == WL_CONNECTED && !isOtaStarted) {
    isOtaStarted = true;
    //GearDisplay(isOtaStarted);
    isTimeoutTimerActive = false; // Отменяем защитный таймаут

    // Настройки беспроводной прошивки (без веб-сервера)
    ArduinoOTA.setHostname("ESP32-S3_LCD_Moto"); // Имя платы в сети
    // ArduinoOTA.setPassword("admin");        // Раскомментируйте, если нужен пароль на прошивку

    // Колбэки для визуализации процесса в Serial (не блокируют loop)
    ArduinoOTA.onStart([]() {
      //Serial.println("\n[OTA] Начало удаленной прошивки...");
    });
    ArduinoOTA.onEnd([]() {
      //Serial.println("\n[OTA] Прошивка успешно завершена! Перезагрузка...");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      //Serial.printf("[OTA] Прогресс: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      //Serial.printf("[OTA] Ошибка [%u]\n", error);
    });

    ArduinoOTA.begin(); // Запуск службы прошивки
    //Serial.println("[OTA] Служба запущена! Плата доступна для прошивки через IDE.");
    //Serial.print("[OTA] IP-адрес: ");
    //Serial.println(WiFi.localIP());
  }
  else if (WiFi.status() == WL_DISCONNECTED && isOtaStarted) {
    isOtaStarted = false;
    isTimeoutTimerActive = true; // Запускаем защитный таймаут
  }

  // 3. ОБРАБОТЧИК СЕТЕВОГО ПОРТА OTA (работает асинхронно, если сеть есть)
  if (isOtaStarted) {
    ArduinoOTA.handle(); // Быстрая проверка сетевого порта на наличие новой прошивки
  }

}