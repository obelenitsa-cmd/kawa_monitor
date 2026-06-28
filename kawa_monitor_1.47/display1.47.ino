#include <Arduino_GFX_Library.h>
#include "DotDigital12.h"


//====================================================================== если ESP32-S3 LCD 1.47 Touch  ==========================
#define GFX_BL 46
#define LCD_RST 40

#define Touch_I2C_SDA 42
#define Touch_I2C_SCL 41
#define Touch_RST     47
#define Touch_INT     48

Arduino_DataBus *bus = new Arduino_ESP32SPI(45, 21, 38, 39);
Arduino_GFX *gfx = new Arduino_ST7789(bus, LCD_RST, 5, false, 172, 320, 34, 0, 34, 0);

#define PIN_PHOTO 7

//======================================================================
//====================================================================== если ESP32-S3 LCD 1.47 No Touch ==========================
//#define GFX_CS      42
//#define GFX_RST     39
//#define GFX_BL      48       // if DF_GFX_BL - default backlight pin

//Arduino_DataBus *bus = new Arduino_ESP32SPI( 41, 42, 40, 45, -1, FSPI );
//Arduino_GFX *gfx = new Arduino_ST7789( bus, 39, 1, true, 172, 320, 34, 0, 34, 0); 

//#define PIN_PHOTO 6
//======================================================================
//============ RGB-565 (16-bit)
#define BLACK 0x0000
#define WHITE 0xFFFF
#define GREEN 0xAFE6
//#define GREEN2 0x07E0
//#define LIME 0x67E0
#define YELLOW	0xFFE0
//#define CADMIUM_YELLOW	0xFFA0

 
uint8_t geardata;
float FW_bar, new_FW_bar, RW_bar, new_RW_bar;
bool OtaOn = false;
bool OnECU = true;

//Динамическая подсвека от фотоэлемента для N-P-N транзистора GPIO подтянут к "+" эмиттер к GND
// Параметры фильтрации
float filtered_light = 4095;
// При частоте опроса 5 Гц (каждые 200 мс - см. LoopPHOTO()) коэффициент 0.06 
// дает плавную адаптацию яркости за 2.5 секунды.
const float filter_k = 0.4; // Коэффициент сглаживания (0.01 - медленно, 1 - быстро)
int new_pwm_brightness = 4095;
int minpwm = 32; //минимальное значение ШИМ подсветки
int raw_light, pwm_brightness;

void SetupDisplay()
{
#ifdef DEV_DEVICE_INIT
  DEV_DEVICE_INIT();
#endif

 //==================== Подсвека ==================
  pinMode(GFX_BL, OUTPUT);
  //digitalWrite(GFX_BL, HIGH); // Дискретная подсветка
  //analogWrite(GFX_BL, 255); // ШИМ подсветки 0-255
  // Поднимаем частоту ШИМ на пине до 15 кГц. (убераем любые наводки по питанию на фототранзистор.)
  analogWriteFrequency(GFX_BL, 15000); // ШИМ подсветка, частота

  // Вход фототранзистора (коллектор) к 3.3В через дополнительное сопротивление 10кОм (с внутренней плдтяжкой -INPUT_PULLUP) или 8кОм (без внутренней плдтяжки)
  //pinMode(PIN_PHOTO, INPUT_PULLUP); 
  pinMode(PIN_PHOTO, INPUT); 

  filtered_light = analogRead(PIN_PHOTO); // Начальная инициализация фильтра средним значением

  // ******************Init Display**************
  gfx->begin(); 
  gfx->fillScreen(BLACK);

  gfx->setFont(); 
  gfx->setTextColor(WHITE, BLACK);
  gfx->setTextSize(3, 4, 2 ); //TPMS
  gfx->setCursor(12, 71); // TPMS
  gfx->print("bar"); // TPMS
  gfx->setCursor(301, 46); // Напряжение
  gfx->print("v"); // Напряжение
  gfx->setCursor(301, 95); // Температура
  gfx->print("c"); // Температура
  gfx->drawCircle(295, 100, 1, WHITE); // Температура, символ градус
}

// ================================================таймер индикации низкого заряда батареи
//unsigned long previousMillis = 0;
//unsigned long interval = 500; 
//bool state = false;             

void LoopDisplay() {

 float voltdata, newvolt;
 int tempdata, newtemp;

 char buffer[4][8]; // Создаем 4 буфера, каждый размером по 5 байт
 char clean_buffer[4][8] = {0}; // Инициализация нулями при объявлении

  // ================================================если нет или потеряно подключение к ECU
  if (OnECU != ECUconnected){  // обновление данных только если данные изменились
    if (!ECUconnected){ 
      gfx->setFont(); 
      gfx->setTextColor(WHITE, BLACK);
      gfx->setTextSize(4, 5, 2 );
      //==============No Volt
      gfx->setCursor(224, 11);
      gfx->print("    ");
      //==============No Temp
      gfx->setCursor(248, 132);
      gfx->print("    ");
      delay(1);
    }
    OnECU = ECUconnected;
  }

 //================================================OTA WiFi
 if (isOtaStarted != OtaOn) {
      gfx->setCursor(103, 138);
      gfx->setFont(&DotDigital12); //шрифт
      gfx->setTextSize(4 , 4 , 4 ); 
      gfx->setTextColor(GREEN, BLACK);
    if (isOtaStarted) { 
      gfx->print("&");
      delay(1);
    }
    if (!isOtaStarted) { 
      gfx->setTextColor(WHITE, BLACK);
      gfx->print("-");
      delay(1);
    }
    OtaOn = isOtaStarted;
  }
  else if (!isOtaStarted){
    //================================================Передача
    if (geardata != dgear){  // обновление данных только если данные изменились
     geardata = dgear;
     gfx->setCursor(103, 138);
     gfx->setFont(&DotDigital12); //шрифт
     gfx->setTextSize(4 , 4 , 4 ); 
     gfx->setTextColor(WHITE, BLACK);
     if (dgear==78) {
       gfx->setTextColor(GREEN, BLACK);
       gfx->print("N");
      }
     else if (dgear>0 && dgear<10){
       gfx->print(dgear);
      } 
     else if (dgear==45) {
       gfx->print("-");
      }
     else {
       gfx->setTextColor(YELLOW, BLACK);
       gfx->print("F");
      }
     delay(1);
    }
  }
  
  // ================================================Volt (XX / 12.75)  возможно (A - 128) * 0.5 или (HexA * 20) / 256
  voltdata = (datavolt * 20) / 256;
  snprintf(buffer[0], sizeof(buffer[0]), "%4.1f", voltdata);  // один знак после запятой
  if (newvolt != datavolt){  // обновление данных только если данные изменились
   newvolt = voltdata; 
   gfx->setFont(); 
   gfx->setTextColor(WHITE, BLACK);
   gfx->setTextSize(4 , 5 , 2 );
   gfx->setCursor(224, 13);
   gfx->print(buffer[0]);
   delay(1);
  }
  
  //================================================Идеальная температура ДВС: 82°C-104°C. Перегрев 104°C-115°C. Критическая: 115°C–120°C (перегрев)
  tempdata = (datatemp * 160 / 255) - 30;
  snprintf(buffer[1], sizeof(buffer[1]), "%3d", tempdata); // % — начало спецификатора формата, d — вывод целого числа со знаком (int).
  if (newtemp != datatemp){  // обновление данных только если данные изменились
   newtemp = datatemp;
   gfx->setFont(); 
   gfx->setTextSize(4 , 5 , 2 );
   gfx->setCursor(248, 132);
    if (tempdata >= 115){ // если перегрев >= 115°C
      gfx->setTextColor(YELLOW, BLACK);
    } 
    else { 
      gfx->setTextColor(WHITE, BLACK); 
    }
    gfx->print(buffer[1]);
    delay(1);
  }

  /*
  // ============================================================================1 байт: Давление Front: 225 kPa (2.25 kgf/cm², 32 psi)  допустимое отклонение 0,1–0,2 kgf/cm², разогрев на 0.2 - 0.4
  FW_bar = FW_pressure * .03144;
  //char buffer[5];
  snprintf(buffer[2], sizeof(buffer[2]), "%3.1f", FW_bar);  // один знак после запятой
     
  if (new_FW_bar != FW_bar){  // обновление данных только если данные изменились
    new_FW_bar = FW_bar;
    gfx->setFont(); 
    gfx->setTextColor(WHITE, BLACK);
    gfx->setTextSize(4 , 5 , 2 );
    gfx->setCursor(12, 13); 
    if (FW_bar <= 1.85 || FW_bar >= 2.65){ // если низкое или высокое давление
      gfx->setTextColor(YELLOW, BLACK);
    } else {
         gfx->setTextColor(WHITE, BLACK);
        }
    gfx->print(buffer[2]); 

    delay(1);
  }
  */

    // ============================================================================2 байт: Давление Front: 225 kPa (2.25 kgf/cm², 32 psi)  допустимое отклонение 0,1–0,2 kgf/cm², разогрев на 0.2 - 0.4
    if (FW_pressure == 146){
      FW_bar = 0;
    }
    else if (FW_pressure > 146){
      FW_bar = ((float)FW_pressure - 146.0f) / 145.038f;
    }
  snprintf(buffer[2], sizeof(buffer[2]), "%3.1f", FW_bar);  // один знак после запятой
     
  if (new_FW_bar != FW_bar){  // обновление данных только если данные изменились
    new_FW_bar = FW_bar;
    gfx->setFont(); 
    gfx->setTextColor(WHITE, BLACK);
    gfx->setTextSize(4 , 5 , 2 );
    gfx->setCursor(12, 13); 
    if (FW_bar <= 1.85 || FW_bar >= 2.65){ // если низкое или высокое давление
      gfx->setTextColor(YELLOW, BLACK);
    } else {
         gfx->setTextColor(WHITE, BLACK);
        }
    gfx->print(buffer[2]); 

    delay(1);
  }

  /*
  // =========================================================================1 байт: Давление Rear: 250 kPa (2.50 kgf/cm², 36 psi) допустимое отклонение 0,1–0,2 kgf/cm², разогрев на 0.2 - 0.4
    RW_bar = RW_pressure * .03144; //1 байт выходного 8-битного значения
  //char buffer[5];
  snprintf(buffer[3], sizeof(buffer[3]), "%3.1f", RW_bar);  // один знак после запятой
     
  if (new_RW_bar != RW_bar){  // обновление данных только если данные изменились
    new_RW_bar = RW_bar;
    gfx->setFont(); 
    gfx->setTextColor(WHITE, BLACK);
    gfx->setTextSize(4 , 5 , 2 );
    gfx->setCursor(12, 132); 
    if (FW_bar <= 2.1 || FW_bar >= 2.9){ // если низкое или высокое давление
      gfx->setTextColor(YELLOW, BLACK);
    } else {
         gfx->setTextColor(WHITE, BLACK);
        }
    gfx->print(buffer[3]); 

    // ======================================================================================================= индикация низкой батареи
    //unsigned long currentMillis = millis(); 

  //if (currentMillis - previousMillis >= interval) {
    //previousMillis = currentMillis;

    //state = !state; // Переключаем состояние

    //if (state) {
      //Serial.println("Batt");
      //interval = 500; // Задаем строго 500 мс для режима ON
    //} else {
      //Serial.println("BAR");
      //interval = 500; // Время, сколько плата будет "отдыхать" в OFF перед новым циклом
    //}
  //}
  // ======================================================================================================== индикация низкой батареи
    delay(1);
  }
  */

  // =========================================================================2 байта: Давление Rear: 250 kPa (2.50 kgf/cm², 36 psi) допустимое отклонение 0,1–0,2 kgf/cm², разогрев на 0.2 - 0.4
    if (RW_pressure == 146){
      RW_bar = 0;
    }
    else if (RW_pressure > 146){
      RW_bar = ((float)RW_pressure - 146.0f) / 145.038f;
    }

  snprintf(buffer[3], sizeof(buffer[3]), "%3.1f", RW_bar);  // один знак после запятой
     
  if (new_RW_bar != RW_bar){  // обновление данных только если данные изменились
    new_RW_bar = RW_bar;
    gfx->setFont(); 
    gfx->setTextColor(WHITE, BLACK);
    gfx->setTextSize(4 , 5 , 2 );
    gfx->setCursor(12, 132); 
    if (FW_bar <= 2.1 || FW_bar >= 2.9){ // если низкое или высокое давление
      gfx->setTextColor(YELLOW, BLACK);
    } else {
         gfx->setTextColor(WHITE, BLACK);
        }
    gfx->print(buffer[3]); 

    delay(1);
  }

  //=================================================================== Управление яркостью, фототранзистор
  raw_light = analogRead(PIN_PHOTO); // Читаем текущее значение освещенности (0...4095 для ESP32-S3) (180-190 средняя освещенность.)
  filtered_light = (raw_light * filter_k) + (filtered_light * (1.0 - filter_k)); // Сглаживаем пульсации и шумы датчика
  pwm_brightness = map((int)filtered_light, 4095, 40, minpwm, 255); // Масштабируем значение под ШИМ (0...255); ТЕМНОТА (4095) ➔ минимальный ШИМ подсветки (64); СОЛНЦЕ  (40)    ➔ максимальный ШИМ подсветки (255)
  pwm_brightness = constrain(pwm_brightness, minpwm, 255); // все, что меньше 102, станет 102. Все, что больше 255, станет 255
  analogWrite(GFX_BL, pwm_brightness); // Задаем яркость дисплея 
  //Serial.println(raw_light);

  /*
  if (new_pwm_brightness > raw_light) {  // обновление данных только если значение уменьшилось (стало  светлее)
  //if (new_pwm_brightness < raw_light) {  // обновление данных только если значение увеличились  (стало темнее)
  new_pwm_brightness = raw_light;

   gfx->setFont(); 
   gfx->setTextSize(3 , 4 , 2 );
   gfx->setCursor(224, 70);
   gfx->setTextColor(WHITE, BLACK);
   gfx->print(raw_light);
   gfx->print("  ");
  }
  */

  delay(200);
}
