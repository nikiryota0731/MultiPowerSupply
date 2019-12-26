#include <LiquidCrystal_I2C.h>
#include <Rotary.h>
#include "sw_controller.h"
#include "motor.h"
#include "MovingAverage.h"

#define CW false
#define CCW true

const uint8_t SWPIN[]    = {5, 4, 6, 9}; //R, L, mode, power
const uint8_t LEDPIN[]   = {8, 7}; //R, B
const uint8_t ENCPIN[]   = {2, 3}; //A, B
const uint8_t MDPIN[]    = {12, 10, 11}; //in1. in1, pwm
const uint8_t LEVELPIN[] = {A1, A2, A0}; //12V, 6V, 3V

const char DIRNAME[2][5] = {"CW", "CCW"};
const char POWERNAME[2][5] = {"ON", "OFF"};

const int volumeScale = 5;

const double levelScale[] = {2.73318872, 1.8221258134, 1.004784689}; //12V, 6V, 3V

LiquidCrystal_I2C lcd(0x27, 20, 4);
Rotary enc = Rotary(ENCPIN[0], ENCPIN[1]);
swController swR(TOGGLE), swL(TOGGLE), swMode, swPower;
motor motorDriver(MDPIN[0], MDPIN[1], MDPIN[2]);
MovingAverage levelAverage[3] = {
  MovingAverage(50),
  MovingAverage(50),
  MovingAverage(50)
};

bool DIR = false;
bool power = false;
double volume = 0;
int lcdPage = 0;
long oldTime = 0;

int level12V, level6V, level3V;
double level[3];

void setup() {
  Serial.begin(115200);
  attachInterrupt(0, ENCupdate, CHANGE);
  attachInterrupt(1, ENCupdate, CHANGE);
  lcd.init();
  lcd.backlight();
  for (int i = 0; i < sizeof SWPIN / sizeof SWPIN[0]; i++)
    pinMode(SWPIN[i], INPUT_PULLUP);
  for (int i = 0; i < sizeof ENCPIN / sizeof ENCPIN[0]; i++)
    pinMode(ENCPIN[i], INPUT_PULLUP);
  for (int i = 0; i < sizeof LEDPIN / sizeof LEDPIN[0]; i++)
    pinMode(LEDPIN[i], OUTPUT);
  for (int i = 0; i < sizeof LEVELPIN / sizeof LEVELPIN[0]; i++)
    pinMode(LEVELPIN[i], INPUT);

}

void loop() {
  showLCD();
  allUpdate();
  // show();
}

void showLCD() {
  switch (lcdPage) {
    case 0: //モーター出力確認ページ
      lcd.setCursor(0, 0);
      lcd.print("power : ");  lcd.print((int)volume);  lcd.print("%");
      lcd.print("  ");
      lcd.setCursor(0, 1);
      lcd.print("DIR : "); lcd.print(DIRNAME[DIR]);
      lcd.print(" "); lcd.print("/ ");
      lcd.print(POWERNAME[power]); lcd.print(" ");

      if (power) {
        switch (millis() - oldTime < 300) {
          case 0:
            if (swR.getChange())
              oldTime = millis();
            break;
          case 1:
            if (swR.getChange()) {
              lcdPage = 1;
              lcd.clear();
            }
            break;
        }
      }
      break;
    case 1: //Li-Po電圧確認ページ

      lcd.setCursor(0, 0);
      lcd.print(levelAverage[0].GetValue() - levelAverage[1].GetValue()); lcd.print(",");
      lcd.print(levelAverage[1].GetValue() - levelAverage[2].GetValue()); lcd.print(",");
      lcd.print(levelAverage[2].GetValue()); lcd.print(",");
      lcd.setCursor(0, 1);
      lcd.print(levelAverage[0].GetValue());

      switch (millis() - oldTime < 500) {
        case 0:
          if (swR.getChange())
            oldTime = millis();
          break;
        case 1:
          if (swR.getChange()) {
            lcdPage = 0;
            lcd.clear();
          }
          break;
      }
      if (!power) {
        lcdPage = 0;
        lcd.clear();
      }
      break;
  };

}

void allUpdate() {

  swR.update(digitalRead(SWPIN[0]));
  swL.update(digitalRead(SWPIN[1]));
  swMode.update(digitalRead(SWPIN[2]));
  swPower.update(digitalRead(SWPIN[3]));

  if (swR.getChange()) DIR = CW;
  if (swL.getChange()) DIR = CCW;

  volume = enc.getCount() * volumeScale;
  power = swMode.getData();
  if (power) volume = 0;
  motorDriver.drive(volume * 250 / 100, !DIR);
  volume = enc.getCount() * volumeScale;
  digitalWrite(LEDPIN[0], !power);
  digitalWrite(LEDPIN[1], power);

  for (int i = 0; i < 3; i++) {
    level[i] = analogRead(LEVELPIN[i]) * 5.0 / 1023.0 * levelScale[i];
    levelAverage[i].Update(level[i]);
  }
}

void ENCupdate() {
  enc.update(-100 / volumeScale, 100 / volumeScale);
}

void show() {
  for (int i = 0; i < 3; i++) {
    Serial.print(analogRead(LEVELPIN[i]) * 5.0 / 1023.0 );
    Serial.print("\t");
  }
  Serial.print("|");
  Serial.print("\t");

  for (int i = 0; i < 3; i++) {
    Serial.print(levelAverage[i].GetValue());
    Serial.print("\t");
  }
  Serial.print("|");
  Serial.print("\t");

  Serial.print(levelAverage[0].GetValue() - levelAverage[1].GetValue());
  Serial.print("\t");
  Serial.print(levelAverage[1].GetValue() - levelAverage[2].GetValue());
  Serial.print("\t");
  Serial.print(levelAverage[2].GetValue());
  Serial.print("\t");
  Serial.println();
}
