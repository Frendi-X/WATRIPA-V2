#include<LiquidCrystal_I2C.h>
#include<SoftwareSerial.h>
#include <Servo.h>

#define RX_Pin 10
#define TX_Pin 11

LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial mySerial(RX_Pin, TX_Pin);
Servo myServo;

/* -------------------------------------- PIN RELAY ------------------------------------- */
#define RelayPompa 7
#define RelayAsam 8
#define RelayBasa 9
/* --------------------------------------- PIN DMS -------------------------------------- */
#define DMSpin  6
#define indikator 13
#define adcPin A0
/* ------------------------------------ PIN LAIN-LAIN ----------------------------------- */
#define servoPin 2
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

unsigned long lastServoMoveTime = 0;
const unsigned long servoMoveInterval = 10; // waktu antar langkah pergerakan (ms)
int currentAngle = 0;
int servoDirection = 1; // 1 = maju (0→180), -1 = mundur (180→0)

unsigned long dmsLastReadTime = 0;
unsigned long superloop = 0;

/* ----------------------- FUNGSI PEMBACAAN SENSOR SOIL MOSTURE --------------------------- */
int readSensor() {
  int val = analogRead(SoilMoisture_Pin);
  return val;
}
/* ---------------------- FUNGSI PEMBACAAN POTENSIO SETPOINT TANAH ------------------------ */
int readSP_tanah() {
  int val1 = analogRead(SP_tanahPin);
  int setPoint_PH = val1;
  return setPoint_PH;
}
/* ------------------------ FUNGSI PEMBACAAN POTENSIO SETPOINT PH ------------------------- */
int readSP_PH() {
  int val2 = analogRead(SP_PHPin);
  int setPoint_tanah = map(val2, 0, 1023, 0, 14);
  return setPoint_tanah;
}
/* ------------------------ FUNGSI PEMBACAAN SENSOR DMS (PH TANAH) ------------------------ */
float DMS_readData() {
  digitalWrite(DMSpin, LOW);
  digitalWrite(indikator, HIGH);
  AnalogValue = analogRead(adcPin);

  pH = (-0.0233 * ADC) + 12.698;
  if (pH != lastReading) {
    lastReading = pH;
  }

  if (lastReading > 8.0) {
    lastReading = 0.0;
  }

  if (lastReading < 0) {
    lastReading = lastReading * -1;
  }

  Serial.print("ADC=");
  Serial.print(AnalogValue);
  Serial.print(" pH=");
  Serial.println(lastReading, 1);
  digitalWrite(DMSpin, HIGH);
  digitalWrite(indikator, LOW);

  return lastReading;
}
/* -------------------------------- FUNGSI GERAKAN SERVO ---------------------------------- */
void servoRun() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastServoMoveTime >= servoMoveInterval) {
    lastServoMoveTime = currentMillis;

    currentAngle += servoDirection;

    if (currentAngle >= 180) {
      currentAngle = 180;
      servoDirection = -1;
    } else if (currentAngle <= 0) {
      currentAngle = 0;
      servoDirection = 1;
    }

    myServo.write(currentAngle);
    Serial.println(currentAngle);
  }
}
/* --------------------------------- FUNGSI PERINGATAN PIR -------------------------------- */
void handlePIR() {
  static bool motionDetected = false;
  static int beepStep = 0;
  static unsigned long beepTimer = 0;
  const unsigned long beepDuration = 100;   // durasi ON atau OFF tiap beep
  bool pirState = digitalRead(sensorPIR) == adaGerakan;

  if (pirState && !motionDetected) {
    motionDetected = true;
    beepStep = 1;
    beepTimer = millis();
    Serial.println("DEBUG: PIR Triggered");
  }

  if (beepStep == 1 && millis() - beepTimer >= 0) {
    digitalWrite(Buzzer, LOW);  // Beep ON
    digitalWrite(ledPin, HIGH);
    beepTimer = millis();
    beepStep = 2;
  }
  else if (beepStep == 2 && millis() - beepTimer >= beepDuration) {
    digitalWrite(Buzzer, HIGH); // Beep OFF
    digitalWrite(ledPin, LOW);
    beepTimer = millis();
    beepStep = 3;
  }
  else if (beepStep == 3 && millis() - beepTimer >= beepDuration) {
    digitalWrite(Buzzer, LOW);  // Beep ON (kedua)
    digitalWrite(ledPin, HIGH);
    beepTimer = millis();
    beepStep = 4;
  }
  else if (beepStep == 4 && millis() - beepTimer >= beepDuration) {
    digitalWrite(Buzzer, HIGH); // Beep OFF
    digitalWrite(ledPin, LOW);
    beepStep = 0;
  }

  // Reset jika PIR sudah tidak mendeteksi gerakan
  if (!pirState) {
    motionDetected = false;
  }
}

void setup() {
  Serial.begin(9600);
  mySerial.begin(115200);

  lcd.init();
  lcd.backlight();
  myServo.attach(servoPin);

  pinMode(DMSpin, OUTPUT);
  pinMode(indikator, OUTPUT);
  digitalWrite(DMSpin, HIGH);
  pinMode(Buzzer, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(RelayPompa,  OUTPUT); pinMode(RelayAsam,  OUTPUT);  pinMode(RelayBasa,  OUTPUT);
  pinMode(SP_tanahPin, INPUT_PULLUP); pinMode(SP_PHPin, INPUT_PULLUP); pinMode(adcPin, INPUT_PULLUP);
  pinMode(sensorPIR, INPUT_PULLUP);
  digitalWrite(Buzzer, HIGH);
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
  handlePIR();
  /* ------------------------------------ GERAKANS SERVO ------------------------------ */
  servoRun();
  /* --------------------------------- PEMBACAAN SENSOR DMS --------------------------- */
  if (millis() - dmsLastReadTime > 1000) {
    dmsLastReadTime = millis();
    PH = DMS_readData();
  }

  /* ------------------------------ SUPER LOOP/KONTROL LOOP UTAMA ------------------------ */
  if (millis() - superloop > 500) {
    superloop = millis();

    /* ---------------------------- PEMBACAAN FUNGSI SENSOR-SENSOR ----------------------- */
    kelembabantanah = readSensor();
    SP_tanah = readSP_tanah();
    /* -------------------------- LCD DISPLAY DATA KELEMBABAN DAN PH --------------------- */
    lcd.setCursor(12, 0);
    lcd.print(kelembabantanah);
    lcd.print(" ");
    lcd.setCursor(12, 1);
    lcd.print(lastReading);
    lcd.print(" ");

    Serial.print("\t\tADC Soil Mosture: ");
    Serial.print(kelembabantanah);
    Serial.print("\t\tSP Tanah: ");
    Serial.print(SP_tanah);
    Serial.print("\t\tSP PH: ");
    Serial.println(SP_PH);

    /* ------------------------------ KONTROL POMPA TANAH BASAH --------------------------- */
    if (kelembabantanah < tanahbasah) {
      Serial.println("Status: Tanah Basah");
      digitalWrite(RelayPompa, HIGH);
      lcd.setCursor(0, 0);
      lcd.print("T.Basah ");
    }
    /* ------------------------------ KONTROL POMPA TANAH NORMAL --------------------------- */
    else if (kelembabantanah >= tanahbasah && kelembabantanah <= tanahkering) {
      Serial.println("Status: Tanah Normal");
      digitalWrite(RelayPompa, HIGH);
      lcd.setCursor(0, 0);
      lcd.print("T.Normal");
    }
    /* ------------------------------ KONTROL POMPA TANAH KERING --------------------------- */
    else if (kelembabantanah > tanahkering) {
      Serial.println("Status: Tanah Kering");
      digitalWrite(RelayPompa, LOW);
      lcd.setCursor(0, 0);
      lcd.print("T.Kering");
    }

    /* -------------------------------- KONTROL POMPA PH ASAM ------------------------------ */
    if (PH < PH_Asam) {
      Serial.println("Status: PH Asam");
      digitalWrite(RelayAsam, LOW);
      lcd.setCursor(0, 1);
      lcd.print("PH Asam  ");
    }
    /* ------------------------------- KONTROL POMPA PH NORMAL ------------------------------ */
    else if (PH >= PH_Asam && PH <= PH_Basa) {
      Serial.println("Status: PH Normal");
      digitalWrite(RelayAsam, HIGH);  digitalWrite(RelayBasa, HIGH);
      lcd.setCursor(0, 1);
      lcd.print("PH Normal");
    }
    /* -------------------------------- KONTROL POMPA PH BASA --=---------------------------- */
    else if (PH > PH_Basa) {
      Serial.println("Status: PH Basa");
      digitalWrite(RelayBasa, LOW);
      lcd.setCursor(0, 1);
      lcd.print("PH Basa  ");
    }
    Serial.println();
  }
}
