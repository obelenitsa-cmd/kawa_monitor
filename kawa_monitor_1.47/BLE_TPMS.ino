//FW("5b:84:22:11:11:11"); // 3 байт давление
//RW("5b:70:51:11:11:11"); // 3 байт давление

//FW("12:30:af:00:02:4b"); // 3 и 4 байт давление
//RW("12:30:af:00:02:ce"); // 3 и 4 байт давление

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include "global_flags.h"

// Глобальные переменные для всего проекта
uint16_t FW_pressure = 0;
uint16_t RW_pressure = 0;
bool FW_LowBat = false;
bool RW_LowBat = false;
bool bleIsActive = true; 

BLEScan *pBLEScan = nullptr;

// Адреса ваших датчиков TPMS
BLEAddress targetFW("12:30:af:00:02:4b");
BLEAddress targetRW("12:30:af:00:02:ce");

// Переменная для таймера бесшовного перезапуска
uint32_t bleScanTimer = 0;
#define ASYNC_SCAN_TIME_MS 10000 // Перезапускаем цикл каждые 10 секунд

// Вспомогательная функция для текстовой оценки сигнала в монитор порта
//String getSignalStatus(int rssi) {
//  if (rssi >= -60) return "Отличный";
//  if (rssi >= -80) return "Хороший";
//  if (rssi >= -95) return "Слабый";
//  return "Критический";
//}

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    BLEAddress foundAddress = advertisedDevice.getAddress();
    bool isFW = (foundAddress == targetFW);
    bool isRW = (foundAddress == targetRW);

    if (!isFW && !isRW) return; 

    if (advertisedDevice.haveManufacturerData()) {
      String strManufacturerData = advertisedDevice.getManufacturerData(); 
      uint8_t* cManufacturerData = (uint8_t*)strManufacturerData.c_str();
      size_t dataLength = strManufacturerData.length();

      // Безопасное чтение данных
      if (dataLength >= 5) { 
        uint16_t calculatedPressure = ((uint16_t)cManufacturerData[3] << 8) | cManufacturerData[4];
        uint8_t batVal = (uint8_t)cManufacturerData[2];   
        bool isLowBat = (batVal <= 27); // низкое напряжение батареи, 2,7V или ниже

        // Читаем уровень сигнала 
        int rssi = advertisedDevice.getRSSI();
        //String signalStatus = getSignalStatus(rssi); // считывание сигнала для Монитора порта

        if (isFW) {
          FW_pressure = calculatedPressure;
          FW_LowBat = isLowBat;

          // Вывод в Монитор порта для ПЕРЕДНЕГО колеса
          //Serial.print("[TPMS-FW] Сигнал: ");
          //Serial.print(rssi);
          //Serial.print(" dBm (");
          //Serial.print(signalStatus);
          //Serial.println(")");
        }  
        if (isRW) {
          RW_pressure = calculatedPressure;
          RW_LowBat = isLowBat;

          // Вывод в Монитор порта для ЗАДНЕГО колеса
          //Serial.print("[TPMS-RW] Сигнал: ");
          //Serial.print(rssi);
          //Serial.print(" dBm (");
          //Serial.print(signalStatus);
          //Serial.println(")");
        }
      }
    }
  }
};

void stopBLEScanning() {
  if (pBLEScan != nullptr && bleIsActive) {
    pBLEScan->stop(); 
    bleIsActive = false;
    //Serial.println("[BLE-TPMS] Сканирование остановлено для OTA.");
  }
}

void startBLEScanning() {
  if (pBLEScan != nullptr && !bleIsActive) {
    delay(200); // Пауза для стабилизации радиомодуля после WiFi
    pBLEScan->start(0, [](BLEScanResults results) {}, true);  
    bleIsActive = true;
    //Serial.println("[BLE-TPMS] Непрерывный поток BLE запущен.");
  }
}

void SetupBLE_TPMS() {
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  
  // Включаем аппаратную обработку ВСЕХ пакетов от датчиков без игнорирования повторов
  pBLEScan->setDuplicateFilter(false); 
  
  // Максимально плотное окно приема (99% времени слушаем эфир)
  pBLEScan->setInterval(100); 
  pBLEScan->setWindow(99);    
   
  // Запуск бесконечного сканирования без сохранения результатов в ОЗУ (true в конце)
  pBLEScan->start(0, [](BLEScanResults results) {}, true); 
  
  bleIsActive = true;
  bleScanTimer = millis(); 
}

// Постоянная логика (вызывается в loop() на каждом шаге)
void LoopBLE_TPMS() {
  if (millis() < 5000) return; // Защитная пауза при старте
  // Проверяем OTA триггеры
  if (isOtaStarted == true) {
    if (bleIsActive) stopBLEScanning();
    return; 
  } else {
    if (!bleIsActive) startBLEScanning();
  }
  // ТАЙМЕР БЕСШОВНОГО ПЕРЕЗАПУСКА И СТИРАНИЯ ДАННЫХ ПОЛНОСТЬЮ УДАЛЕН.
  // Сканер работает в режиме raw-потока 24/7 без единой остановки.
}