# Pengusir-Hama-ESP32-CAM-dengan-Telegram (WATRIPA V2)

![alt text](https://github.com/Frendi-X/WATRIPA-V2/blob/main/Project.png?raw=true)

Proyek ini adalah sebuah sistem pemantauan kondisi tanah dan kualitas air yang menggunakan ESP32-CAM yang terhubung ke Telegram Bot. Sistem ini dapat mengirimkan data terkait status tanah (kering, normal, basah), status pompa, nilai pH tanah, serta foto objek (misalnya tanaman atau area tanah) kepada pengguna melalui pesan Telegram. Sistem ini juga memungkinkan pengguna untuk mengontrol beberapa aspek melalui perintah yang dikirimkan ke bot Telegram, seperti menyalakan flash kamera dan meminta foto.

# 🌱 Smart Soil & pH Monitoring System with Arduino

Proyek ini adalah **Sistem monitoring dan kontrol kelembaban tanah serta pH tanah berbasis Arduino** yang terintegrasi dengan **ESP32-CAM**, **LCD**, **relay**, **sensor PIR**, dan **servo**. Sistem ini ditujukan untuk aplikasi pertanian cerdas (smart farming) yang mampu **mengontrol pompa otomatis** berdasarkan kelembaban dan pH tanah.

---

## 🔧 Komponen yang Digunakan

- Arduino (Uno/Nano)
- LCD I2C 16x2
- DFPlayer Mini (opsional via `SoftwareSerial`)
- Sensor Soil Moisture
- Sensor PIR
- Potensio (setpoint tanah & pH)
- Servo motor
- 3 buah relay: Pompa, Asam, dan Basa
- Buzzer & LED indikator
- ESP32-CAM (komunikasi serial)

---

## ⚙️ Fitur Utama

- 📊 **Pembacaan Kelembaban Tanah**  
  Menggunakan sensor soil moisture analog. Nilai kelembaban dibandingkan dengan *setpoint tanah* untuk klasifikasi basah, normal, atau kering.

- ⚗️ **Pembacaan pH Tanah via DMS (Digital Measurement System)**  
  Menggunakan sensor analog dan mapping ADC untuk mendeteksi status pH: Asam, Normal, atau Basa.

- 🔁 **Kontrol Otomatis Pompa**  
  - Pompa Air aktif jika tanah terlalu kering.
  - Pompa Asam atau Basa aktif sesuai kondisi pH.

- 📟 **Tampilan LCD**  
  Menampilkan nilai kelembaban dan pH secara real-time, serta status sistem.

- 🧠 **Komunikasi Serial dengan ESP32-CAM**  
  Mengirim status kelembaban dan pH secara serial ke ESP32-CAM dalam format khusus (`#data!`).

- 🛑 **Deteksi Gerakan (Sensor PIR)**  
  Jika terdeteksi gerakan, sistem akan menyalakan buzzer dan LED serta menjalankan animasi servo.

---

## 📶 Format Komunikasi Serial
### Dari Arduino ke ESP32-CAM:
#1! → Tanah Basah
#2! → Tanah Normal
#3! → Tanah Kering
#11! → pH Asam
#22! → pH Normal
#33! → pH Basa
#x.yz@ → Nilai pH aktual

## 🧪 Kalibrasi

Saat startup, sistem melakukan kalibrasi 4 parameter penting melalui potensio:
1. Tanah Basah (`tanahbasah`)
2. Tanah Kering (`tanahkering`)
3. pH Asam (`PH_Asam`)
4. pH Basa (`PH_Basa`)

Masing-masing dibaca 10x dan ditampilkan di LCD.

---

## 📁 Struktur Kode

- `setup()`: Inisialisasi pin, kalibrasi awal, dan LCD.
- `loop()`:  
  - Pembacaan soil & pH sensor  
  - Deteksi PIR  
  - Serial parsing & pengiriman status ke ESP32  
  - Kontrol pompa berdasarkan kondisi  
  - Display status di LCD  

---

## 📌 Catatan

- Pastikan sensor pH analog menghasilkan rentang ADC sesuai logika `mapFloat()`:
  - 700–850 → pH 6.0–8.0 (normal)
  - 450–479 → pH 8.1–12.0 (basa)
  - 480–500 → pH 2.0–5.9 (asam)
- Gunakan sumber daya eksternal jika mengendalikan banyak relay & motor.

---

## 👨‍🌾 Pengembangan Selanjutnya

- Integrasi konektivitas IoT (MQTT / Blynk)
- Logging data ke SD Card / Firebase
- Tambahkan sistem penyiraman otomatis berbasis jadwal

---

## Contacs us : 
* [Frendi RoboTech](https://www.instagram.com/frendi.co/)
* [Whatsapp : +6287888227410](https://wa.me/+6287888227410)
* [Email    : frendirobotech@gmail.com](https://mail.google.com/mail/u/0/?view=cm&tf=1&fs=1&to=frendirobotech@gmail.com) atau [Email    : frendix45@gmail.com](https://mail.google.com/mail/u/0/?view=cm&tf=1&fs=1&to=frendix45@gmail.com)

---

## 👨‍💻 Author

Dikembangkan oleh: [Reog Robotic & Robotech Electronics]  
Lisensi: Open Source (MIT)




