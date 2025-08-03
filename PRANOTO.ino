#include <PS4Controller.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include "settingps.h"

#define MLIFT_PWM 14
#define MLIFT_CH 9
#define MLIFT_D1 12
#define MLIFT_D2 13

#define MOTOR_FL_IN 4
#define MOTOR_FL_IN2 16
#define MOTOR_FR_IN 19
#define MOTOR_FR_IN2 23
#define MOTOR_BL_IN 17
#define MOTOR_BL_IN2 18
#define MOTOR_BR_IN 26
#define MOTOR_BR_IN2 27

#define MOTOR_FL_PWM_CH 5
#define MOTOR_FR_PWM_CH 6
#define MOTOR_BL_PWM_CH 7
#define MOTOR_BR_PWM_CH 8

#define pinGrip1 15
#define pinGrip2 5
#define pinRoll 25
#define defaultSpeed 50

Servo CapitKA, CapitKI;

int pos0, pos1, pos2, pos3;
int speedFL, speedFR, speedBL, speedBR;
int g1, g2;
int g1Open, g2Open;
int correctFL = 0, correctFR = 0, correctBL = 0, correctBR = 0;
int angkatStep = 0, tutupStep = 0, startStep = 0;

int LanalogAtas, LanalogBawah, LanalogKanan, LanalogKiri;
int LanalogSerongAtasKanan, LanalogSerongAtasKiri;
int LanalogSerongBawahKanan, LanalogSerongBawahKiri;
int RanalogKanan, RanalogKiri;
int speedLift;

bool openSetting = false;
bool prevStartState = false;
unsigned long lastStartToggle = 0;
const unsigned long startDebounce = 250;

unsigned long lastAngkatPress = 0;
unsigned long lastTutupPress = 0;
const unsigned long buttonDelay = 250;
bool gripOpened = false;

void setup() {
  Serial.begin(115200);
  PS4.begin("A8:81:F8:55:5A:9B");
  Serial.println("Motor Setup");
  setupMotors();

  pinMode(2, OUTPUT);

  CapitKI.attach(pinGrip1);
  CapitKA.attach(pinGrip2);

  Serial.println("Motor Lifter Setup");
  setupLift();
  initSettingan();

  while (!PS4.isConnected()) {
    digitalWrite(2, !digitalRead(2));
    delay(100);
  }
  digitalWrite(2, HIGH);
}

void loop() {
  if (PS4.isConnected()) {
    int lx = PS4.data.analog.stick.lx;
    int ly = PS4.data.analog.stick.ly;
    int rx = PS4.data.analog.stick.rx;

    int threshold = 100;

    LanalogAtas = LanalogBawah = LanalogKanan = LanalogKiri = 0;
    LanalogSerongAtasKanan = LanalogSerongAtasKiri = 0;
    LanalogSerongBawahKanan = LanalogSerongBawahKiri = 0;
    RanalogKanan = RanalogKiri = 0;

    if (ly < -threshold && abs(lx) < threshold) {
      LanalogAtas = 1;
    } else if (ly > threshold && abs(lx) < threshold) {
      LanalogBawah = 1;
    } else if (lx > threshold && abs(ly) < threshold) {
      LanalogKanan = 1;
    } else if (lx < -threshold && abs(ly) < threshold) {
      LanalogKiri = 1;
    } else if (ly < -threshold && lx > threshold) {
      LanalogSerongAtasKanan = 1;
    } else if (ly < -threshold && lx < -threshold) {
      LanalogSerongAtasKiri = 1;
    } else if (ly > threshold && lx > threshold) {
      LanalogSerongBawahKanan = 1;
    } else if (ly > threshold && lx < -threshold) {
      LanalogSerongBawahKiri = 1;
    }

    if (rx > threshold) {
      RanalogKanan = 1;
    } else if (rx < -threshold) {
      RanalogKiri = 1;
    }
    logic();
  } else {
    ESP.restart();
  }
}

void setupMotors() {
  pinMode(MOTOR_FL_IN, OUTPUT);
  pinMode(MOTOR_FR_IN, OUTPUT);
  pinMode(MOTOR_BL_IN, OUTPUT);
  pinMode(MOTOR_BR_IN, OUTPUT);

  pinMode(MOTOR_FL_IN2, OUTPUT);
  pinMode(MOTOR_FR_IN2, OUTPUT);
  pinMode(MOTOR_BL_IN2, OUTPUT);
  pinMode(MOTOR_BR_IN2, OUTPUT);

  ledcAttachPin(MOTOR_FL_IN, MOTOR_FL_PWM_CH);
  ledcAttachPin(MOTOR_FR_IN, MOTOR_FR_PWM_CH);
  ledcAttachPin(MOTOR_BL_IN, MOTOR_BL_PWM_CH);
  ledcAttachPin(MOTOR_BR_IN, MOTOR_BR_PWM_CH);

  ledcSetup(MOTOR_FL_PWM_CH, 490, 8);
  ledcSetup(MOTOR_FR_PWM_CH, 490, 8);
  ledcSetup(MOTOR_BL_PWM_CH, 490, 8);
  ledcSetup(MOTOR_BR_PWM_CH, 490, 8);
}

void motor4(int sFL, int sFR, int sBL, int sBR) {
  sFL = constrain(sFL - correctFL, -255, 255);
  sFR = constrain(sFR - correctFR, -255, 255);
  sBL = constrain(sBL - correctBL, -255, 255);
  sBR = constrain(sBR - correctBR, -255, 255);

  if (sFL > 0) {
    digitalWrite(MOTOR_FL_IN2, HIGH);
    ledcWrite(MOTOR_FL_PWM_CH, 255 - sFL);
  } else if (sFL < 0) {
    digitalWrite(MOTOR_FL_IN2, LOW);
    ledcWrite(MOTOR_FL_PWM_CH, -sFL);
  } else {
    digitalWrite(MOTOR_FL_IN2, LOW);
    ledcWrite(MOTOR_FL_PWM_CH, 0);
  }

  if (sFR > 0) {
    digitalWrite(MOTOR_FR_IN2, HIGH);
    ledcWrite(MOTOR_FR_PWM_CH, 255 - sFR);
  } else if (sFR < 0) {
    digitalWrite(MOTOR_FR_IN2, LOW);
    ledcWrite(MOTOR_FR_PWM_CH, -sFR);
  } else {
    digitalWrite(MOTOR_FR_IN2, LOW);
    ledcWrite(MOTOR_FR_PWM_CH, 0);
  }

  if (sBL > 0) {
    digitalWrite(MOTOR_BL_IN2, HIGH);
    ledcWrite(MOTOR_BL_PWM_CH, 255 - sBL);
  } else if (sBL < 0) {
    digitalWrite(MOTOR_BL_IN2, LOW);
    ledcWrite(MOTOR_BL_PWM_CH, -sBL);
  } else {
    digitalWrite(MOTOR_BL_IN2, LOW);
    ledcWrite(MOTOR_BL_PWM_CH, 0);
  }

  if (sBR > 0) {
    digitalWrite(MOTOR_BR_IN2, HIGH);
    ledcWrite(MOTOR_BR_PWM_CH, 255 - sBR);
  } else if (sBR < 0) {
    digitalWrite(MOTOR_BR_IN2, LOW);
    ledcWrite(MOTOR_BR_PWM_CH, -sBR);
  } else {
    digitalWrite(MOTOR_BR_IN2, LOW);
    ledcWrite(MOTOR_BR_PWM_CH, 0);
  }
}

void logic() {
  speedLift = getSpeedSetting();
  int speedLow = 30;
  speedFL = getMotorSpeed(0);
  speedFR = getMotorSpeed(1);
  speedBL = getMotorSpeed(2);
  speedBR = getMotorSpeed(3);

  pos0 = getMotLifterPos(0);
  pos1 = getMotLifterPos(1);
  pos2 = getMotLifterPos(2);
  pos3 = getMotLifterPos(3);

  g1 = getGrip1();
  g2 = getGrip2();
  g1Open = getOpenG1();
  g2Open = getOpenG2();

  bool atas = PS4.data.button.up || LanalogAtas;
  bool bawah = PS4.data.button.down || LanalogBawah;
  bool kiri = PS4.data.button.left || LanalogKiri;
  bool kanan = PS4.data.button.right || LanalogKanan;
  bool serkatas = LanalogSerongAtasKanan;
  bool serkawah = LanalogSerongBawahKanan;
  bool serkitas = LanalogSerongAtasKiri;
  bool serkiwah = LanalogSerongBawahKiri;

  bool tarka = RanalogKanan;
  bool tarki = RanalogKiri;
  bool pakaiGrip2 = isGrip2Used();

  bool segitiga = PS4.data.button.triangle;
  bool bulat = PS4.data.button.circle;
  bool ex = PS4.data.button.cross;
  bool kotak = PS4.data.button.square;
  bool R1 = PS4.data.button.r1;
  bool R2 = PS4.data.button.r2;
  bool L1 = PS4.data.button.l1;
  bool L2 = PS4.data.button.l2;
  bool start = PS4.data.button.options;
  bool share = PS4.data.button.share;

  // Tombol start toggle sekali tekan
  if (start && !prevStartState && millis() - lastStartToggle > startDebounce) {
    openSetting = !openSetting;
    lastStartToggle = millis();

    if (openSetting) {
      inModeMenu = true;            // <-- Tambahkan ini saat membuka menu lagi
      drawModeMenu();               // <-- Biar menu langsung tampil lagi
    } else {
      closeMenu();                  // Tampilkan layar PRANOTO RACING TEAM saat keluar
    }
  }

  if (openSetting){
    updateSettingMenu(atas, bawah, ex, L1, R1, kotak);
  }
  else if(!openSetting){
    if (ex && kiri)               motor4(speedFL, -speedFR, speedBL, -speedBR);
    else if (ex && kanan)         motor4(-speedFL, speedFR, -speedBL, speedBR);
    else if (tarki)               motor4(speedFL, -speedFR, speedBL, -speedBR);
    else if (tarka)               motor4(-speedFL, speedFR, -speedBL, speedBR);
    else if (ex || atas)          motor4(L1 ? 255 : speedFL - speedLow, L1 ? 255 : speedFR - speedLow, L1 ? 255 : speedBL - speedLow, L1 ? 255 : speedBR - speedLow);
    else if (kotak || bawah)      motor4(-(L1 ? 255 : speedFL - speedLow), -(L1 ? 255 : speedFR - speedLow), -(L1 ? 255 : speedBL - speedLow), -(L1 ? 255 : speedBR - speedLow));
    else if (kiri)                motor4(-speedFL, speedFR, speedBL, -speedBR);
    else if (kanan)               motor4(speedFL, -speedFR, -speedBL, speedBR);
    else if (serkitas)            motor4(speedFL, 0, 0, speedBR);
    else if (serkiwah)            motor4(-speedFL, 0, 0, -speedBR);
    else if (serkatas)            motor4(0, speedFR, speedBL, 0);
    else if (serkawah)            motor4(0, -speedFR, -speedBL, 0);
    else                          motor4(0, 0, 0, 0);

    if (R1 && !R2) {
      angkat(speedLift);
    } else if (R2 && !R1) {
      turun(speedLift);
    } else {
      digitalWrite(MLIFT_D1, 0);
      digitalWrite(MLIFT_D2, 0);
      ledcWrite(MLIFT_CH, 0);
    }


    if (bulat && millis() - lastTutupPress > buttonDelay) {
      gripOpened = !gripOpened;
      if (gripOpened) CapitKI.write(g1Open);
      else            CapitKI.write(g1);
      lastTutupPress = millis();
    }
  }
}


void setupLift() {
  pinMode(MLIFT_PWM, OUTPUT);
  pinMode(MLIFT_D1, OUTPUT);
  pinMode(MLIFT_D2, OUTPUT);

  ledcAttachPin(MLIFT_PWM, MLIFT_CH);

  ledcSetup(MLIFT_CH, 490, 8);
}

void angkat(int spd){
  digitalWrite(MLIFT_D1, 1);
  digitalWrite(MLIFT_D2, 0);
  ledcWrite(MLIFT_CH, spd);
}

void turun(int spd){
  digitalWrite(MLIFT_D1, 0);
  digitalWrite(MLIFT_D2, 1);
  ledcWrite(MLIFT_CH, spd);
}