#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <iostream>
#include <string>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include "secret.h"

using namespace std;

#define enbIn3 D3
#define enbIn4 D2
#define enaIn1 D7
#define enaIn2 D8
#define ena D6
#define enb D5
#define IR_RECEIVE_PIN D1

IRrecv irrecv(IR_RECEIVE_PIN);
decode_results results;

String hexIr;
float distanceObject;
int speed = 100;
String speedChar = "100";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

int port = MQTT_PORT;
const char* mqtt_server = MQTT_HOST;
const char* topic = MQTT_TOPIC;
String message;

const char* wifiList[][2] = {
  {WIFI_SSID_1, WIFI_PASSWORD_1},
  {WIFI_SSID_2, WIFI_PASSWORD_2},
  {WIFI_SSID_3, WIFI_PASSWORD_3}
};

int wifiTimeout = 20;
int wifiKnownIndex = 0;

void setup() {
  Serial.begin(9600);
  
  pinMode(ena, OUTPUT);
  pinMode(enb, OUTPUT);
  pinMode(enaIn1,OUTPUT);
  pinMode(enaIn2,OUTPUT);
  pinMode(enbIn3,OUTPUT);
  pinMode(enbIn4,OUTPUT);

  connectToStrongestWifi();

  client.setServer(mqtt_server, port);
  client.setCallback(callback);

  irrecv.enableIRIn();  // Start the receiver
  while (!Serial)  // Wait for the serial connection to be establised.
    delay(50);
  Serial.println();
  Serial.print("IRrecvDemo is now running and waiting for IR message on Pin ");
  Serial.println(IR_RECEIVE_PIN);
}

void loop() { 
  analogWrite(ena, speed);
  analogWrite(enb, speed);

  // Check internet connectivity
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Koneksi WiFi terputus, mencoba terhubung kembali...");
    // attempt to connect to WiFi network:
    connectToStrongestWifi();
  } else {
    if (!client.connected()) {
      reconnect();
    }

    client.loop();
  }
 
  if (irrecv.decode(&results)) {
    hexIr = String(results.value, HEX);

    int keyPress = getIRKeyPress(hexIr);
    Serial.println(keyPress);

    if (keyPress == 1) {
      showdistanceObjectNow();
    } else if (keyPress == 2) {
      changeSpeed();
    } else if (keyPress == 3) {
      getCurrentTireSpeed();
    } else if (keyPress == 50) {
      rotate_ena();
    } else if (keyPress == 51) {
      rotate_ena_backward();
    } else if (keyPress == 100) {
      tireStop();
    } else if (keyPress == 52) {
      rotate_ena_turn_left();
    } else if (keyPress == 53) {
      rotate_ena_turn_right();
    }
  }

  irrecv.resume();

  delay(100);
}

void callback(char* topic, unsigned char* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  speedChar = "";

  for (int i = 1; i < length; i++) {
    speedChar += (char)payload[i];
  }

  Serial.print("Changing speed to: ");
  Serial.println(speedChar);

  speed = speedChar.toInt();

  if ((char)payload[0] == 'U') {
    Serial.println("Move Foward");
    rotate_ena();
  } else if ((char)payload[0] == 'D') {
    Serial.println("Move Backward");
    rotate_ena_backward();
  } else if ((char)payload[0] == 'L') {
    Serial.println("Move Left");
    rotate_ena_backward();
  } else if ((char)payload[0] == 'R') {
    Serial.println("Move Right");
    rotate_ena_turn_right();
  } else if ((char)payload[0] == 'S') {
    Serial.println("Car Stop");
    tireStop();
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    Serial.print(" to : ");
    Serial.print(topic);
    Serial.print(" on port: ");
    Serial.print(port);
    Serial.print(" with id: ");
    Serial.print(clientId);

    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(topic, "hello world");
      // ... and resubscribe
      client.subscribe(topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void connectToStrongestWifi() {
  Serial.println("Scanning for Wifi!");

  // Scan for nearby networks
  int n = WiFi.scanNetworks();
  Serial.print("Connections found: ");
  Serial.println(n);

  // Variables to keep track of the strongest network
  int strongestSignal = -100;
  String strongestSSID;
  String strongestPass;

  Serial.print("Trying wifi index ");
  Serial.print(wifiKnownIndex);
  Serial.print(" / ");
  Serial.println((sizeof(wifiList) / sizeof(wifiList[0])));

  // Go through each found network
  for (int i = 0; i < n; ++i) {
    String ssid_scan = WiFi.SSID(i);
    int32_t rssi_scan = WiFi.RSSI(i);

    if (wifiKnownIndex == (sizeof(wifiList) / sizeof(wifiList[0]))) {
      Serial.println("Restart from strongest index 0");
      wifiKnownIndex = 0;
    }

    strongestSignal = rssi_scan;
    strongestSSID = wifiList[wifiKnownIndex][0];
    strongestPass = wifiList[wifiKnownIndex][1];

  }

  int secondsPassed = 0;
  // If a known network with strong signal was found
  if (strongestSignal != -100) {
    Serial.print("Connecting to: ");
    Serial.println(strongestSSID.c_str());
    WiFi.begin(strongestSSID.c_str(), strongestPass.c_str());

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.print(".");
      secondsPassed++;

      if (secondsPassed == wifiTimeout) {
        Serial.println("");
        Serial.print("Failed to connect to: ");
        Serial.println(strongestSSID);

        wifiKnownIndex++;
        break;
      }

      if (WiFi.status() == WL_CONNECTED){
        Serial.println("");
        Serial.print("Connected to: ");
        Serial.println(strongestSSID);
      }
    }

  } else {
    Serial.println("Failed to connect to any known networks.");
  }
}

void rotate_ena(){
  digitalWrite(enaIn1, HIGH);
  digitalWrite(enaIn2, LOW);
  digitalWrite(enbIn3, LOW);
  digitalWrite(enbIn4, HIGH);
}

void rotate_ena_backward(){
  digitalWrite(enaIn1, LOW);
  digitalWrite(enaIn2, HIGH);
  digitalWrite(enbIn3, HIGH);
  digitalWrite(enbIn4, LOW);
}

void rotate_ena_turn_right(){
  digitalWrite(enaIn1, LOW);
  digitalWrite(enaIn2, HIGH);
  digitalWrite(enbIn3, LOW);
  digitalWrite(enbIn4, HIGH);
}

void rotate_ena_turn_left(){
  digitalWrite(enaIn1, HIGH);
  digitalWrite(enaIn2, LOW);
  digitalWrite(enbIn3, HIGH);
  digitalWrite(enbIn4, LOW);
}

void tireStop(){
  digitalWrite(enaIn1, LOW);
  digitalWrite(enaIn2, LOW);
  digitalWrite(enbIn3, LOW);
  digitalWrite(enbIn4, LOW);
}

void showdistanceObjectNow(){
  Serial.print("distanceObject between nearest object is ");
  Serial.print(distanceObject);
  Serial.println(" cm");
}

void changeSpeed(){
  Serial.print("Changing tire speed from ");
  Serial.print(speed);
  Serial.print(" to ");

  String newSpeed;
  int isDone = 0;

  irrecv.resume();
  delay(500);

  while (isDone == 0){

    if (irrecv.decode(&results)) {
      hexIr = String(results.value, HEX);

      int keyPress = getIRKeyPress(hexIr);

      if(keyPress == 100) {
        isDone = 1;
        break;
      }

      newSpeed += String(keyPress);
      Serial.println(newSpeed);
      irrecv.resume();
      delay(500);
    }

    delay(500);
  }

  speed = newSpeed.toInt();
  Serial.println(speed);
}

void getCurrentTireSpeed(){
  Serial.print("Current tire speed is ");
  Serial.println(speed);
}

int getIRKeyPress(String hexIr){
  int keyPress = 99;

  if(hexIr == "ffa25d"){
      keyPress = 1;
    } else if (hexIr == "ff629d"){
      keyPress = 2;
    } else if (hexIr == "ffe21d"){
      keyPress = 3;
    } else if (hexIr == "ff22dd"){
      keyPress = 4;
    } else if (hexIr == "ff02fd"){
      keyPress = 5;
    } else if (hexIr == "ffc23d"){
      keyPress = 6;
    } else if (hexIr == "ffe01f"){
      keyPress = 7;
    } else if (hexIr == "ffa857"){
      keyPress = 8;
    } else if (hexIr == "ff906f"){
      keyPress = 9;
    } else if (hexIr == "ff6897"){
    } else if (hexIr == "ffb04f"){
    } else if (hexIr == "ff38c7"){
      keyPress = 100;
    } else if (hexIr == "ff18e7"){
      keyPress = 50;
    } else if (hexIr == "ff4ab5"){
      keyPress = 51;
    } else if (hexIr == "ff5aa5"){
      keyPress = 53;
    } else if (hexIr == "ff10ef"){
      keyPress = 52;
    } else if (hexIr == "ff9867"){
      keyPress = 0;
    }


  return keyPress;
}