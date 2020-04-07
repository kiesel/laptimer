#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "LiquidCrystal_I2C.h"

#include "config.h"
#include "lane.h"

LiquidCrystal_I2C lcd(PCF8574_ADDR_A21_A11_A01, 4, 5, 6, 16, 11, 12, 13, 14, POSITIVE);
LaneTimer lane1 = LaneTimer(LANE1_INPUT);
LaneTimer lane2 = LaneTimer(LANE2_INPUT);

void checkResetButtonPressed()
{
  if (digitalRead(RESET_BUTTON) == LOW)
  {
    Serial.println("Resetting values");
    lane1.reset();
    lane2.reset();

    pinMode(RESET_BUTTON, OUTPUT);
    digitalWrite(RESET_BUTTON, HIGH);
    pinMode(RESET_BUTTON, INPUT);
  }
}

ICACHE_RAM_ATTR void interruptHandlerLane1()
{
  lane1.interruptHandler();
}

ICACHE_RAM_ATTR void interruptHandlerLane2()
{
  lane2.interruptHandler();
}

void setup()
{
  WiFi.persistent(false); //disable saving wifi config into SDK flash area
  WiFi.forceSleepBegin(); //disable AP & station by calling "WiFi.mode(WIFI_OFF)" & put modem to sleep

  Serial.begin(115200);

  while (lcd.begin(16, 2, LCD_5x8DOTS, D2, D1) != 1) //colums - 20, rows - 4, pixels - 5x8, SDA - D2, SCL - D1
  {
    Serial.println(F("PCF8574 is not connected or lcd pins declaration is wrong. Only pins numbers: 4,5,6,16,11,12,13,14 are legal."));
    delay(5000);
  }

  lcd.clear();

  /* prints static text */
  lcd.setCursor(0, 0); //set 1-st colum & 2-nd row, 1-st colum & row started at zero
  lcd.print(F("CARRERA STOPPUHR"));
  lcd.setCursor(0, 1); //set 1-st colum & 3-rd row, 1-st colum & row started at zero
  lcd.print(F("von Lenn"));
  delay(5000);
  lcd.clear();

  // Setup analog read
  Serial.println("Setting up LANE1 input");

  pinMode(LANE1_INPUT, INPUT);
  attachInterrupt(LANE1_INPUT, interruptHandlerLane1, RISING);

  pinMode(LANE2_INPUT, INPUT);
  attachInterrupt(LANE2_INPUT, interruptHandlerLane2, RISING);

  pinMode(RESET_BUTTON, INPUT);
}

void loop()
{
  lcd.setCursor(0, 0);
  lcd.print(lane1.shortStats());
  lcd.setCursor(0, 1);
  lcd.print(lane2.shortStats());

  lane1.loop();
  lane2.loop();

  checkResetButtonPressed();
  delay(100);
}