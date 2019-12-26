#pragma once

//コンストラクタでピンを指定//
//driveの引数は-255～255    //

class motor {
  public:
    motor(int pin_A, int pin_B, int pin_PWM);
    void drive(double motorPower);  //val:-255~255
    void drive(double motorPower, bool DIR);  //val:0~255
    void show();
  private:
    int power;
    int pinA;
    int pinB;
    
    int pinPWM;
};

motor::motor(int pin_A, int pin_B, int pin_PWM) {
  pinA   = pin_A;
  pinB   = pin_B;
  pinPWM = pin_PWM;
  pinMode(pinA, OUTPUT);
  pinMode(pinB, OUTPUT);
}

void motor::drive(double motorPower) {
//  motorPower = constrain(motorPower, -245, 245);
  motorPower = constrain(motorPower, -220, 220);
  
  if (motorPower == 0) {
    digitalWrite(pinA, HIGH);
    digitalWrite(pinB, HIGH);
    analogWrite(pinPWM, 0);
  } else if (motorPower > 0) {
    digitalWrite(pinA, HIGH);
    digitalWrite(pinB, LOW);
    analogWrite(pinPWM, abs(motorPower));
  } else if (motorPower < -0) {
    digitalWrite(pinA, LOW);
    digitalWrite(pinB, HIGH);
    analogWrite(pinPWM, abs(motorPower));
  }
  power = motorPower;
}
void motor::drive(double motorPower, bool DIR) {
  if(DIR)  motorPower = -motorPower;
  drive(motorPower);
}

void motor::show() {
  Serial.print(power);
  Serial.print("\t");
}
