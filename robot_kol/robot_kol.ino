/*
  Robot Arm Control / Robot Kol Kontrol - Arduino Uno + PCA9685
  
  S1 (Channel/Kanal 1): MG996R - BASE / TABAN
    1=left/sol(330), 2=right/sag(370), 3=stop/dur(350)
  S0 (Channel/Kanal 0): DS 60KG - SHOULDER / OMUZ
    4=left/sol(300), 5=right/sag(400), 6=stop/dur(350)
  S2 (Channel/Kanal 2): MG996R - Positional / Pozisyonel
    7=Pos1/Poz1(150), 8=Pos2/Poz2(375), 9=Pos3/Poz3(600)
  S3 (Channel/Kanal 3): MG90S - 4 positions / 4 pozisyon
    Q=Pos1(100) W=Pos2(267) E=Pos3(433) R=Pos4(600)
  S4 (Channel/Kanal 4): MG90S - 4 positions / 4 pozisyon
    A=Pos1(100) S=Pos2(267) D=Pos3(433) F=Pos4(600)
  S5 (Channel/Kanal 5): MG90S - 3 positions / 3 pozisyon
    G=Pos1(100) H=Pos2(350) J=Pos3(600)
  
  Commands / Komutlar:
    1-9 = S0-S2 control / kontrol
    Q-W-E-R = S3 4 positions / pozisyon
    A-S-D-F = S4 4 positions / pozisyon
    G-H-J = S5 3 positions / pozisyon
    STOP = Stop all / Tumunu durdur
*/

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);

// === S0 (DS 60KG) ===
#define S0_SOL       300
#define S0_SAG       400
#define S0_DUR       350

// === S1 (MG996R) ===
#define S1_SOL       330
#define S1_SAG       370
#define S1_DUR       350

// === S2 (MG996R - Positional / Pozisyonel) ===
#define S2_POS1      150    // 7 - one end / bir uc (0 deg / derece)
#define S2_POS2      375    // 8 - middle / orta (90 deg / derece)
#define S2_POS3      600    // 9 - other end / diger uc (180 deg / derece)
#define S2_ADIM      2      // Pulse step per move / Her adimda kac pulse (small=slow / kucuk=yavas)
#define S2_BEKLEME   15     // Delay between steps ms / Adimlar arasi bekleme ms (big=slow / buyuk=yavas)

// === S3 (MG90S) - 4 Positions / Pozisyon ===
#define S3_POS1      100    // Q - far left / en sol
#define S3_POS2      267    // W
#define S3_POS3      433    // E
#define S3_POS4      600    // R - far right / en sag

// === S4 (MG90S) - 4 Positions / Pozisyon ===
#define S4_POS1      100    // A - far left / en sol
#define S4_POS2      267    // S
#define S4_POS3      433    // D
#define S4_POS4      600    // F - far right / en sag

// === S5 (MG90S) - 3 Positions / Pozisyon ===
#define S5_POS1      100    // G - one end / bir uc
#define S5_POS2      350    // H - middle / orta
#define S5_POS3      600    // J - other end / diger uc

// === Timing settings / Zaman ayarlari ===
#define HAREKET_SURESI  500

String inputString = "";
bool stringComplete = false;
int s2Pozisyon = S2_POS2;  // S2 starting position / baslangic pozisyonu (middle / orta)

void setup() {
  Serial.begin(115200);
  
  pwm.begin();
  pwm.setPWMFreq(50);
  delay(10);
  
  // Turn off all channels at startup / Baslangicta tum kanallari kapat
  for (int i = 0; i < 16; i++) {
    pwm.setPWM(i, 0, 4096);
  }
  
  Serial.println("=== ROBOT KOL KONTROL ===");
  Serial.println("S0-S2:");
  Serial.println("  1=S1sol 2=S1sag 3=S1dur");
  Serial.println("  4=S0sol 5=S0sag 6=S0dur");
  Serial.println("  7=S2poz1 8=S2poz2 9=S2poz3");
  Serial.println("S3: Q=Poz1 W=Poz2 E=Poz3 R=Poz4");
  Serial.println("S4: A=Poz1 S=Poz2 D=Poz3 F=Poz4");
  Serial.println("S5: G=Poz1 H=Poz2 J=Poz3");
  Serial.println("STOP = Tumunu durdur");
  Serial.println();
}

void loop() {
  // Read serial input / Seri porttan veri oku
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == '\n') {
      stringComplete = true;
    } else if (inChar != '\r') {
      inputString += inChar;
    }
  }
  
  // Process command when line is complete / Satir tamamlandiginda komutu isle
  if (stringComplete) {
    inputString.trim();
    inputString.toUpperCase();
    handleCommand(inputString);
    inputString = "";
    stringComplete = false;
  }
}

void handleCommand(String cmd) {
  // Stop everything / Her seyi durdur
  if (cmd == "STOP") {
    killAll();
    Serial.println("OK STOP");
    return;
  }
  
  // Numeric commands (1-9) / Sayisal komutlar (1-9)
  if (cmd == "1") { hareketEt(1, S1_SOL); return; }
  if (cmd == "2") { hareketEt(1, S1_SAG); return; }
  if (cmd == "3") { hareketEt(1, S1_DUR); return; }
  if (cmd == "4") { hareketEt(0, S0_SOL); return; }
  if (cmd == "5") { hareketEt(0, S0_SAG); return; }
  if (cmd == "6") { hareketEt(0, S0_DUR); return; }
  if (cmd == "7") { hareketEtYavasS2(S2_POS1); return; }
  if (cmd == "8") { hareketEtYavasS2(S2_POS2); return; }
  if (cmd == "9") { hareketEtYavasS2(S2_POS3); return; }
  
  // S3 (Q-W-E-R) - 4 positions, signal cut to avoid heating / 4 pozisyon, sinyal kesilir, isinmaz
  if (cmd == "Q") { hareketEt(3, S3_POS1); return; }
  if (cmd == "W") { hareketEt(3, S3_POS2); return; }
  if (cmd == "E") { hareketEt(3, S3_POS3); return; }
  if (cmd == "R") { hareketEt(3, S3_POS4); return; }
  
  // S4 (A-S-D-F) - 4 positions, signal cut to avoid heating / 4 pozisyon, sinyal kesilir, isinmaz
  if (cmd == "A") { hareketEt(4, S4_POS1); return; }
  if (cmd == "S") { hareketEt(4, S4_POS2); return; }
  if (cmd == "D") { hareketEt(4, S4_POS3); return; }
  if (cmd == "F") { hareketEt(4, S4_POS4); return; }
  
  // S5 (G-H-J) - 3 positions, signal cut to avoid heating / 3 pozisyon, sinyal kesilir, isinmaz
  if (cmd == "G") { hareketEt(5, S5_POS1); return; }
  if (cmd == "H") { hareketEt(5, S5_POS2); return; }
  if (cmd == "J") { hareketEt(5, S5_POS3); return; }
  
  // Unknown command / Bilinmeyen komut
  Serial.println("ERR: 1-9 / Q-W-E-R / A-S-D-F / G-H-J / STOP");
}

// Standard move: send pulse, wait, cut signal / Standart hareket: pulse gonder, bekle, sinyali kes
void hareketEt(int kanal, int pulse) {
  Serial.print("S"); Serial.print(kanal);
  Serial.print(" pulse="); Serial.println(pulse);
  
  pwm.setPWM(kanal, 0, pulse);
  delay(HAREKET_SURESI);
  pwm.setPWM(kanal, 0, 4096);
  
  Serial.println("OK");
}

// S2 slow move - steps gradually to target / S2 yavas hareket - adim adim hedefe gider
void hareketEtYavasS2(int hedef) {
  Serial.print("S2 yavas: "); Serial.print(s2Pozisyon);
  Serial.print(" -> "); Serial.println(hedef);
  
  while (s2Pozisyon != hedef) {
    if (s2Pozisyon < hedef) {
      s2Pozisyon += S2_ADIM;
      if (s2Pozisyon > hedef) s2Pozisyon = hedef;
    } else {
      s2Pozisyon -= S2_ADIM;
      if (s2Pozisyon < hedef) s2Pozisyon = hedef;
    }
    pwm.setPWM(2, 0, s2Pozisyon);
    delay(S2_BEKLEME);
  }
  
  Serial.println("OK");
}

// Positional servo - keeps signal, holds angle / Pozisyonel servo - sinyal kesilmez, acida sabit kalir
void hareketEtPoz(int kanal, int pulse) {
  Serial.print("S"); Serial.print(kanal);
  Serial.print(" poz="); Serial.println(pulse);
  
  pwm.setPWM(kanal, 0, pulse);
  
  Serial.println("OK");
}

// Cut signal on all channels / Tum kanallarin sinyalini kes
void killAll() {
  for (int i = 0; i < 16; i++) {
    pwm.setPWM(i, 0, 4096);
  }
}
