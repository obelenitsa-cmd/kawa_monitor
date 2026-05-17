#include <HardwareSerial.h>

HardwareSerial KSerial(1); // Создаем объект для UART1. Если использовать UART0 при включении питания летит "мусор" (отладочная информация)

#define MAXSENDTIME     100       // Время ожидания 

const uint8_t ISORequestByteDelay = 7; // Время между байтами ; по ISO14230-2: 5-20
const uint8_t ISORequestDelay = 60; // Время между запросами; по ISO14230-2: 55-5000 Время между окончанием ответа ЭБУ и началом нового запроса  или время между окончанием запроса  и началом нового запроса, если ЭБУ не отвечает (фактически расчетное 70)

const uint8_t ECUaddr = 0x11; // Kawasaki = 0x11
const uint8_t myAddr = 0xF1; 

bool ECUconnected = false;
bool disconn = false;
bool VltErr, TmpErr, GeaErr;

uint8_t gear, pidgear; 
uint8_t volt, pidvolt; 
uint8_t temp, pidtemp; 

unsigned long previousMillis = 0; // Время последнего сброса таймера

 void LoopKline() {
    uint8_t cmdSize;
    uint8_t cmdBuf[6];
    uint8_t respSize;
    uint8_t respBuf[12];
    uint8_t ect;

    if (!ECUconnected) {
      if (millis() - previousMillis >= 2000) { // СОБЫТИЕ: время истекло, сброса не было
        DataDisplay(disconn);
        previousMillis = millis(); // Чтобы событие не повторялось бесконечно до нового сброса:
      }

      ECUconnected = initPulse();
    }

    while (ECUconnected) {
      VltErr = 0;
      TmpErr = 0;
      GeaErr = 0;
      previousMillis = millis();        // СБРОС ТАЙМЕРА: запоминаем текущее время

      cmdSize = 2;       // каждый запрос пакет из 2-х байт
      cmdBuf[0] = 0x21;  // регистр команды запроса
      
        for (uint8_t i = 0; i < 5; i++) {
          respBuf[i] = 0;
        }
        
        //========================================================= Напряжение 
        cmdBuf[1] = 0x0A;  // запрос напряжения
        respSize = sendRequest(cmdBuf, respBuf, cmdSize, 12);
       
         pidvolt = (uint8_t) respBuf[1];  // данные о PID
         volt = (uint8_t) respBuf[2]; // данные в байте напряжения 
        if (respSize == 3) {
          if (pidvolt == 0x0A) {
          VoltDisplay(volt); //напряжение
          }
        }
        else if (respSize == 0) { // Ошибка
          VltErr = 1;
        }
        delay(ISORequestDelay);

        //========================================================= Температура 
        cmdBuf[1] = 0x06;  // запрос температуры
        respSize = sendRequest(cmdBuf, respBuf, cmdSize, 12);

         pidtemp = (uint8_t) respBuf[1];  // данные о PID
         temp = (uint8_t) respBuf[2]; // данные в байте температуры

        if (respSize == 3) {
          if (pidtemp == 0x06) {
          TempDisplay(temp); //температура
            }
          }
        else if (respSize == 0) { // Ошибка
          TmpErr = 1;
          }

        delay(ISORequestDelay);  
        
        //========================================================= Передача         
        cmdBuf[1] = 0x0B;  // запрос передачи
        respSize = sendRequest(cmdBuf, respBuf, cmdSize, 12);

         pidgear = (uint8_t) respBuf[1];  // данные о PID
         gear = (uint8_t) respBuf[2]; // данные в байте передачи 
        if (respSize == 3) {
          if (pidgear == 0x0B) {
           GearDisplay(gear); //передача
          }
        }
        else if (respSize == 0) { // нет данных
          GearDisplay(45); 
          GeaErr = 1;
        }
        delay(ISORequestDelay);

        //========================================================= нет никаких данных
        if (VltErr == 1 && TmpErr == 1 && GeaErr == 1) {
          ECUconnected = false; 
          break;
        }

      }
    delay(1);  // авто-переподключение
  }
  

 // Инициализация подключения к ECU. ISO 14230-2 "Быстая инициализация".
 bool initPulse() {

   uint8_t rLen;
   uint8_t req[2];
   uint8_t resp[3];

   //Serial.end();
   KSerial.end();

   delay(10);
   pinMode(11, INPUT_PULLUP); // Подтягиваем RX к 3.3В
   pinMode(10, OUTPUT);
   digitalWrite(10, HIGH);
   delay(800);
  
   digitalWrite(10, LOW);
   delay(25); // В некоторых случаях 70
   digitalWrite(10, HIGH);
   delay(25);


   KSerial.begin(10400 /*в некоторых случаях 10417*/, SERIAL_8N1, 11, 10); // Инициализация: Baud rate, Конфигурация, RX пин, TX пин
  
   req[0] = 0x81;       // Начало связи - пакет из одного байта "0x81".
   rLen = sendRequest(req, resp, 1, 3);

   // Положительный ответ начала связи должен быть размером в 3 байта: 0xC1 0xEA 0x8F
   if ((rLen == 3) && (resp[0] == 0xC1) && (resp[1] == 0xEA) && (resp[2] == 0x8F)) {
     // в случае успеха сразу отправляем запрос на диагностику 2 байта: 0x10 0x80
     req[0] = 0x10;
     req[1] = 0x80;
    
     rLen = sendRequest(req, resp, 2, 3);

    } else { // если ответа нет и время ожидания истекло все равно отправляем запрос на диагностику 2 байта: 0x10 0x80
       req[0] = 0x10;
       req[1] = 0x80;
    
       rLen = sendRequest(req, resp, 2, 3);
      }

    
    //Если ответ 50 80 - Запрос на начало диагностики принят
     if ((rLen == 2) && (resp[0] == 0x50) && (resp[1] == 0x80)) {  
      return true;
     } 

    return false; // в противном случае инициализация провалена.
      
  }

 // Отправить запрос в ECU и дождаться ответа. Возвращает количество байт возвращенного ответа.
 uint8_t sendRequest(const uint8_t *request, uint8_t *response, uint8_t reqLen, uint8_t maxLen) {
  
  uint8_t buf[16], rbuf[16];
  uint8_t bytesToSend;
  uint8_t bytesSent = 0;
  uint8_t bytesToRcv = 0;
  uint8_t bytesRcvd = 0;
  uint8_t rCnt = 0;
  uint8_t c, z;
  bool forMe = false;
  char radioBuf[32];
  uint32_t startTime;
  
  for (uint8_t i = 0; i < 16; i++) {
    buf[i] = 0;
  }
  
  for (uint8_t i = 0; i < maxLen; i++) {
    response[i] = 0;  // обнулить буфер ответов до значения maxLen
  }

  if (reqLen == 1) {
    buf[0] = 0x81;
  }
  else {
    buf[0] = 0x80;
  }
  
  buf[1] = ECUaddr;
  buf[2] = myAddr;

  if (reqLen == 1) {
    buf[3] = request[0];
    buf[4] = calcChecksum(buf, 4);
    bytesToSend = 5;
  }
  else {
    buf[3] = reqLen;
    for (z = 0; z < reqLen; z++) {
      buf[4 + z] = request[z];
    }
    buf[4 + z] = calcChecksum(buf, 4 + z);
    bytesToSend = 5 + z;
  }
  
  for (uint8_t i = 0; i < bytesToSend; i++) {
    bytesSent += KSerial.write(buf[i]);    // отправить команду
    delay(ISORequestByteDelay);
  }
  
 // Подождать нужное время для получения ответа.
  
  startTime = millis();

  while ((bytesRcvd <= maxLen) && ((millis() - startTime) < MAXSENDTIME)) {    // ожидание ответа
    
    if (KSerial.available()) {
    
      c = KSerial.read();
      startTime = millis(); // сбросить таймер для каждого полученного байта

      rbuf[rCnt] = c;
      switch (rCnt) {
      
      case 0:
        
        if (c == 0x81) { // должен быть адресный пакет либо 0x80, либо 0x81
          bytesToRcv = 1;
        }
        else if (c == 0x80) {
          bytesToRcv = 0;
        }
        rCnt++;
        break;
        
      case 1:
        if (c == myAddr) { // целевой адрес
          forMe = true;
        }
        rCnt++;
        break;
        
      case 2:
        if (c == ECUaddr) { // исходный адрес
          forMe = true;
        } else if (c == myAddr) {
          forMe = false; // игнорировать ЭХО
        }
        rCnt++;
        break;
        
      case 3:
        if (bytesToRcv == 1) { // количество байтов ответа
          bytesRcvd++;
          if (forMe) {
            response[0] = c; // сохранение однобайтового ответа
          }
        } else {
          bytesToRcv = c;  // количество байтов данных в пакете
        }
        rCnt++;
        break;
        
      default:
        if (bytesToRcv == bytesRcvd) {
          if (forMe) { // Проверка или игнорирование контрольной суммы
            
            if (calcChecksum(rbuf, rCnt) == rbuf[rCnt]) { // Контрольная сумма - OK
              return(bytesRcvd);
            } else {
              return(0);   // Ошибка контрольной суммы.
            }
          }
          
          // Сброс счетчиков
          rCnt = 0;
          bytesRcvd = 0;
          
         // Задержка между ответами от ECU по ISO 14230
        }
        else { // буфер данных ответа если значение >= 4
          
          if (forMe) {
            response[bytesRcvd] = c;
          }
          
          bytesRcvd++;
          rCnt++;
        }
        break;
      }
    }
  }

  return false;
}

// Контрольная сумма — сумма всех байтов данных по модулю 0xFF.
uint8_t calcChecksum(uint8_t *data, uint8_t len) {
  uint8_t crc = 0;

  for (uint8_t i = 0; i < len; i++) {
    crc = crc + data[i];
  }
  return crc;
}
