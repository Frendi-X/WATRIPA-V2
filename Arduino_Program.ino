#include<LiquidCrystal_I2C.h>
#include<SoftwareSerial.h>

#define RX_Pin 9
#define TX_Pin 10

LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial mySerial(RX_Pin, TX_Pin);

/* -------------------------------------- PIN RELAY ------------------------------------- */
#define RelayPompa 7
#define RelayAsam 8
#define RelayBasa 9
/* --------------------------------------- PIN DMS -------------------------------------- */
#define DMSpin  6
#define indikator 13
#define adcPin A0
/* ------------------------------------ PIN LAIN-LAIN ----------------------------------- */
#define ledPin 3
#define Buzzer 4
#define sensorPIR 5
#define SP_tanahPin A1       // A1 -> Pin Potensio Set Nilai tanah
#define SP_PHPin A2          // A2 -> Pin Potensio Set Nilai PH
#define SoilMoisture_Pin A3  // A3 -> Pin Sensor Soil Moisture (Kelembaban Tanah)
#define adaGerakan 1


/* ------------------------------------- VARIABEL DMS ------------------------------------ */
int AnalogValue;
float lastReading;
float pH;
/* ---------------------------------- VARIABEL SETPOINT PH ------------------------------- */
int SP_tanah,
    SP_PH,
    kelembabantanah;
float PH;
/* -------------------------------- VARIABEL DATA TANAH & PH ----------------------------- */
int tanahbasah = 500,
    tanahkering = 750,
    PH_Basa,
    PH_Asam;
byte sendd = 0,
     senddd = 0;
/* ---------------------------------- VARIABEL PARSING DATA ------------------------------- */
String data;
char CharData;
String StringData, dataSubs;
int index1, index2;
int Data;
/* ------------------------------------ VARIABEL MILLIS ----------------------------------- */
unsigned long hitungan_milis;
unsigned long milis_sekarang;
const unsigned long nilai = 100;

int readSensor() {
  int val = analogRead(sensorPin);
  return val;
}

int readSP_tanah() {
  int val1 = analogRead(SP_tanahPin);
  int setPoint_PH = val1;
  return setPoint_PH;
}

int readSP_PH() {
  int val2 = analogRead(SP_PHPin);
  int setPoint_tanah = map(val2, 0, 1023, 0, 14);
  return setPoint_tanah;
}

float DMS_readData() {
  digitalWrite(DMSpin, LOW);     // aktifkan DMS
  digitalWrite(indikator, HIGH); // led indikator built-in menyala
  delay(5 * 1000);               // wait DMS capture data
  AnalogValue = analogRead(adcPin);

  pH = (-0.0233 * ADC) + 12.698;  // ini adalah rumus regresi linier konversi adc ke pH
  if (pH != lastReading) {
    lastReading = pH;
  }

  if (lastReading > 8.0) {
    lastReading = 0.0; // nol kan nilai pH saat out of range
  }

  Serial.print("ADC=");
  Serial.print(AnalogValue);     // menampilkan nilai ADC di serial monitor pada baudrate 115200
  Serial.print(" pH=");
  Serial.println(lastReading, 1); // menampilkan nilai pH di serial monitor pada baudrate 115200

  digitalWrite(DMSpin, HIGH);
  digitalWrite(indikator, LOW);
  delay(3 * 1000);               // wait for DMS ready

  return lastReading;
}

void setup() {
  Serial.begin(9600);
  mySerial.begin(115200);

  lcd.init();
  lcd.backlight();

  pinMode(DMSpin, OUTPUT);
  pinMode(indikator, OUTPUT);
  digitalWrite(DMSpin, HIGH);
  pinMode(Buzzer, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(RelayPompa,  OUTPUT); pinMode(RelayAsam,  OUTPUT);  pinMode(RelayBasa,  OUTPUT);
  pinMode(SP_tanahPin, INPUT); pinMode(SP_PHPin, INPUT);
  pinMode(sensorPIR, INPUT_PULLUP);
  digitalWrite(Buzzer, LOW);
  digitalWrite(ledPin, LOW);
  digitalWrite(RelayPompa, HIGH); digitalWrite(RelayAsam, HIGH); digitalWrite(RelayBasa, HIGH);

  for (int i = 0; i <= 10; i++) {
    tanahbasah = readSP_tanah();
    lcd.setCursor(0, 0);
    lcd.print("Tanah(B) :");
    lcd.print(tanahbasah);
    lcd.print("   ");
    lcd.setCursor(0, 1);
    lcd.print("Setting");
    delay(1000);
  }
  lcd.clear();
  delay(100);

  for (int i = 0; i <= 10; i++) {
    tanahkering = readSP_tanah();
    lcd.setCursor(0, 0);
    lcd.print("Tanah(K) :");
    lcd.print(tanahkering);
    lcd.print("   ");
    lcd.setCursor(0, 1);
    lcd.print("Setting");
    delay(1000);
  }
  lcd.clear();
  delay(100);
  for (int i = 0; i <= 10; i++) {
    PH_Asam = readSP_PH();
    lcd.setCursor(0, 0);
    lcd.print("PH(A) :");
    lcd.print(PH_Asam);
    lcd.print("   ");
    lcd.setCursor(0, 1);
    lcd.print("Setting");
    delay(1000);
  }
  lcd.clear();
  delay(100);
  for (int i = 0; i <= 10; i++) {
    PH_Basa = readSP_PH();
    lcd.setCursor(0, 0);
    lcd.print("PH(B) :");
    lcd.print(PH_Basa);
    lcd.print("   ");
    lcd.setCursor(0, 1);
    lcd.print("Setting");
    delay(1000);
  }
  lcd.clear();
  delay(100);
}

void loop() {
  /* -------------------------- LOOP PEMBACAAN DATA DARI ESP32 CAM -------------------- */
  while (mySerial.available() > 0)
  {
    delay(10);
    CharData = mySerial.read();
    StringData += CharData;

    /* ------------------------- PARSING DATA MASUK DARI ESP32 CAM -------------------- */
    if (StringData.length() > 0 && CharData == '?')
    {
      index1 = StringData.indexOf('#');
      index2 = StringData.indexOf('?', index1 + 1);
      dataSubs = StringData.substring(index1 + 1, index2);
      StringData = "";

      //Mengubah Data String ke Float
      char buf[dataSubs.length()];
      dataSubs.toCharArray(buf, dataSubs.length() + 1);
      float Data1 = atof(buf);

      /* --------------------------- DATA MASUK DARI ESP32 CAM ------------------------ */
      Data = Data1;
      Serial.print("Data Masuk : ");
      Serial.println(Data);

      /* ---------------------------- SEND DATA KE ESP32 CAM -------------------------- */
      /* -------------------- KATEGORI PENGELOMPOKAN STATUS TANAH --------------------- */
      if (Data == 1) {
        if (kelembabantanah < tanahbasah) {
          mySerial.print("#1!"); // 1 -> Status Tanah Basah
        }
        else if (kelembabantanah >= tanahbasah && kelembabantanah <= tanahkering) {
          mySerial.print("#2!"); // 2 -> Status Tanah Normal
        }
        else if (kelembabantanah > tanahkering) {
          mySerial.print("#3!"); // 3 -> Status Tanah Kering
        }

        /* -------------------- KATEGORI PENGELOMPOKAN STATUS PH ---------------------- */
        if (PH < PH_Asam) {
          mySerial.print("#11!"); // 11 -> Status PH Asam
        }
        else if (PH >= PH_Asam && PH <= PH_Basa) {
          mySerial.print("#22!"); // 22 -> Status PH Normal
        }
        else if (PH > PH_Basa) {
          mySerial.print("#33!"); // 33 -> Status PH Basa
        }
        /* ----------------------- SEND DATA PH KE ESP32 CAM -------------------------- */
        mySerial.print("#" + String(PH) + "@");
        /* ------------------------------ RESET DATA ---------------------------------- */
        Data = "";
      }
    }
  }

  /* -------------------------------- PERINGATAN SENSOR PIR --------------------------- */
  milis_sekarang = millis();
  if (milis_sekarang - hitungan_milis >= nilai)
  {
    if (digitalRead(sensorPIR) == adaGerakan) {
      Serial.print(digitalRead(sensorPIR));
      digitalWrite(Buzzer, HIGH);
      digitalWrite(ledPin, HIGH);
      delay(100);
      digitalWrite(Buzzer, LOW);
      digitalWrite(ledPin, LOW);
      delay(100);
    }
  }

  /* ---------------------------- PEMBACAAN FUNGSI SENSOR-SENSOR ----------------------- */
  // PH = readSP_PH();
  PH = DMS_readData();
  kelembabantanah = readSensor();
  SP_tanah = readSP_tanah();

  /* -------------------------- LCD DISPLAY DATA KELEMBABAN DAN PH --------------------- */
  lcd.setCursor(12, 0);
  lcd.print(kelembabantanah);
  lcd.print(" ");
  lcd.setCursor(12, 1);
  lcd.print(PH);
  lcd.print(" ");

  Serial.print(digitalRead(sensorPIR));
  Serial.print("\t\tADC Soil : ");
  Serial.print(kelembabantanah);
  Serial.print("\t\tSP Tanah : ");
  Serial.print(SP_tanah);
  Serial.print("\t\tSP PH : ");
  Serial.println(SP_PH);

  /* ------------------------------ KONTROL POMPA TANAH BASAH --------------------------- */
  if (kelembabantanah < tanahbasah) {
    Serial.println("Status: Tanah Basah");
    digitalWrite(RelayPompa, HIGH);
    lcd.setCursor(0, 0);
    lcd.print("Tanah Basah ");
    sendd = 1;
  }
  /* ------------------------------ KONTROL POMPA TANAH NORMAL --------------------------- */
  else if (kelembabantanah >= tanahbasah && kelembabantanah <= tanahkering) {
    Serial.println("Status: Tanah Normal");
    digitalWrite(RelayPompa, HIGH);
    lcd.setCursor(0, 0);
    lcd.print("Tanah Normal");
    sendd = 1;
  }
  /* ------------------------------ KONTROL POMPA TANAH KERING --------------------------- */
  else if (kelembabantanah > tanahkering) {
    Serial.println("Status: Tanah Kering");
    digitalWrite(RelayPompa, LOW);
    lcd.setCursor(0, 0);
    lcd.print("Tanah Kering");
    while (sendd <= 1) {
      if (sendd == 2) break;
      mySerial.print("#1?");
      sendd = 2;
    }
  }

  /* -------------------------------- KONTROL POMPA PH ASAM ------------------------------ */
  if (PH < PH_Asam) {
    Serial.println("Status: PH Asam");
    digitalWrite(RelayAsam, LOW);
    lcd.setCursor(0, 1);
    lcd.print("PH Asam  ");
    senddd = 1;
  }
  /* ------------------------------- KONTROL POMPA PH NORMAL ------------------------------ */
  else if (PH >= PH_Asam && PH <= PH_Basa) {
    Serial.println("Status: PH Normal");
    digitalWrite(RelayAsam, HIGH);  digitalWrite(RelayBasa, HIGH);
    lcd.setCursor(0, 1);
    lcd.print("PH Normal");
    senddd = 1;
  }
  /* -------------------------------- KONTROL POMPA PH BASA --=---------------------------- */
  else if (PH > PH_Basa) {
    Serial.println("Status: PH Basa");
    digitalWrite(RelayBasa, LOW);
    lcd.setCursor(0, 1);
    lcd.print("PH Basa  ");
    while (senddd <= 1) {
      if (senddd == 2) break;
      mySerial.print("#2?");
      senddd = 2;
    }
  }
  delay(100);
  Serial.println();
}
