#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

const char* ssid = "Samsung Galaxy A54";
const char* password = "akugatau";

// Initialize Telegram BOT
String BOTtoken = "Your BOT Token";  // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
String CHAT_ID = "Your Chat ID";
bool sendPhoto = false;

WiFiClientSecure clientTCP;
UniversalTelegramBot bot(BOTtoken, clientTCP);

#define FLASH_LED_PIN 4
bool flashState = LOW;

String data;
char CharData;
String StringData, dataSubs;
int index1, index2;

int Data_Photo,
    Data_statusTanah,
    Data_nilaiPH;
String status_tanah,
       status_pompa,
       status_PH,
       nilai_PH;

boolean send_statusTanah = false,
        send_statusPH = false;


//Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

//CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22


void configInitCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  //init with high specs to pre-allocate larger buffers
  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;  //0-63 lower number means higher quality
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;  //0-63 lower number means higher quality
    config.fb_count = 1;
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    delay(1000);
    ESP.restart();
  }

  // Drop down frame size for higher initial frame rate
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_CIF);  // UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA
}


/* -------------------------------- REQ USER TELEGRAM ------------------------------- */
void handleNewMessages(int numNewMessages) {
  Serial.print("Handle New Messages: ");
  Serial.println(numNewMessages);

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }

    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);
    String from_name = bot.messages[i].from_name;

    /* -------------------------------- START REQUEST ------------------------------- */
    if (text == "/start") {
      String welcome = "Selamat Datang , " + from_name + "\n";
      welcome += "Gunakan Commands Berikut : \n";
      welcome += "/photo : Foto Objek\n";
      welcome += "/flash : Menyalakan Flash Kamera\n";
      welcome += "/cekkondisi : Mengecek Kondisi (Tanah, Pompa, PH dan Foto Objek)\n";
      bot.sendMessage(CHAT_ID, welcome, "");
    }
    /* -------------------------------- FLASH REQUEST ------------------------------- */
    if (text == "/flash") {
      flashState = !flashState;
      digitalWrite(FLASH_LED_PIN, flashState);
      Serial.println("Change flash LED state");
    }
    /* -------------------------------- PHOTO REQUEST ------------------------------- */
    if (text == "/photo") {
      sendPhoto = true;
      Serial.println("New photo request");
    }
    /* ------------------------------ CEK KONDISI REQUEST --------------------------- */
    if (text == "/cekkondisi") {
      /* ------------------- DATA REQUEST ARDUINO: 1 -> REQ KONDISI ----------------- */
      Serial2.print("#1?");
      sendPhoto = true;
      Serial.println("New photo request and Condition");
    }
  }
}

String sendPhotoTelegram() {
  const char* myDomain = "api.telegram.org";
  String getAll = "";
  String getBody = "";

  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
    return "Camera capture failed";
  }

  Serial.println("Connect to " + String(myDomain));


  if (clientTCP.connect(myDomain, 443)) {
    Serial.println("Connection successful");

    String head = "--electroniclinic\r\nContent-Disposition: form-data; name=\"chat_id\"; \r\n\r\n" + CHAT_ID + "\r\n--electroniclinic\r\nContent-Disposition: form-data; name=\"photo\"; filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String tail = "\r\n--electroniclinic--\r\n";

    uint16_t imageLen = fb->len;
    uint16_t extraLen = head.length() + tail.length();
    uint16_t totalLen = imageLen + extraLen;

    clientTCP.println("POST /bot" + BOTtoken + "/sendPhoto HTTP/1.1");
    clientTCP.println("Host: " + String(myDomain));
    clientTCP.println("Content-Length: " + String(totalLen));
    clientTCP.println("Content-Type: multipart/form-data; boundary=electroniclinic");
    clientTCP.println();
    clientTCP.print(head);

    uint8_t *fbBuf = fb->buf;
    size_t fbLen = fb->len;
    for (size_t n = 0; n < fbLen; n = n + 1024) {
      if (n + 1024 < fbLen) {
        clientTCP.write(fbBuf, 1024);
        fbBuf += 1024;
      }
      else if (fbLen % 1024 > 0) {
        size_t remainder = fbLen % 1024;
        clientTCP.write(fbBuf, remainder);
      }
    }

    clientTCP.print(tail);

    esp_camera_fb_return(fb);

    int waitTime = 10000;   // timeout 10 seconds
    long startTimer = millis();
    boolean state = false;

    while ((startTimer + waitTime) > millis()) {
      Serial.print(".");
      delay(100);
      while (clientTCP.available()) {
        char c = clientTCP.read();
        if (state == true) getBody += String(c);
        if (c == '\n') {
          if (getAll.length() == 0) state = true;
          getAll = "";
        }
        else if (c != '\r')
          getAll += String(c);
        startTimer = millis();
      }
      if (getBody.length() > 0) break;
    }
    clientTCP.stop();
    Serial.println(getBody);

  }
  else {
    getBody = "Connected to api.telegram.org failed.";
    Serial.println("Connected to api.telegram.org failed.");
  }
  return getBody;

}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  /* -------------------------------------SERIAL MONITOR ----------------------------------- */
  Serial.begin(115200);
  /* -------------------------------- SERIAL KOMUNIKASI DATA ------------------------------- */
  Serial2.begin(115200, SERIAL_8N1, 14, 15);

  /* -------------------------------------- FLASH OUTPUT ----------------------------------- */
  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, flashState);
  configInitCamera();

  /* ------------------------------------ CONNECT WIFI AP ---------------------------------- */
  WiFi.mode(WIFI_STA);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  clientTCP.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("ESP32-CAM IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {

  /* ---------------------------------- BACA DATA DARI ARDUINO ------------------------------ */
  while (Serial2.available() > 0)
  {
    //delay(10);
    CharData = Serial2.read();
    StringData += CharData;

    /* ------------- PARSING DATA MASUK DARI ARDUINO -> PENGIRIMAN FOTO OTOMATIS ------------ */
    if (StringData.length() > 0 && CharData == '?')
    {
      index1 = StringData.indexOf('#');
      index2 = StringData.indexOf('?', index1 + 1);
      dataSubs = StringData.substring(index1 + 1, index2);
      StringData = "";

      //Mengubah Data String ke Float
      char buf[dataSubs.length()];
      dataSubs.toCharArray(buf, dataSubs.length() + 1);
      float Data11 = atof(buf);

      /* ----------------------------- DATA MASUK DARI ARDUINO (FOTO) ----------------------- */
      Data_Photo = Data11;
      Serial.print("Data Photo : ");
      Serial.println(Data_Photo);

      /* ------------------- DATA 1 -> TANAH KERING MNGIRIMKAN FOTO OTOMATIS ---------------- */
      if (Data_Photo == 1) {
        sendPhoto = true;
        Serial.println("New photo request");
      }
      /* ---------------------- DATA 2 -> PH BASA MNGIRIMKAN FOTO OTOMATIS ------------------ */
      else if (Data_Photo == 2) {
        sendPhoto = true;
        Serial.println("New photo request");
      }
    }

    /* --------- PARSING DATA MASUK DARI ARDUINO -> DATA STATUS TANAH DAN STATUS PH --------- */
    else if (StringData.length() > 0 && CharData == '!')
    {
      index1 = StringData.indexOf('#');
      index2 = StringData.indexOf('!', index1 + 1);
      dataSubs = StringData.substring(index1 + 1, index2);
      StringData = "";

      //Mengubah Data String ke Float
      char buf[dataSubs.length()];
      dataSubs.toCharArray(buf, dataSubs.length() + 1);
      float Data11 = atof(buf);

      /* ------------------------ DATA MASUK DARI ARDUINO (STATUS TANAH) --------------------- */
      Data_statusTanah = Data11;
      Serial.print("Status Tanah : ");
      Serial.println(Data_statusTanah);

      /* -------------------------------- 1 -> STATUS TANAH BASAH ---------------------------- */
      if (Data_statusTanah == 1) {
        status_tanah = "Basah";
        status_pompa = "OFF";
      }
      /* -------------------------------- 2 -> STATUS TANAH NORMAL --------------------------- */
      else if (Data_statusTanah == 2) {
        status_tanah = "Normal";
        status_pompa = "OFF";
      }
      /* -------------------------------- 3 -> STATUS TANAH KERING --------------------------- */
      else if (Data_statusTanah == 3) {
        status_tanah = "Kering";
        status_pompa = "ON";
      }

      /* ---------------------------------- 11 -> STATUS PH ASAM ----------------------------- */
      if (Data_statusTanah == 11) {
        status_PH = "Status PH : Asam\n";
      }
      /* ---------------------------------- 22 -> STATUS PH NORMAL --------------------------- */
      else if (Data_statusTanah == 22) {
        status_PH = "Status PH : Normal\n";
      }
      /* ---------------------------------- 33 -> STATUS PH BASA ----------------------------- */
      else if (Data_statusTanah == 33) {
        status_PH = "Status PH : Basa\n";
      }
      send_statusTanah = true;
    }

    /* ----------------------- PARSING DATA MASUK DARI ARDUINO -> NILAI PH ------------------- */
    else if (StringData.length() > 0 && CharData == '@')
    {
      index1 = StringData.indexOf('#');
      index2 = StringData.indexOf('@', index1 + 1);
      dataSubs = StringData.substring(index1 + 1, index2);
      StringData = "";

      //Mengubah Data String ke Float
      char buf[dataSubs.length()];
      dataSubs.toCharArray(buf, dataSubs.length() + 1);
      float Data111 = atof(buf);

      /* -------------------------- DATA MASUK DARI ARDUINO (NILAI PH) ------------------------ */
      Data_nilaiPH = Data111;
      Serial.print("Nilai PH : ");
      Serial.println(Data_nilaiPH);

      /* ------------------------------------- DATA NILAI PH ---------------------------------- */
      nilai_PH = "Nilai PH : " + String(Data_nilaiPH);
      send_statusPH = true;

      /* ------------------------ PESAN TELEGRAM STATUS TANAH DAN PH LENGKAP ------------------ */
      String texttt;
      texttt = "Pengecekan Kondisi : \n";
      texttt += "Status Tanah : " + String(status_tanah) + "\n";
      texttt += "Pompa : " + String(status_pompa) + "\n";
      texttt += status_PH;
      texttt += nilai_PH;
      bot.sendMessage(CHAT_ID, texttt, "");
    }
  }

  /* ------------------------------------ REQUEST FOTO TELEGRAM ------------------------------- */
  if (sendPhoto) {
    Serial.println("Preparing photo");
    sendPhotoTelegram();
    sendPhoto = false;
  }

  /* -------------------------- MILLIS PEMBACAAN REQ USER DARI TELEGRAM ----------------------- */
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}
