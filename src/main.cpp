#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "LiquidCrystal_I2C.h"

#define LANE1_INPUT D7
#define RESET_BUTTON 16
#define DEBOUNCE_TIME 250
LiquidCrystal_I2C lcd(PCF8574_ADDR_A21_A11_A01, 4, 5, 6, 16, 11, 12, 13, 14, POSITIVE);

int laps = -1;
unsigned int roundStartedAt = 0;
unsigned int bestLapTime = 0;
volatile bool haveInterrupt = false;
volatile unsigned int latestMillis = 0;
volatile unsigned int debounceMillis = 0;

void handleCircuitClose();
void handleCircuitOpen();

ICACHE_RAM_ATTR void handleCircuitClose()
{
  if (haveInterrupt)
  {
    Serial.println("Previous interrupt not consumed.");
    debounceMillis = millis();
    return;
  }

  int deltaT = millis() - debounceMillis;
  if (deltaT < DEBOUNCE_TIME)
  {
    Serial.printf("Debouncer ignored interrupt w/ [%dms].\n", deltaT);
    debounceMillis = millis();
    return;
  }

  Serial.printf("Interrupt deltaT [%dms]\n", deltaT);
  latestMillis = millis();
  debounceMillis = latestMillis;
  haveInterrupt = true;
}

void checkResetButtonPressed()
{
  if (digitalRead(RESET_BUTTON) == LOW)
  {
    Serial.println("Resetting values");
    laps = -1;
    bestLapTime = 0;
    roundStartedAt = 0;
    debounceMillis = 0;
  }
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
  attachInterrupt(LANE1_INPUT, handleCircuitClose, RISING);

  pinMode(RESET_BUTTON, INPUT);
}

void loop()
{
  lcd.setCursor(0, 0);
  lcd.printf("L%02d T%0.1fs B%0.1fs  ",
             laps > 0 ? laps : 0,
             (roundStartedAt > 0 ? (millis() - (int)roundStartedAt) / 1000.0 : 0.0),
             bestLapTime > 0 ? bestLapTime / 1000.0 : 0.0);

  if (haveInterrupt)
  {
    unsigned int triggerMillis = latestMillis;
    latestMillis = 0;
    haveInterrupt = false;

    // Complete previous lap
    if (laps > -1)
    {
      unsigned int lapTime = (triggerMillis - roundStartedAt);
      if (bestLapTime == 0 || lapTime < bestLapTime)
      {

        bestLapTime = lapTime;
      }
    }

    laps++;
    roundStartedAt = triggerMillis;
  }

  checkResetButtonPressed();
  delay(50);
}