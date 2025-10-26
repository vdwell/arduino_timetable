#define NUM_ROUTES 2
#define MAX_SLOTS 20

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
  byte id;
  byte number;
  int start;    // minutes from midnight
  int interval; // minutes
  const char *name;
  byte pathid;
};

Routes routes[NUM_ROUTES] = {
    {0, 10, 0, 5, "Ams-Gdx", 1},
    {1, 12, 2, 5, "Ams-Par", 2},
};

int current_second_of_hour = 150;
int myarr[2] = {0, 0};
int arr2d[NUM_ROUTES][MAX_SLOTS] = {};

Datime dt;

void Timeslots(const Routes routes[])
{
  for (int i = 0; i < NUM_ROUTES; i++)
  {
    // std::cout << routes[i].id << "\n";
    arr2d[i][0] = routes[i].start * 60;

    for (int j = 1; j < MAX_SLOTS; j++)
    {
      if (j * routes[i].interval * 60 + routes[i].start * 60 >= 3600)
      {
        arr2d[i][j] = 0;
      }
      else
      {

        arr2d[i][j] = arr2d[i][j - 1] + routes[i].interval * 60;
      };
    }
  }
}

int FindClosestSlot(int arr2d[NUM_ROUTES][MAX_SLOTS], int current_second_of_hour)
{

  for (int i = 0; i < NUM_ROUTES; i++)
  {
    for (int j = 0; j < MAX_SLOTS; j++)
    {
      if (arr2d[i][j] > current_second_of_hour)
      {
        if (myarr[1] == 0)
        {
          myarr[0] = i;
          myarr[1] = arr2d[i][j];
          break;
        }
        else if (arr2d[i][j] < myarr[1])
        {
          myarr[0] = i;
          myarr[1] = arr2d[i][j];
          break;
        }
      }
    }
  }
  return 0;
}

void Reset2DArray(int arr2d[NUM_ROUTES][MAX_SLOTS])
{
  for (int i = 0; i < NUM_ROUTES; i++)
  {
    for (int j = 0; j < MAX_SLOTS; j++)
    {
      arr2d[i][j] = 0;
    }
  }
}

void PrintZero(int val)
{
  if (val < 10)
  {
    lcd.print("0");
  }
  lcd.print("");
}

void setup()
{
  Serial.begin(9600);

  setStampZone(2); // часовой пояс
  Wire.begin();
  rtc.begin();
  Serial.print("OK: ");
  Serial.println(rtc.isOK());

  Serial.print("Reset: ");
  Serial.println(rtc.isReset());

  // был сброс питания RTC, время некорректное
  if (rtc.isReset())
  {
    rtc.setBuildTime(); // установить время компиляции прошивки
  }
  // rtc.setTime(2000, 1, 30, 12, 45, 0);

  Datime dt = rtc.getTime();
  Serial.println(dt.year);
  Serial.println(dt.month);
  Serial.println(dt.day);
  Serial.println(dt.hour);
  Serial.println(dt.minute);
  Serial.println(dt.second);

  lcd.init();      // инициализация
  lcd.backlight(); // включить подсветку
}

void loop()
{
  if (millis() - myTimer1 >= 4000)
  {
    myTimer1 = millis();

    switch (mode)
    {
    case CLOCK:
    {
      myTimer1 = millis();
      lcd.clear();
      lcd.setCursor(0, 0);
      dt = rtc.getTime();
      lcd.print(dt.year);
      lcd.print(":");
      lcd.print(dt.month);
      lcd.print(":");
      lcd.print(dt.day);
      lcd.setCursor(0, 1);
      lcd.print((dt.hour < 10) ? "0" : "");
      lcd.print(dt.hour);
      lcd.print(":");
      lcd.print((dt.minute < 10) ? "0" : "");
      lcd.print(dt.minute);
      lcd.print(":");
      lcd.print((dt.hour < 10) ? "0" : "");
      lcd.print(dt.hour);
      break;
    }
    case TT:
    {
      lcd.clear();
      // get updated data for schedule
      dt = rtc.getTime();
      current_second_of_hour = dt.minute * 60 + dt.second;
      // if () {
      myarr[0] = 0;
      myarr[1] = 0;
      Reset2DArray(arr2d);
      // }

      // get closest train time
      Timeslots(routes);
      FindClosestSlot(arr2d, current_second_of_hour);
      lcd.setCursor(0, 0);
      lcd.print(routes[myarr[0]].number);
      lcd.print(" ");
      lcd.print(routes[myarr[0]].name);
      lcd.setCursor(0, 1);
      lcd.print((dt.hour < 10) ? "0" : "");
      lcd.print(dt.hour);
      lcd.print(":");
      lcd.print((myarr[1] / 60 < 10) ? "0" : "");
      lcd.print(myarr[1] / 60);
      lcd.print(":00");
      lcd.print(" Spr: ");
      lcd.print(routes[myarr[0]].pathid);
      break;
    }
    default:
    {
      break;
    }
    }
    // switch to the next mode
    mode = Modes((int)mode + 1);
    if (mode == Modes::_len)
    {
      mode = Modes::CLOCK;
    }
  }
}