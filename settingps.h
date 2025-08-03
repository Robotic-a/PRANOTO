#ifndef SETTINGPS_H
#define SETTINGPS_H

#include <Wire.h>
#include <EEPROM.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "icons.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define EEPROM_SIZE 128

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- EEPROM Struct ---
struct SettingData {
  int speedLifter;
  int motorSpeed[4];
  int stepPos[4];
  int motLifterPos[4];
  int grip1, grip2;
  int openG1, openG2;
  bool useGrip2;
};

// --- State & Debounce Timing ---
bool inModeMenu = true, inResetConfirm = false, editingParam = false;
int selectedMode = 0, selectedParamIndex = 0;
bool confirmResetYes = true;

// Debounce
unsigned long lastMenuActionTime = 0;
const unsigned long menuDelay = 200;

// Mode List
const char* modeOptions[] = {"MOTOR", "STEP LIFTER", "MOT LIFTER", "GRIP", "SYSTEM"};
const int modeCount = 5;

// Parameter Values
int speedLifter = 150;
int motorSpeed[4] = {100, 100, 100, 100};
int stepPos[4]     = {0, 325, 600, 800};
int motLifterPos[4]= {0, 300, 550, 800};
int grip1 = 90, grip2 = 90;
int openG1 = 90, openG2 = 90;
bool useGrip2 = false;

int batteryPower = 75;
int batteryStik  = 40;

// --- Function Prototypes ---
void handleModeMenu(bool, bool, bool, bool, bool, bool);
void handleResetConfirm(bool, bool, bool, bool);
void resetAllSettings();
void drawModeMenu();
void drawResetConfirm();
bool canUpdateMenu();
void saveSettingsToEEPROM();
void loadSettingsFromEEPROM();

// --- Initialize OLED + EEPROM ---
void initSettingan() {
  EEPROM.begin(EEPROM_SIZE);
  loadSettingsFromEEPROM();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 failed"));
    while (1);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("MAMATHOREE...");
  display.display();
  delay(500);
}

// --- EEPROM Save/Load ---
void saveSettingsToEEPROM() {
  SettingData data;
  data.speedLifter = speedLifter;
  memcpy(data.motorSpeed, motorSpeed, sizeof(motorSpeed));
  memcpy(data.stepPos, stepPos, sizeof(stepPos));
  memcpy(data.motLifterPos, motLifterPos, sizeof(motLifterPos));
  data.grip1 = grip1;
  data.grip2 = grip2;
  data.openG1 = openG1;
  data.openG2 = openG2;
  data.useGrip2 = useGrip2;

  EEPROM.put(0, data);
  EEPROM.commit();
}

void loadSettingsFromEEPROM() {
  SettingData data;
  EEPROM.get(0, data);

  speedLifter = data.speedLifter;
  memcpy(motorSpeed, data.motorSpeed, sizeof(motorSpeed));
  memcpy(stepPos, data.stepPos, sizeof(stepPos));
  memcpy(motLifterPos, data.motLifterPos, sizeof(motLifterPos));
  grip1 = data.grip1;
  grip2 = data.grip2;
  openG1 = data.openG1;
  openG2 = data.openG2;
  useGrip2 = data.useGrip2;
}

// --- Debounce Check ---
bool canUpdateMenu() {
  if (millis() - lastMenuActionTime > menuDelay) {
    lastMenuActionTime = millis();
    return true;
  }
  return false;
}

// --- Menu Update ---
void updateSettingMenu(bool btnUp, bool btnDown, bool btnEnter,
                       bool btnKiri, bool btnKanan, bool btnBack) {
  if (inResetConfirm) {
    handleResetConfirm(btnUp, btnDown, btnEnter, btnBack);
  } else if (inModeMenu) {
    handleModeMenu(btnUp, btnDown, btnEnter, btnKiri, btnKanan, btnBack);
  }
}

// --- Mode Menu Logic ---
void handleModeMenu(bool btnUp, bool btnDown, bool btnEnter, bool btnKiri, bool btnKanan, bool btnBack) {
  const int maxParams = 6;
  selectedParamIndex = constrain(selectedParamIndex, 0, maxParams - 1);

  if (!editingParam) {
    if (btnUp && canUpdateMenu()) selectedParamIndex = (selectedParamIndex + maxParams - 1) % maxParams;
    if (btnDown && canUpdateMenu()) selectedParamIndex = (selectedParamIndex + 1) % maxParams;
    if (btnKiri && canUpdateMenu()) {
      selectedMode = (selectedMode + modeCount - 1) % modeCount;
      selectedParamIndex = 0;
    }
    if (btnKanan && canUpdateMenu()) {
      selectedMode = (selectedMode + 1) % modeCount;
      selectedParamIndex = 0;
    }
    if (btnEnter && canUpdateMenu()) {
      if (selectedMode == 4 && selectedParamIndex == 0) {
        inResetConfirm = true;
        confirmResetYes = true;
      } else {
        editingParam = true;
      }
    }
  } else {
    if (btnUp && canUpdateMenu()) {
      switch (selectedMode) {
        case 0: if (selectedParamIndex < 4) motorSpeed[selectedParamIndex] += 5; break;
        case 1: if (selectedParamIndex < 4) stepPos[selectedParamIndex] += 5; break;
        case 2: 
          if (selectedParamIndex < 4) motLifterPos[selectedParamIndex] += 5; 
          else if (selectedParamIndex == 4) speedLifter += 5;
          break;
        case 3:
          if (selectedParamIndex == 0) grip1 += 3;
          else if (selectedParamIndex == 1) grip2 += 3;
          else if (selectedParamIndex == 2) openG1 += 3;
          else if (selectedParamIndex == 3) openG2 += 3;
          else if (selectedParamIndex == 4) useGrip2 = !useGrip2;
          break;
      }
    }
    if (btnDown && canUpdateMenu()) {
      switch (selectedMode) {
        case 0: if (selectedParamIndex < 4) motorSpeed[selectedParamIndex] -= 1; break;
        case 1: if (selectedParamIndex < 4) stepPos[selectedParamIndex] -= 1; break;
        case 2: 
          if (selectedParamIndex < 4) motLifterPos[selectedParamIndex] -= 1;
          else if (selectedParamIndex == 4) speedLifter -= 1;
          break;
        case 3:
          if (selectedParamIndex == 0) grip1 -= 1;
          else if (selectedParamIndex == 1) grip2 -= 1;
          else if (selectedParamIndex == 2) openG1 -= 1;
          else if (selectedParamIndex == 3) openG2 -= 1;
          else if (selectedParamIndex == 4) useGrip2 = !useGrip2;
          break;
      }
    }
    if (btnBack && canUpdateMenu()) {
      editingParam = false;
      saveSettingsToEEPROM();
    }
  }

  drawModeMenu();
}

// --- Reset Menu Logic ---
void handleResetConfirm(bool btnUp, bool btnDown, bool btnEnter, bool btnBack) {
  if ((btnUp || btnDown) && canUpdateMenu()) confirmResetYes = !confirmResetYes;
  if (btnEnter && canUpdateMenu()) {
    if (confirmResetYes) resetAllSettings();
    inResetConfirm = false;
  }
  if (btnBack && canUpdateMenu()) {
    inResetConfirm = false;
  }
  drawResetConfirm();
}

// --- Reset Defaults ---
void resetAllSettings() {
  speedLifter = 150;
  for (int i = 0; i < 4; i++) {
    motorSpeed[i] = 100;
    stepPos[i]      = 300 + i * 150;
    motLifterPos[i] = 300 + i * 150;
  }
  grip1 = grip2 = 90;
  useGrip2 = false;
  openG1 = openG2 = 90;

  saveSettingsToEEPROM();
}

// --- Menu Drawing ---
void drawModeMenu() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.print("Mode: "); display.print(modeOptions[selectedMode]);
  if (editingParam) display.print(" [E]");
  display.println();

  const int totalParams = 6, maxVis = 3;
  int startIdx = constrain(selectedParamIndex - 1, 0, totalParams - maxVis);
  int endIdx = min(startIdx + maxVis, totalParams);

  for (int i = startIdx; i < endIdx; i++) {
    display.print(i == selectedParamIndex ? "> " : "  ");
    switch (selectedMode) {
      case 0: display.print("M"); display.print(i + 1); display.print(": ");
              if (i < 4) display.println(motorSpeed[i]); break;
      case 1: display.print("S"); display.print(i); display.print(": ");
              if (i < 4) display.println(stepPos[i]); break;
      case 2: 
        if (i == 0) display.println(String("LPos0: ") + motLifterPos[0]);
        else if (i == 1) display.println(String("LPos1: ") + motLifterPos[1]);
        else if (i == 2) display.println(String("LPos2: ") + motLifterPos[2]);
        else if (i == 3) display.println(String("LPos3: ") + motLifterPos[3]);
        else if (i == 4) display.println(String("LSpeed: ") + speedLifter);
        break;
      case 3:
        if (i == 0) display.println(String("G1: ") + grip1);
        else if (i == 1) display.println(String("G2: ") + grip2);
        else if (i == 2) display.println(String("OpenG1: ") + openG1);
        else if (i == 3) display.println(String("OpenG2: ") + openG2);
        else if (i == 4) display.println(String("Use2: ") + (useGrip2 ? "YES" : "NO"));
        break;
      case 4:
        if (i == 0) display.println("Reset All");
        break;
    }
  }
  display.display();
}

void drawResetConfirm() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.println("Reset All?");
  display.println(confirmResetYes ? "> YES" : "  YES");
  display.println(confirmResetYes ? "  NO" : "> NO");
  display.display();
}

void setBatteryStatus(int power, int stik) {
  batteryPower = constrain(power, 0, 100);
  batteryStik = constrain(stik, 0, 100);
  saveSettingsToEEPROM();
}

void closeMenu() {
  inModeMenu = false;
  inResetConfirm = false;
  editingParam = false;

  display.clearDisplay();

  display.drawBitmap(3, 29, epd_bitmap_power_bat, 58, 20, WHITE);
  display.drawBitmap(67, 29, epd_bitmap_stik_bat, 58, 20, WHITE);

  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor((128 - 42) / 2, 1);
  display.print("PRANOTO");

  display.setCursor((128 - 66) / 2, 10);
  display.print("RACING TEAM");

  display.setCursor(18, 35);
  display.print(String(batteryPower) + "%");

  display.setCursor(16, 55);
  display.print("BATT");

  display.setCursor(82, 35);
  display.print(String(batteryStik) + "%");

  display.setCursor(82, 55);
  display.print("STIK");

  display.display();
}

// --- Getter Functions ---
inline int getSpeedSetting() { return speedLifter; }
inline int getMotorSpeed(int i) { return motorSpeed[i]; }
inline int getStepPos(int i) { return stepPos[i]; }
inline int getMotLifterPos(int i) { return motLifterPos[i]; }
inline int getGrip1() { return grip1; }
inline int getGrip2() { return grip2; }
inline bool isGrip2Used() { return useGrip2; }
inline int getOpenG1() { return openG1; }
inline int getOpenG2() { return openG2; }

#endif  // SETTINGPS_H










