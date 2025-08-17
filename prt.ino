#define ENA 4
#define ENB 18

#define DIR1 25
#define DIR2 19
#define DIR3 12
#define DIR4 13

#define CHA 5
#define CHB 6

const int S0 = 14;
const int S1 = 27;
const int S2 = 26;

const int Z1 = 33; // COM IC1 -> ADC1
const int Z2 = 34; // COM IC2 -> ADC1

int sensors[16];
int urutinlah[14];

bool majuhitam = false;
bool majuputih = false;

bool kananhitam = false;
bool kananputih = false;

bool kirihitam = false;
bool kiriputih = false;

int speed = 200;
int spdcor = 190;
int rangehitam = 3000;

void setup() {
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  Serial.begin(115200);
  motorpwm();
}

void loop() {
  // --- baca multiplexer ---
  for (int ch = 0; ch < 8; ch++) {
    digitalWrite(S0, ch & 0x01);
    digitalWrite(S1, (ch >> 1) & 0x01);
    digitalWrite(S2, (ch >> 2) & 0x01);
    delayMicroseconds(50);

    int v1 = analogRead(Z1);
    int v2 = analogRead(Z2);

    sensors[ch]     = v1;
    sensors[ch + 8] = v2;
  }

  // --- mapping urutan sensor ---
  urutinlah[0] = sensors[8];
  urutinlah[1] = sensors[9];
  urutinlah[2] = sensors[10];
  urutinlah[3] = sensors[11];
  urutinlah[4] = sensors[12];
  urutinlah[5] = sensors[13];
  urutinlah[6] = sensors[14];
  urutinlah[7] = sensors[0];
  urutinlah[8] = sensors[4];
  urutinlah[9] = sensors[2];
  urutinlah[10] = sensors[7];
  urutinlah[11] = sensors[1];
  urutinlah[12] = sensors[5];
  urutinlah[13] = sensors[3];

  // --- debug print ---
  // for (int i = 0; i < 14; i++) {
  //   Serial.print(urutinlah[i]);
  //   Serial.print(i < 13 ? ',' : '\n');
  // }
  Serial.print("majuhitam: ");
Serial.print(majuhitam);
Serial.print(" , ");

Serial.print("kananhitam: ");
Serial.print(kananhitam);
Serial.print(" , ");

Serial.print("kirihitam: ");
Serial.print(kirihitam);
Serial.println(" , ");

  // --- reset flag ---
  majuhitam = false;
  majuputih = false;
  kananhitam = false;
  kananputih = false;
  kirihitam = false;
  kiriputih = false;

  // --- cek apakah hitam valid ---
  bool tengahHitam = (urutinlah[5] > rangehitam || urutinlah[6] > rangehitam || urutinlah[7] > rangehitam || urutinlah[8] > rangehitam);
  bool kiriHitam = (urutinlah[0] > rangehitam || urutinlah[1] > rangehitam || urutinlah[2] > rangehitam || urutinlah[3] > rangehitam  || urutinlah[4] > rangehitam);
  bool kananHitam = (urutinlah[9] > rangehitam || urutinlah[10] > rangehitam || urutinlah[11] > rangehitam || urutinlah[12] > rangehitam  || urutinlah[13] > rangehitam);

  bool luarPutih = true;
  for (int i = 0; i < 14; i++) {
    if (i >= 5 && i <= 8) continue; // skip sensor tengah
    if (urutinlah[i] >= 2000) {
      luarPutih = false;
      break;
    }
  }
  bool kananPutih = true;
  for (int i = 0; i < 14; i++) {
    if (i >= 0 && i <= 5) continue; // skip sensor kanan
    if (urutinlah[i] >= 2000) {
      kananPutih = false;
      break;
    }
  }
  bool kiriPutih = true;
  for (int i = 0; i < 14; i++) {
    if (i >= 8 && i <= 13) continue; // skip sensor kiri
    if (urutinlah[i] >= 2000) {
      kiriPutih = false;
      break;
    }
  }

  if (tengahHitam) {
    majuhitam = true;
  }
  else if(kananHitam || kiriPutih){
    kananhitam = true;
  }
  else if(kiriHitam || kananPutih){
    kirihitam = true;
  }

  // --- logika gerakan ---
  if (majuhitam) {
    if (urutinlah[5] > rangehitam || urutinlah[6] > rangehitam || urutinlah[7] > rangehitam || urutinlah[8] > rangehitam) {
      mj(speed);   // maju
    }
    else {
      Serial.println("error");
      hop();
    }
  } 
  else if(kananhitam){ 
    if (urutinlah[2] > rangehitam || urutinlah[3] > rangehitam || urutinlah[4] > rangehitam) {
      kiri(spdcor-50); // garis lebih ke kiri → belok kanan
    } 
    else {
      Serial.println("Kelebihan kanan");
      kiri(spdcor);
    }
  }
  else if(kirihitam){
    if (urutinlah[9] > rangehitam || urutinlah[10] > rangehitam || urutinlah[11] > rangehitam) {
      kanan(spdcor-50); // garis lebih ke kanan → belok kiri
    } 
    else {
      Serial.println("Kelebihan kiri");
      kanan(spdcor);
    }
  }
  else {
    hop(); // tidak ada hitam sama sekali
  }

  delay(50);
}

// ================= MOTOR CONTROL =================

void motorpwm() {
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  pinMode(DIR1, OUTPUT);
  pinMode(DIR2, OUTPUT);
  pinMode(DIR3, OUTPUT);
  pinMode(DIR4, OUTPUT);

  ledcAttachPin(ENA, CHA);
  ledcAttachPin(ENB, CHB);

  ledcSetup(CHA, 490, 8);
  ledcSetup(CHB, 490, 8);
}

void mj(int spd){
  digitalWrite(DIR1, 1);
  digitalWrite(DIR2, 0);
  digitalWrite(DIR3, 1);
  digitalWrite(DIR4, 0);
  ledcWrite(CHA, spd);
  ledcWrite(CHB, spd);
}

void kanan(int spd){
  digitalWrite(DIR1, 0);
  digitalWrite(DIR2, 1);
  digitalWrite(DIR3, 1);
  digitalWrite(DIR4, 0);
  ledcWrite(CHA, spd);
  ledcWrite(CHB, spd);
}

void kiri(int spd){
  digitalWrite(DIR1, 1);
  digitalWrite(DIR2, 0);
  digitalWrite(DIR3, 0);
  digitalWrite(DIR4, 1);
  ledcWrite(CHA, spd);
  ledcWrite(CHB, spd);
}

void hop(){
  digitalWrite(DIR1, 0);
  digitalWrite(DIR2, 0);
  digitalWrite(DIR3, 0);
  digitalWrite(DIR4, 0);
  ledcWrite(CHA, 0);
  ledcWrite(CHB, 0);
}
