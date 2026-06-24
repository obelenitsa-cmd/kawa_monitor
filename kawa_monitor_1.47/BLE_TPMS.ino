#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

int scanTime = 5;  //In seconds
BLEScan *pBLEScan;

//float FW_voltage; // sensor battery voltage V
//int FW_temperature; // sensor temperature в °C
//uint8_t FW_pressure; // sensor pressure 1 байт
uint16_t FW_pressure; // sensor pressure 2 байта

//float RW_voltage; // sensor battery voltage V
//int RW_temperature; // sensor temperature в °C
//uint8_t RW_pressure; // sensor pressure 1 байт
uint16_t RW_pressure; // sensor pressure 2 байта

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {

    /*
    if (advertisedDevice.getAddress().toString() == "5b:84:22:11:11:11") { // адрес датчика FW (устройство без названия)

     if (advertisedDevice.haveManufacturerData() == true) { // Передает ли транслирующее устройство данные производителя? Эти данные содержат значения давления и температуры.
       String strManufacturerData = advertisedDevice.getManufacturerData(); // получить транслируемые данные в виде строки

       byte cManufacturerData[100]; // Вспомогательная переменная для преобразования строки (с заглавной буквой S) в строку в стиле C.
       memcpy(cManufacturerData, strManufacturerData.c_str(), strManufacturerData.length()); // Преобразовать строку в строку в стиле C.

       //FW_voltage = (float)cManufacturerData[2];
       FW_pressure = (uint8_t)cManufacturerData[3]; 
       //FW_press_dat(FW_pressure);
       //FW_temperature = cManufacturerData[4];

       delay(1);
      }
    } 
    
    if (advertisedDevice.getAddress().toString() == "5b:70:51:11:11:11") { // адрес датчика RW (устройство без названия)

      if (advertisedDevice.haveManufacturerData() == true) { // Передает ли транслирующее устройство данные производителя? Эти данные содержат значения давления и температуры.
        String strManufacturerData = advertisedDevice.getManufacturerData(); // получить транслируемые данные в виде строки

        byte cManufacturerData[100]; // Вспомогательная переменная для преобразования строки (с заглавной буквой S) в строку в стиле C.
        memcpy(cManufacturerData, strManufacturerData.c_str(), strManufacturerData.length()); // Преобразовать строку в строку в стиле C.

        //RW_voltage = (float)cManufacturerData[2];
        RW_pressure = (uint8_t)cManufacturerData[3]; 
        //RW_press_dat(RW_pressure);
        //RW_temperature = cManufacturerData[4];
        delay(1);
      }
    }
    */

        //if (advertisedDevice.haveName()) { // Есть ли у устройства название
      //if (advertisedDevice.getName() == "AI-8000") { // Если название "AI-8000", значит, это наш датчик давления.
        //Serial.printf("Found AI-8000 Device: %s \n", advertisedDevice.toString().c_str()); // вывести все полученные данные в последовательный порт

       if (advertisedDevice.getAddress().toString() == "12:30:af:00:02:4b") { // адрес датчика FW
       Serial.println(advertisedDevice.toString().c_str());

         if (advertisedDevice.haveManufacturerData() == true) { // Передает ли транслирующее устройство данные производителя? Эти данные содержат значения давления и температуры.
           String strManufacturerData = advertisedDevice.getManufacturerData(); // получить транслируемые данные в виде строки

           byte cManufacturerData[100]; // Вспомогательная переменная для преобразования строки (с заглавной буквой S) в строку в стиле C.
           memcpy(cManufacturerData, strManufacturerData.c_str(), strManufacturerData.length()); // Преобразовать строку в строку в стиле C.

            //FW_voltage = (float)cManufacturerData[2];
           FW_pressure = (uint16_t)cManufacturerData[3] << 8 | cManufacturerData[4]; //два байта для выходного 16-битного значения, psi
           //FW_press_dat(FW_pressure);
           //FW_temperature = cManufacturerData[4];
           delay(1);
         }
       }
      //}
    //}

    //if (advertisedDevice.haveName()) { // Есть ли у устройства название
      //if (advertisedDevice.getName() == "AI-8000") { // Если название "AI-8000", значит, это наш датчик давления.
        //Serial.printf("Found AI-8000 Device: %s \n", advertisedDevice.toString().c_str()); // вывести все полученные данные в последовательный порт

       if (advertisedDevice.getAddress().toString() == "12:30:af:00:02:ce") { // адрес датчика RW
       Serial.println(advertisedDevice.toString().c_str());

         if (advertisedDevice.haveManufacturerData() == true) { // Передает ли транслирующее устройство данные производителя? Эти данные содержат значения давления и температуры.
           String strManufacturerData = advertisedDevice.getManufacturerData(); // получить транслируемые данные в виде строки

           byte cManufacturerData[100]; // Вспомогательная переменная для преобразования строки (с заглавной буквой S) в строку в стиле C.
           memcpy(cManufacturerData, strManufacturerData.c_str(), strManufacturerData.length()); // Преобразовать строку в строку в стиле C.

            //RW_voltage = (float)cManufacturerData[2];
           RW_pressure = (uint16_t)cManufacturerData[3] << 8 | cManufacturerData[4]; //два байта для выходного 16-битного значения, psi
           //RW_press_dat(RW_pressure);
           //RW_temperature = cManufacturerData[4];
           delay(1);
         }
       }
      //}
    //}
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
