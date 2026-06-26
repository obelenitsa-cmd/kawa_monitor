#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

int scanTime = 5;  //In seconds
BLEScan *pBLEScan;
 
uint16_t FW_pressure; // sensor pressure 2 байта (1 байт uint8_t)
uint16_t RW_pressure; // sensor pressure 2 байта (1 байт uint8_t)

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {

       if (advertisedDevice.getAddress().toString() == "12:30:af:00:02:4b") { // адрес датчика FW
       //Serial.println(advertisedDevice.toString().c_str());

         if (advertisedDevice.haveManufacturerData() == true) { // Передает ли транслирующее устройство данные производителя? Эти данные содержат значения давления и температуры.
           String strManufacturerData = advertisedDevice.getManufacturerData(); // получить транслируемые данные в виде строки

           byte cManufacturerData[100]; // Вспомогательная переменная для преобразования строки (с заглавной буквой S) в строку в стиле C.
           memcpy(cManufacturerData, strManufacturerData.c_str(), strManufacturerData.length()); // Преобразовать строку в строку в стиле C.

            //FW_pressure = (uint8_t)cManufacturerData[3]; // для датчика "5b:84:22:11:11:11" 1 байт
           FW_pressure = (uint16_t)cManufacturerData[3] << 8 | cManufacturerData[4]; //два байта для выходного 16-битного значения, psi для "12:30:af:00:02:4b"

           delay(1);
         }
       }

       if (advertisedDevice.getAddress().toString() == "12:30:af:00:02:ce") { // адрес датчика RW
       //Serial.println(advertisedDevice.toString().c_str());

         if (advertisedDevice.haveManufacturerData() == true) { // Передает ли транслирующее устройство данные производителя? Эти данные содержат значения давления и температуры.
           String strManufacturerData = advertisedDevice.getManufacturerData(); // получить транслируемые данные в виде строки

           byte cManufacturerData[100]; // Вспомогательная переменная для преобразования строки (с заглавной буквой S) в строку в стиле C.
           memcpy(cManufacturerData, strManufacturerData.c_str(), strManufacturerData.length()); // Преобразовать строку в строку в стиле C.

            //RW_pressure = (uint8_t)cManufacturerData[3]; // Для датчика "5b:70:51:11:11:11" 1 байт
           RW_pressure = (uint16_t)cManufacturerData[3] << 8 | cManufacturerData[4]; //два байта для выходного 16-битного значения, psi для датчика "12:30:af:00:02:ce"
           delay(1);
         }
       }
  }
};

void SetupBLE_TPMS() {

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();  //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);  //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
}

void LoopBLE_TPMS() {
  BLEScanResults *foundDevices = pBLEScan->start(scanTime, false);
  pBLEScan->clearResults();  // delete results fromBLEScan buffer to release memory
  delay(50);
}
