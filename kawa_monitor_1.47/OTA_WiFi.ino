#include <WiFi.h>          
#include <ArduinoOTA.h>
#include "global_flags.h"

std::atomic<bool> isOtaStarted(false);

const char* ssid     = "Update";  // Сеть
const char* password = "12345678"; // Пароль

// Таймаут для поиска сети при старте
const unsigned long WIFI_TIMEOUT = 30000; // 30 секунд

unsigned long startTime = 0;       
bool isTimeoutTimerActive = true;
volatile bool wifiDisconnectedEventTriggered = false; 

void SetupOTAWiFi() {

  WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
    if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
      wifiDisconnectedEventTriggered = true; // Безопасная операция, не вызывающая Core Panic
    }
  }, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
}

void LoopOTAWiFi() {
  
  // ==========================================
  // 1. БЕЗОПАСНЫЙ СБРОС ФЛАГА ПРИ ОБРЫВЕ СВЯЗИ
  // ==========================================
  if (wifiDisconnectedEventTriggered) {
    wifiDisconnectedEventTriggered = false; 
    
    if (isOtaStarted) {
      isOtaStarted = false;          // МГНОВЕННО передаем 0 в BLE и дисплей!
      isTimeoutTimerActive = true;   // Активируем защитный таймер
      startTime = millis();          // Фиксируем точку отсчета времени
      
      // МЫ УБРАЛИ ОТСЮДА WiFi.disconnect и WiFi.mode, чтобы не ломать Bluetooth!
      //Serial.println("\n[OTA-LOOP] WiFi Lost! isOtaStarted safely set to 0. Notification sent to BLE.");
    }
  }

  // ==========================================
  // 2. ЗАЩИТНЫЙ ТАЙМЕР НА 30 СЕКУНД
  // ==========================================
  if (isTimeoutTimerActive) {
    if (millis() - startTime >= WIFI_TIMEOUT) {
      isTimeoutTimerActive = false;  
      isOtaStarted = false;          
      WiFi.mode(WIFI_OFF); // Мягкое выключение, если BLE к этому моменту готов
      //Serial.println("\n[Wi-Fi] Время ожидания истекло. Режим OTA отменен.");
    }
  }

  // ==========================================
  // 3. ИНИЦИАЛИЗАЦИЯ OTA (СТРОГО ОДИН РАЗ при подключении)
  // ==========================================
  // Добавлена статическая переменная, чтобы код инициализации НЕ вызывался по кругу!
  static bool isOtaInitialized = false;

  if (WiFi.status() == WL_CONNECTED) {
    if (!isOtaInitialized) {
      isOtaStarted = true;
      isTimeoutTimerActive = false; // Отменяем защитный таймаут поиска сети
      isOtaInitialized = true;      // Блокируем повторный вход в этот IF

      //Serial.print("OTAisOtaStarted ");
      //Serial.println(isOtaStarted);

      // Настройки беспроводной прошивки
      ArduinoOTA.setHostname("ESP32-S3_LCD_Moto"); 

      // Колбэки для визуализации процесса
      ArduinoOTA.onStart([]() { 
        //Serial.println("\n[OTA] Начало удаленной прошивки..."); 
      });
      ArduinoOTA.onEnd([]() { 
        //Serial.println("\n[OTA] Успешно! Перезагрузка..."); 
      });
      ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        //Serial.printf("[OTA] Прогресс: %u%%\r", (progress / (total / 100)));
      });
      ArduinoOTA.onError([](ota_error_t error) { 
        //Serial.printf("[OTA] Ошибка [%u]\n", error); 
      });

      ArduinoOTA.begin(); // Запуск службы прошивки
      //Serial.println("[OTA] Служба запущена успешно!");
    }
  } else {
    // Если статус WiFi стал не WL_CONNECTED, сбрасываем флаг инициализации для будущего переподключения
    isOtaInitialized = false; 
  }

  // ==========================================
  // 4. ОБРАБОТЧИК СЕТЕВОГО ПОРТА OTA
  // ==========================================
  if (isOtaStarted == true) {
    ArduinoOTA.handle(); 
  }
}