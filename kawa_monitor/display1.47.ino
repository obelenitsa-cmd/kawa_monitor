
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
Arduino_GFX *gfx = new Arduino_ST7789(bus, 40, 5, false, 172, 320, 34, 0, 34, 0);
//======================================================================
//====================================================================== если ESP32-S3 LCD 1.47 No Touch ==========================
//#define GFX_CS      42
//#define GFX_RST     39
//#define GFX_BL      48       // if DF_GFX_BL - default backlight pin

//Arduino_DataBus *bus = new Arduino_ESP32SPI( 41, 42, 40, 45, -1, FSPI );
//Arduino_GFX *gfx = new Arduino_ST7789( bus, 39, 1, true, 172, 320, 34, 0, 34, 0); 
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
int tempdata;
float fvolt, fvoltdata, FW_bar, new_FW_bar, RW_bar, new_RW_bar;
bool Warning_ico = false;

void SetupDisplay()
{
#ifdef DEV_DEVICE_INIT
  DEV_DEVICE_INIT();
#endif
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);
  // ******************Init Display**************
  gfx->begin(); 
  gfx->fillScreen(BLACK);

  gfx->setFont(); 
  gfx->setTextColor(WHITE, BLACK);
  gfx->setTextSize(3 /* x scale */, 4 /* y scale */, 2 /* pixel_margin */); // for TPMS
  gfx->setCursor(12, 71); // for TPMS
  gfx->print("bar"); // for TPMS

}

void DataDisplay(bool disconn) // если нет или потеряно подключение к ECU
{
     gfx->setFont(); 
     gfx->setTextColor(WHITE, BLACK);
     gfx->setTextSize(4 /* x scale */, 5 /* y scale */, 2 /* pixel_margin */);
     //==============No Volt
     gfx->setCursor(224, 11);
     gfx->print("    ");
     //==============No Temp
     gfx->setCursor(248, 132);
     gfx->print("    ");
     //==============No Gear
     gfx->setFont(&DotDigital12); 
     gfx->setTextSize(4 /* x scale */, 4 /* y scale */, 4 /* pixel_margin */); 
     gfx->setCursor(103, 138);
     gfx->print("-");

     delay(1); 
}

void VoltDisplay(float volt) // Volt (XX / 12.75)  возможно (A - 128) * 0.5 или (HexA * 20) / 256
 {
   fvolt = (volt * 20) / 256;
   char buffer[5];
   snprintf(buffer, sizeof(buffer), "%4.1f", fvolt);  // один знак после запятой
   
   if (fvoltdata != fvolt){  // обновление данных только если данные изменились
     fvoltdata = fvolt;
     gfx->setFont(); 
     gfx->setTextColor(WHITE, BLACK);
     gfx->setTextSize(4 /* x scale */, 5 /* y scale */, 2 /* pixel_margin */);
     gfx->setCursor(224, 13);
     gfx->print(buffer);

     gfx->setTextSize(3 /* x scale */, 4 /* y scale */, 2 /* pixel_margin */);
     gfx->setCursor(301, 46);
     gfx->print("v");

     delay(1);
    }
  }

//Идеальная температура ДВС: 82°C-104°C. Перегрев 104°C-115°C. Критическая: 115°C–120°C (перегрев)
void TempDisplay(int temp) // Температура по Цельсию =  (HexA * 160 / 255) - 30 
{
  temp = (temp * 160 / 255) - 30;

  char buffer[5];
  snprintf(buffer, sizeof(buffer), "%3d", temp); // % — начало спецификатора формата, d — вывод целого числа со знаком (int).
 
  if (tempdata != temp){  // обновление данных только если данные изменились
   tempdata = temp;
   
   gfx->setFont(); 
   gfx->setTextSize(4 /* x scale */, 5 /* y scale */, 2 /* pixel_margin */);
   gfx->setCursor(248, 132);
      if (temp >= 115){ // если перегрев >= 115°C
        gfx->setTextColor(YELLOW, BLACK);
      } else {
         gfx->setTextColor(WHITE, BLACK);
        }
    gfx->print(buffer);

    gfx->setTextSize(3 /* x scale */, 4 /* y scale */, 2 /* pixel_margin */);
    //gfx->setCursor(301, 136); 
    gfx->setCursor(301, 95); 
    gfx->setTextColor(WHITE, BLACK);
    gfx->print("c");
    gfx->drawCircle(295, 100, 1, WHITE); // символ градус

    delay(1);
  }

}

void GearDisplay(uint8_t gear) 
{
  if (geardata != gear){  // обновление данных только если данные изменились
   geardata = gear;
   gfx->setCursor(103, 138);
   gfx->setFont(&DotDigital12); //шрифт
   gfx->setTextSize(4 /* x scale */, 4 /* y scale */, 4 /* pixel_margin */); 
   gfx->setTextColor(WHITE, BLACK);
      if (gear==78) {
        gfx->setTextColor(GREEN, BLACK);
        gfx->print("N");
      }
      else if (gear>0 && gear<10){
        gfx->print(gear);
      } 
      else if (gear==45) {
        gfx->print("-");
      }
      else {
        gfx->setTextColor(YELLOW, BLACK);
        gfx->print("F");
      }
   delay(1);
  }
}

void FW_press_dat(float FW_pressure) // Давление Front: 225 kPa (2.25 kgf/cm², 32 psi)  допустимое отклонение 0,1–0,2 kgf/cm²
{
  FW_bar = FW_pressure * .03144;
  char buffer[5];
  snprintf(buffer, sizeof(buffer), "%3.1f", FW_bar);  // один знак после запятой
     
  if (new_FW_bar != FW_bar){  // обновление данных только если данные изменились
    new_FW_bar = FW_bar;
    gfx->setFont(); 
    gfx->setTextColor(WHITE, BLACK);
    gfx->setTextSize(4 /* x scale */, 5 /* y scale */, 2 /* pixel_margin */);
    gfx->setCursor(12, 13); 
    if (FW_bar <= 1.95 || FW_bar >= 2.55){ // если низкое или высокое давление
      gfx->setTextColor(YELLOW, BLACK);
    } else {
         gfx->setTextColor(WHITE, BLACK);
        }
    gfx->print(buffer); 

    delay(1);
  }
}

void RW_press_dat(float FW_pressure) // Давление Rear: 250 kPa (2.50 kgf/cm², 36 psi) допустимое отклонение 0,1–0,2 kgf/cm²
{
  RW_bar = RW_pressure * .03144;
  char buffer[5];
  snprintf(buffer, sizeof(buffer), "%3.1f", RW_bar);  // один знак после запятой
     
  if (new_RW_bar != RW_bar){  // обновление данных только если данные изменились
    new_RW_bar = RW_bar;
    gfx->setFont(); 
    gfx->setTextColor(WHITE, BLACK);
    gfx->setTextSize(4 /* x scale */, 5 /* y scale */, 2 /* pixel_margin */);
    gfx->setCursor(12, 132); 
    if (FW_bar <= 2.2 || FW_bar >= 2.8){ // если низкое или высокое давление
      gfx->setTextColor(YELLOW, BLACK);
    } else {
         gfx->setTextColor(WHITE, BLACK);
        }
    gfx->print(buffer); 

    delay(1);
  }
}
