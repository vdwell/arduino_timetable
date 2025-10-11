#include <Arduino.h>
#include <Wire.h>

#include <LiquidCrystal_I2C.h>
#include <GyverDS3231Min.h>

GyverDS3231Min rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2); // адрес, столбцов, строк

uint32_t myTimer1, myTimer2, myTimer3;
uint32_t tmr;
bool flag;
enum Modes
{
  CLOCK = 0,
  TT = 1,
  _len = 2,
};
Modes mode = Modes::CLOCK;

struct Routes
{
  int id;
  int start;    // minutes from midnight
  int interval; // minutes
  const char *name;
};

Routes routes[] = {
    {10, 0, 5, "Ams-Gdx"},
    {12, 5, 5, "Ams-Par"},
};

void setup()
{
  Serial.begin(9600);

  setStampZone(2); // часовой пояс
  Wire.begin();
  rtc.begin();

  Serial.print("OK: ");
  Serial.println(rtc.isOK()); // RTC работает

  Serial.print("Reset: ");
  Serial.println(rtc.isReset()); // сброс питания (батарейки)

  // был сброс питания RTC, время некорректное
  if (rtc.isReset())
  {
    rtc.setBuildTime(); // установить время компиляции прошивки
                        // rtc.setTime(2025, 1, 30, 12, 45, 0); // установить время вручную
  }

  // вывести в Datime
  Datime dt = rtc.getTime();
  Serial.println(dt.year);
  Serial.println(dt.month);
  Serial.println(dt.day);
  Serial.println(dt.hour);
  Serial.println(dt.minute);
  Serial.println(dt.second);

  lcd.init();      // инициализация
  lcd.backlight(); // включить подсветку

  lcd.setCursor(0, 0); // столбец 1 строка 0
  lcd.print("Hello, world!");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0); // столбец 4 строка 1~
  lcd.print("Amsterdam 11:00");
  lcd.setCursor(0, 1); // столбец 4 строка 1
  lcd.print("Guadelx 11:08");
}

void loop()
{
  if (millis() - myTimer1 >= 2000)
  {
    myTimer1 = millis();

    switch (mode)
    {
    case CLOCK:
    {
      myTimer1 = millis();
      lcd.clear();
      lcd.setCursor(0, 0);
      // lcd.print("Time: ");
      lcd.print(rtc.getTime().toString());
      break;
    }
    case TT:
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      // get updated data for schedule

      lcd.print("Amsterdam 11:00");
      lcd.setCursor(0, 1);
      lcd.print("Guadelx 11:08");
      break;
    }
    default:
      break;
    }

    mode = Modes((int)mode + 1);
    if (mode == Modes::_len)
    {
      mode = Modes::CLOCK;
    }
  }
}