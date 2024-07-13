#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <iostream>
#include <string>
#include <SoftwareSerial.h>
#include <MqttClient.h>

// Enable MqttClient logs
#define MQTT_LOG_ENABLED 0

// #define LOG_PRINTFLN(fmt, ...)	printfln_P(PSTR(fmt), ##__VA_ARGS__)
// #define LOG_SIZE_MAX 128

// void printfln_P(const char *fmt, ...) {
// 	char buf[LOG_SIZE_MAX];
// 	va_list ap;
// 	va_start(ap, fmt);
// 	vsnprintf_P(buf, LOG_SIZE_MAX, fmt, ap);
// 	va_end(ap);
// 	Serial.println(buf);
// }

#define HW_UART_SPEED									9600L
#define MQTT_ID											"TEST-ID"
const char* MQTT_TOPIC_SUB = "orewana";
const char* MQTT_TOPIC_PUB = "orewana";
MqttClient *mqtt = NULL;

// ============== Object to supply system functions ================================
class System: public MqttClient::System {
public:
	unsigned long millis() const {
		return ::millis();
	}
};

// ============== Object to implement network connectivity =====================
// Current example assumes the network TCP stack is connected using serial
// interface to pins 10(RX) and 11(TX). The SoftwareSerial library is used
// for actual communication.
#define SW_UART_PIN_RX								10
#define SW_UART_PIN_TX								11
#define SW_UART_SPEED								9600L

class Network {

public:
	Network() {
		mNet = new SoftwareSerial(SW_UART_PIN_RX, SW_UART_PIN_TX);
		mNet->begin(SW_UART_SPEED);
	}

	int connect(const char* hostname, int port) {
		// TCP connection is already established otherwise do it here
		return 0;
	}

	int read(unsigned char* buffer, int len, unsigned long timeoutMs) {
		mNet->setTimeout(timeoutMs);
		return mNet->readBytes((char*) buffer, len);
	}

	int write(unsigned char* buffer, int len, unsigned long timeoutMs) {
		mNet->setTimeout(timeoutMs);
		for (int i = 0; i < len; ++i) {
			mNet->write(buffer[i]);
		}
		mNet->flush();
		return len;
	}

	int disconnect() {
		// Implement TCP network disconnect here
		return 0;
	}

private:
	SoftwareSerial										*mNet;
} *network = NULL;

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

// ============== Subscription callback ========================================
void processMessage(MqttClient::MessageData& md) {
	const MqttClient::Message& msg = md.message;
	char payload[msg.payloadLen + 1];
	memcpy(payload, msg.payload, msg.payloadLen);
	payload[msg.payloadLen] = '\0';

  Serial.printf("Message arrived: qos %d, retained %d, dup %d, packetid %d, payload:[%s]", msg.qos, msg.retained, msg.dup, msg.id, payload);

	// LOG_PRINTFLN(
	// 	"Message arrived: qos %d, retained %d, dup %d, packetid %d, payload:[%s]",
	// 	msg.qos, msg.retained, msg.dup, msg.id, payload
	// );
}

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

  // Setup hardware serial for logging
	// Serial.begin(HW_UART_SPEED);
	// while (!Serial);

	// Setup network
	network = new Network;

	// Setup MqttClient
	MqttClient::System *mqttSystem = new System;
	MqttClient::Logger *mqttLogger = new MqttClient::LoggerImpl<HardwareSerial>(Serial);
	MqttClient::Network * mqttNetwork = new MqttClient::NetworkImpl<Network>(*network, *mqttSystem);

	//// Make 128 bytes send buffer
	MqttClient::Buffer *mqttSendBuffer = new MqttClient::ArrayBuffer<128>();

	//// Make 128 bytes receive buffer
	MqttClient::Buffer *mqttRecvBuffer = new MqttClient::ArrayBuffer<128>();
	
  //// Allow up to 2 subscriptions simultaneously
	MqttClient::MessageHandlers *mqttMessageHandlers = new MqttClient::MessageHandlersImpl<2>();
	
  //// Configure client options
	MqttClient::Options mqttOptions;
	
  ////// Set command timeout to 10 seconds
	mqttOptions.commandTimeoutMs = 10000;
	
  //// Make client object
	mqtt = new MqttClient (
		mqttOptions, *mqttLogger, *mqttSystem, *mqttNetwork, *mqttSendBuffer,
		*mqttRecvBuffer, *mqttMessageHandlers
	);
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

  // Check connection status
	if (!mqtt->isConnected()) {
		// Re-establish TCP connection with MQTT broker
		network->disconnect();
		network->connect("test.mosquitto.org", 8080);

		// Start new MQTT connection
    Serial.println("Connecting...");
		// LOG_PRINTFLN("Connecting");
		MqttClient::ConnectResult connectResult;
    Serial.println("Connecting 1");

		// Connect
		{
      Serial.println("Connecting 2");
			MQTTPacket_connectData options = MQTTPacket_connectData_initializer;

      Serial.println("Connecting 3");
			options.MQTTVersion = 4;
			options.clientID.cstring = (char*)MQTT_ID;
			options.cleansession = true;
			options.keepAliveInterval = 15; // 15 seconds
			MqttClient::Error::type rc = mqtt->connect(options, connectResult);
      Serial.println("Connecting 4");

			if (rc != MqttClient::Error::SUCCESS) {
				Serial.printf("Connection: %i", rc);
        // LOG_PRINTFLN("Connection error: %i", rc);
				return;
			}
		}
		// Subscribe
		{
			MqttClient::Error::type rc = mqtt->subscribe(
				MQTT_TOPIC_SUB, MqttClient::QOS0, processMessage
			);
			if (rc != MqttClient::Error::SUCCESS) {
        Serial.printf("Subscribe error: %i", rc);
        Serial.printf("Drop connection");
				// LOG_PRINTFLN("Subscribe error: %i", rc);
				// LOG_PRINTFLN("Drop connection");

				mqtt->disconnect();
				return;
			}
		}
	} else {
		// Publish
		{
			const char* buf = "Hello";
			MqttClient::Message message;
			message.qos = MqttClient::QOS0;
			message.retained = false;
			message.dup = false;
			message.payload = (void*) buf;
			message.payloadLen = strlen(buf);
			mqtt->publish(MQTT_TOPIC_PUB, message);
		}
		// Idle for 30 seconds
		mqtt->yield(30000L);
	}


  delay(10000);
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