// #include <HCSR04.h>
// #include <IRremote.hpp>

#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <iostream>
#include <string>
using namespace std;

// #define led1 D2
// #define led2 D7
#define enbIn3 D3
#define enbIn4 D2
// #define echoPin D6
// #define trigPin D5
// #define buzzer D8
#define enaIn1 D7
#define enaIn2 D8
#define ena D6
#define enb D5
#define IR_RECEIVE_PIN D1

IRrecv irrecv(IR_RECEIVE_PIN);
decode_results results;

String hexIr;
// HCSR04 hc(trigPin, echoPin);
float distanceObject;
int speed = 100;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  // pinMode(led1, OUTPUT);
  // pinMode(led2, OUTPUT);
  // pinMode(buzzer,OUTPUT);
  pinMode(ena, OUTPUT);
  pinMode(enb, OUTPUT);
  pinMode(enaIn1,OUTPUT);
  pinMode(enaIn2,OUTPUT);
  pinMode(enbIn3,OUTPUT);
  pinMode(enbIn4,OUTPUT);
  // IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK); // Start the receiver

  irrecv.enableIRIn();  // Start the receiver
  while (!Serial)  // Wait for the serial connection to be establised.
    delay(50);
  Serial.println();
  Serial.print("IRrecvDemo is now running and waiting for IR message on Pin ");
  Serial.println(IR_RECEIVE_PIN);
}

void loop() {
  // put your main code here, to run repeatedly:
  // Serial.println("Program loop");
  
  analogWrite(ena, speed);
  analogWrite(enb, speed);

  // if (IrReceiver.decode()) {
  if (irrecv.decode(&results)) {
    // hexIr = String(IrReceiver.decodedIRData.decodedRawData, HEX);
    // Serial.println("ada klil");

    hexIr = String(results.value, HEX);
    Serial.println(hexIr);

    // print() & println() can't handle printing long longs. (uint64_t)
    // serialPrintUint64(results.value, HEX);
    // Serial.println("");
    // irrecv.resume();  // Receive the next value
    
    // if(hexIr == "ba45ff00"){

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
    // IrReceiver.resume(); // Enable receiving of the next value
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

      // Serial.print("Temporary new Speed: ");
      Serial.println(newSpeed);
      irrecv.resume();
      delay(500);
    // Serial.print(newSpeed);
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
      // Serial.println("Key 1 pressed");
      keyPress = 1;
    // } else if (hexIr == "b946ff00"){
    } else if (hexIr == "ff629d"){
      // Serial.println("Key 2 pressed");
      keyPress = 2;
    // } else if (hexIr == "b847ff00"){
    } else if (hexIr == "ffe21d"){
      // Serial.println("Key 3 pressed");
      keyPress = 3;
    // } else if (hexIr == "bb44ff00"){
    } else if (hexIr == "ff22dd"){
      // Serial.println("Key 4 pressed");
      keyPress = 4;
    // } else if (hexIr == "bf40ff00"){
    } else if (hexIr == "ff02fd"){
      // Serial.println("Key 5 pressed");
      keyPress = 5;
    // } else if (hexIr == "bc43ff00"){
    } else if (hexIr == "ffc23d"){
      // Serial.println("Key 6 pressed");
      keyPress = 6;
    // } else if (hexIr == "f807ff00"){
    } else if (hexIr == "ffe01f"){
      // Serial.println("Key 7 pressed");
      keyPress = 7;
    // } else if (hexIr == "ea15ff00"){
    } else if (hexIr == "ffa857"){
      // Serial.println("Key 8 pressed");
      keyPress = 8;
    // } else if (hexIr == "f609ff00"){
    } else if (hexIr == "ff906f"){
      // Serial.println("Key 9 pressed");
      keyPress = 9;
    // } else if (hexIr == "e916ff00"){
    } else if (hexIr == "ff6897"){
      // Serial.println("Key (*) pressed");
    // } else if (hexIr == "f20dff00"){
    } else if (hexIr == "ffb04f"){
      // Serial.println("Key (#) pressed");
    // } else if (hexIr == "e31cff00"){
    } else if (hexIr == "ff38c7"){
      // Serial.println("Key (ok) pressed");
      keyPress = 100;
    // } else if (hexIr == "e718ff00"){
    } else if (hexIr == "ff18e7"){
      // Serial.println("Key ArrowUp pressed");
      keyPress = 50;
    // } else if (hexIr == "ad52ff00"){
    } else if (hexIr == "ff4ab5"){
      // Serial.println("Key ArrowDown pressed");
      keyPress = 51;
    // } else if (hexIr == "a55aff00"){
    } else if (hexIr == "ff5aa5"){
      // Serial.println("Key ArrowRight pressed");
      keyPress = 53;
    // } else if (hexIr == "f708ff00"){
    } else if (hexIr == "ff10ef"){
      // Serial.println("Key ArrowLeft pressed");
      keyPress = 52;
    // } else if (hexIr == "e619ff00"){
    } else if (hexIr == "ff9867"){
      // Serial.println("Key 0 pressed");
      keyPress = 0;
    }


  return keyPress;
}

// void slow() {
//   // digitalWrite(led1, HIGH);
//   // digitalWrite(led2, HIGH);
//   // tone(buzzer, 500);

//   delay(1000);

//   // digitalWrite(led1, LOW);
//   // digitalWrite(led2, LOW);
//   // noTone(buzzer);

//   delay(1000);
// }

// void medium() {
//   // digitalWrite(led1, HIGH);
//   // digitalWrite(led2, HIGH);
//   // tone(buzzer, 500);

//   delay(500);

//   // digitalWrite(led1, LOW);
//   // digitalWrite(led2, LOW);
//   // noTone(buzzer);

//   delay(500);
// }

// void fast() {
//   // digitalWrite(led1, HIGH);
//   // digitalWrite(led2, HIGH);
//   // tone(buzzer, 500);

//   delay(100);

//   // digitalWrite(led1, LOW);
//   // digitalWrite(led2, LOW);
//   // noTone(buzzer);

//   delay(100);
// }