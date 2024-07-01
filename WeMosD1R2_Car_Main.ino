#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <iostream>
#include <string>

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

void setup() {
  Serial.begin(9600);
  pinMode(ena, OUTPUT);
  pinMode(enb, OUTPUT);
  pinMode(enaIn1,OUTPUT);
  pinMode(enaIn2,OUTPUT);
  pinMode(enbIn3,OUTPUT);
  pinMode(enbIn4,OUTPUT);

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

  if (irrecv.decode(&results)) {
    hexIr = String(results.value, HEX);

    int keyPress = getIRKeyPress(hexIr);

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

    irrecv.resume();
  }

  delay(100);
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