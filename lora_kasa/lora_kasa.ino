#include <stdio.h>
#include <SoftwareSerial.h>
#include <Wire.h>

#define LoRa_RX_PIN     11  // Arduino D11 to LoRa module TX
#define LoRa_TX_PIN     12  // Arduino D12 to LoRa module RX

#define INDEX_LED_PIN   13  // Arduino D13 to INDEX LED (Green)

#define SERIAL_WAIT_TIME     5000
#define LoRa_SEND_INTERVAL  3000    // LoRa送信間隔 (ミリ秒)

#define LoRa_fPort_TEMP  12        // port 12 = 温度湿度気圧等

SoftwareSerial LoRa(LoRa_RX_PIN, LoRa_TX_PIN);

float bme_temp;
float bme_humi;
float bme_press;

unsigned long beforetime = 0L;

void setup() {
  // for LED
  pinMode(INDEX_LED_PIN, OUTPUT);

  digitalWrite(INDEX_LED_PIN, HIGH);

  // for console serial
  Serial.begin(9600); 
  Serial.println("LoRa TEMP/HUMI/PRESS Send for KiwiTech");

  // for LoRa module serial
  LoRa.begin(9600);
  LoRa.setTimeout(SERIAL_WAIT_TIME);


  // send initialize command to LoRa
  initLoRa(SERIAL_WAIT_TIME);

  // beforetime = -(LoRa_SEND_INTERVAL);
  beforetime = millis();
}

void loop() {
  if (millis() - beforetime > LoRa_SEND_INTERVAL) {

    sendTemp();

    beforetime = millis();
  }
}

//
// void initLoRa(int waitTime)
//    send command strings to initialize/ready LoRa module
//        waitTime - waiting timeout for command result
//
void initLoRa(int waitTime)
{
  // activate LoRa serial
  LoRa.listen();

  // send dummy enter
  sendCmd("\n", false, "", "", waitTime);
  delay(SERIAL_WAIT_TIME);

  // print any response data before send
  while (LoRa.available() > 0) {
    char ch = LoRa.read();
    Serial.print(ch);
  }

  //
  // LoRa module status clear
  //
  if (!sendCmd("mod reset", true, "", "", waitTime)) {
    Serial.println("Request Failed");
  }
  if (!sendCmd("mod set_echo off", false, "Ok", "", waitTime)) {
    Serial.println("Request Failed");
  }

  //
  // LoRa module various value get
  //
  if (!sendCmd("mod get_hw_model", true, "", "", waitTime)) {
    Serial.println("Request Failed");
  }
  if (!sendCmd("mod get_ver", true, "", "", waitTime)) {
    Serial.println("Request Failed");
  }
  if (!sendCmd("lorawan get_deveui", true, "", "", waitTime)) {
    Serial.println("Request Failed");
  }

  //
  // LoRa module join to Network Server by OTAA
  //
  if (!sendCmd("lorawan get_join_status", true, "joined", "", waitTime)) {
    while (!sendCmd("lorawan join otaa", true, "accepted", "", waitTime * 2)) {
    Serial.println(F("Request Failed"));
      // forever loop until join success
    }
  }
}

//
// bool sendCmd(String cmd, bool echo, String waitStr, String waitStr2, int waitTime)
//    send command string to LoRa module and wait result
//        cmd - command string to send LoRa module
//        echo - command local echo back on/off
//        waitStr, waitStr2 - waiting either strings expected for command result
//        waitTime - waiting timeout for command result
//
bool sendCmd(String cmd, bool echo, String waitStr, String waitStr2, int waitTime)
{
  unsigned long tim;
  String str;
  int in_byte;
  
  // command log
  if (echo) {
    // Serial.print(">");
    Serial.print(cmd);
  }

  // activate LoRa serial
  LoRa.listen();
  LoRa.flush();

  // serial command send
  LoRa.print(cmd);
  LoRa.print('\r');
  
  delay(100);
  tim = millis() + waitTime - 100;

  while (millis() < tim) {
    if (LoRa.available() > 0) {
      char ch = LoRa.read();
      Serial.print(ch);
      str += String(ch);
      if (str.indexOf("\r> ") >= 0) break;
    }
  }
  if (waitStr == NULL) return true;
  if (str.indexOf(waitStr) >= 0 || ( waitStr2 != "" && str.indexOf(waitStr2) >= 0)) return true;
  
  return false;
}

//
// 送信処理>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
bool sendTemp()
{
  char cmdline[64];
  short port = LoRa_fPort_TEMP;    // port 12 = Temp

  unsigned long temp = 100;
  unsigned long humi = 100;
  unsigned long press = 100;

  sprintf(cmdline, "lorawan tx ucnf %d %04x%04x%06lx", port, temp, humi, press);
  if (!sendCmd(cmdline, true, "tx_ok", "rx", SERIAL_WAIT_TIME)) {
    Serial.println("Request Failed");
    return false;
  }
  return true;
}

