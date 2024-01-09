#include <Wire.h>
#include "Adafruit_MPR121.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <FirebaseESP8266.h>
#include <WiFiClient.h>

#define FIREBASE_AUTH "AIzaSyAhni0WPARhPKstzzpknvk5DCnOmLSNXYo"
#define FIREBASE_HOST "fir-iotproject-f2faa-default-rtdb.firebaseio.com"
FirebaseData firebaseData;
String path = "/";
FirebaseJson json;

#ifndef _BV
#define _BV(bit) (1 << (bit))
#endif


char ssid[] = "QUALITY1";
char pass[] = "control1234";
String GAS_ID = "AKfycbyt3A-xMfyj5wwWiLbhZvrUrKmoNr2wtEoAPKlNgHULBsxQDA5IeQF43yDGqV-hQus";  //--> spreadsheet script ID
const char* host = "script.google.com";                                                     // only google.com not https://google.com
WiFiClientSecure client;


int a = 0, b = 0, c = 1;
int note = 0;
int loa = 0, cb;

SoftwareSerial mySoftwareSerial(D5, D7);  // D5 NỐI TX, D7 NỐI RX
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);

Adafruit_MPR121 cap = Adafruit_MPR121();


uint16_t lasttouched = 0;
uint16_t currtouched = 0;

const int numRows = 3;
const int numCols = 4;
int touchMatrix[numRows][numCols];

const String channelNames[12] = {
  "0",
  "1",
  "2",
  "3",
  "4",
  "5",
  "6",
  "7",
  "8",
  "9",
  "10",
  "11"
};

void update_google_sheet(String a, String b) {
  Serial.print("connecting to ");
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClientSecure client;
  const int httpPort = 443;  // 80 is for HTTP / 443 is for HTTPS!

  client.setInsecure();  // this is the magical line that makes everything work

  if (!client.connect(host, httpPort)) {  //works!
    Serial.println("connection failed");
    return;
  }

  String url = "/macros/s/" + GAS_ID + "/exec?dangoi=";

  url += a;

  url += "&timengoi=";
  url += b;

  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");

  Serial.println();
  Serial.println("closing connection");
}

void setup() {
  Serial.begin(9600);
  Serial.print("Connecting");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }


  client.setInsecure();
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  if (!Firebase.beginStream(firebaseData, path)) {
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println();
  }
  mySoftwareSerial.begin(9600);
  while (!Serial) {  // needed to keep leonardo/micro from starting too fast!
    delay(10);
  }

  Serial.println("Adafruit MPR121 Capacitive Touch sensor test");

  if (!cap.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring?");
    while (1)
      ;
  }
  Serial.println("MPR121 found!");

  for (int row = 0; row < numRows; row++) {
    for (int col = 0; col < numCols; col++) {
      touchMatrix[row][col] = 0;
    }
  }
  if (!myDFPlayer.begin(mySoftwareSerial)) {
    Serial.println(F("Không thể khởi động:"));
    Serial.println(F("1.Kiểm tra lại kết nối"));
    Serial.println(F("2.Lắp lại thẻ nhớ"));
    while (true) {
      delay(0);
    }
  }
  Serial.println(F("DFPlayer Mini đang hoạt động"));
  myDFPlayer.volume(35);  //cài đặt volume từ 0 đến 30
  myDFPlayer.play(3);
  delay(3000);
}

void loop() {

  // Get the currently touched pads
  currtouched = cap.touched();

  for (uint8_t i = 0; i < 12; i++) {
    // Update touch matrix based on touch events
    if ((currtouched & _BV(i)) && !(lasttouched & _BV(i))) {
      int row = i / numCols;
      int col = i % numCols;
      touchMatrix[row][col] = 1;
    } else if (!(currtouched & _BV(i)) && (lasttouched & _BV(i))) {
      int row = i / numCols;
      int col = i % numCols;
      touchMatrix[row][col] = 0;
    }
  }
  bool anyChannelTouched = false;
  int numChannelsTouched = 0;  // Count of touched channels
  for (int row = 0; row < numRows; row++) {
    for (int col = 0; col < numCols; col++) {
      if (touchMatrix[row][col] == 1) {
        numChannelsTouched++;
        anyChannelTouched = true;
      }
    }
  }

  if (numChannelsTouched >= 2) {
    Firebase.setInt(firebaseData, path + "Pillow/SittingPillow/IsOn", 1);  // gửi trạng thái có người ngồi về firebase
    if (Firebase.getInt(firebaseData, path + "Pillow/SittingPillow/SoundOn")) loa = firebaseData.intData();
    if (a == 0) {
      Serial.println("Có người ngồi");


      if (loa == 1) {
        myDFPlayer.play(1);
        delay(2000);
      }

      update_google_sheet("goi_ngoi", "co_nguoi_ngoi");
      a = 1;
    }
    if (Firebase.getInt(firebaseData, path + "Pillow/SittingPillow/Warning")) cb = firebaseData.intData();
    if (cb == 1) {
      if (b == 0) {
        if (loa == 1) {
          myDFPlayer.play(2);
          delay(8000);
          b = 1;
        }
      }
    }
  }

  if (numChannelsTouched < 2) {
    if (a == 1) {
      update_google_sheet("goi_ngoi", "khong_co_nguoi_ngoi");
      if (Firebase.getInt(firebaseData, path + "Pillow/SittingPillow/SoundOn")) loa = firebaseData.intData();
      a = 0;
      b = 0;
    }
    Firebase.setInt(firebaseData, path + "Pillow/SittingPillow/IsOn", 0);  // gửi trạng thái không người ngồi về firebase

    Serial.println("Không người ngồi");
  }
  if (Firebase.getInt(firebaseData, path + "Pillow/SittingPillow/Notify")) note = firebaseData.intData();
  if (loa == 1) {
    if (note == 1) {
      Serial.println("Sáng nay đã đến giờ nghỉ ngơi rồi");
      myDFPlayer.play(4);
      delay(8000);
      Firebase.setInt(firebaseData, path + "Pillow/SittingPillow/Notify", 0);
    }
    if (note == 2) {
      Serial.println("Đã đến giờ nghỉ ngơi cho buổi tối");
      myDFPlayer.play(5);
      delay(8000);
      Firebase.setInt(firebaseData, path + "Pillow/SittingPillow/Notify", 0);
    }
  }
  if(loa == 0) {
          Firebase.setInt(firebaseData, path + "Pillow/SittingPillow/Notify", 0);
  }
  lasttouched = currtouched;
}
