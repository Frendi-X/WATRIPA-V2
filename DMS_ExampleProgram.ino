/*****************************************
 * Compile using Arduino IDE 1.8.15
 * Board Arduino Uno/Nano/Promini/Mega2560
 * Using DMS + Probe Kilap
 *****************************************/
 
/* Syarat Pengoperasian ***************************
 * Tanah yang diukur wajib kondisi lembab dan padat
 * Pembacaan tanpa tanah adalah tidak valid
 * Minimal ujung sensor masuk pada kedalaman 8cm
***************************************************/

//input output
#define DMSpin  6     // pin output untuk DMS
#define indikator 13  // pin output led built-in untuk indikator pembacaan sensor
#define adcPin A0     // pin input sensor pH tanah

//variabel     
int AnalogValue;
float lastReading;
float pH;


void setup() {

  Serial.begin(115200);          // setting baudrate komunikasi serial
  pinMode(DMSpin, OUTPUT);
  pinMode(indikator, OUTPUT);
  digitalWrite(DMSpin, HIGH);    // non-aktifkan DMS. DMS aktif LOW
}


void loop() {
  
  digitalWrite(DMSpin, LOW);     // aktifkan DMS
  digitalWrite(indikator, HIGH); // led indikator built-in menyala
  delay(5*1000);                 // wait DMS capture data
  AnalogValue = analogRead(adcPin); 
  
  pH = (-0.0233 * ADC) + 12.698;  // ini adalah rumus regresi linier konversi adc ke pH
    if (pH != lastReading) { 
    lastReading = pH; 
    }
	
  if(lastReading > 8.0){lastReading = 0.0;}  // nol kan nilai pH saat out of range
	
  Serial.print("ADC=");
  Serial.print(AnalogValue);     // menampilkan nilai ADC di serial monitor pada baudrate 115200
  Serial.print(" pH=");
  Serial.println(lastReading,1); // menampilkan nilai pH di serial monitor pada baudrate 115200

  digitalWrite(DMSpin,HIGH);
  digitalWrite(indikator,LOW);
  delay(3*1000);                 // wait for DMS ready
}
