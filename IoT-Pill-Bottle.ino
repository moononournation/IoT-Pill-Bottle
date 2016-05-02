/*
 * IoT Pill Bottle
 * Demo data view:
 * https://thingspeak.com/channels/111062
 * 
 * Reads GPIO13 status and analog value from pin A0, and writes it to a channel on ThingSpeak every 20 seconds.
 * 
 * Deep Sleep ref.:
 * https://www.openhomeautomation.net/esp8266-battery/
 */
/*
  ThingSpeak ( https://www.thingspeak.com ) is a free IoT service for prototyping
  systems that collect, analyze, and react to their environments.
  
  Copyright 2015, The MathWorks, Inc.
  
  Documentation for the ThingSpeak Communication Library for Arduino is in the extras/documentation folder where the library was installed.
  See the accompaning licence file for licensing information.
*/

/*
  *****************************************************************************************
  **** Visit https://www.thingspeak.com to sign up for a free account and create
  **** a channel.  The video tutorial http://community.thingspeak.com/tutorials/thingspeak-channels/ 
  **** has more information. You need to change this to your channel, and your write API key
  **** IF YOU SHARE YOUR CODE WITH OTHERS, MAKE SURE YOU REMOVE YOUR WRITE API KEY!!
  *****************************************************************************************/
#define LIP_SWITCH_PIN 13
#define SLEEP_TIME 20 // ThingSpeak will only accept updates every 15 seconds.
unsigned long myChannelNumber = 111062;
const char * myWriteAPIKey = "QIIY5NP3DZJ2LCDT";
char ssid[] = "YourAP";          //  your network SSID (name) 
char pass[] = "InputYourPasswordHere";   // your network password
bool lip_closed = false;

#ifdef SPARK
	#include "ThingSpeak/ThingSpeak.h"
#else
	#include "ThingSpeak.h"
#endif

/// ***********************************************************************************************************
// This example selects the correct library to use based on the board selected under the Tools menu in the IDE.
// Yun, Wired Ethernet shield, wi-fi shield, esp8266, and Spark are all supported.
// With Uno and Mega, the default is that you're using a wired ethernet shield (http://www.arduino.cc/en/Main/ArduinoEthernetShield)
// If you're using a wi-fi shield (http://www.arduino.cc/en/Main/ArduinoWiFiShield), uncomment the line below
// ***********************************************************************************************************
//#define USE_WIFI_SHIELD
#ifdef ARDUINO_ARCH_AVR

  #ifdef ARDUINO_AVR_YUN
    #include "YunClient.h"
    YunClient client;
  #else

    #ifdef USE_WIFI_SHIELD
      #include <SPI.h>
      // ESP8266 USERS -- YOU MUST COMMENT OUT THE LINE BELOW.  There's a bug in the Arduino IDE that causes it to not respect #ifdef when it comes to #includes
      // If you get "multiple definition of `WiFi'" -- comment out the line below.
      #include <WiFi.h>
      int status = WL_IDLE_STATUS;
      WiFiClient  client;
    #else
      // Use wired ethernet shield
      #include <SPI.h>
      #include <Ethernet.h>
      byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
      EthernetClient client;
    #endif
  #endif
#endif

#ifdef ARDUINO_ARCH_ESP8266
  #include <ESP8266WiFi.h>
  int status = WL_IDLE_STATUS;
  WiFiClient  client;
#endif

#ifdef SPARK
    TCPClient client;
#endif

void setup() {
  #if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_ESP8266)
    #ifdef ARDUINO_AVR_YUN
      Bridge.begin();
    #else
      #if defined(USE_WIFI_SHIELD) || defined(ARDUINO_ARCH_ESP8266)
        WiFi.begin(ssid, pass);
      #else
        Ethernet.begin(mac);
      #endif
    #endif
  #endif

  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  pinMode(LIP_SWITCH_PIN, INPUT_PULLUP);
  pinMode(A0, INPUT);

  ThingSpeak.begin(client);
}

void loop() {
  bool lip_opened = (digitalRead(LIP_SWITCH_PIN) == HIGH);
  if (lip_opened) {
    ThingSpeak.writeField(myChannelNumber, 1, lip_opened, myWriteAPIKey);
    lip_closed = false;
  } else {
    if (lip_closed) {
      digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on by making the voltage LOW
      // read the input on analog pin 0:
      int sensorValue = analogRead(A0);
      delay(5);
      sensorValue += analogRead(A0);
      delay(5);
      sensorValue += analogRead(A0);
      delay(5);
      sensorValue += analogRead(A0);
      sensorValue >>= 2;
      digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
    
      // Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
      ThingSpeak.writeField(myChannelNumber, 2, sensorValue, myWriteAPIKey);
    } else {
      ThingSpeak.writeField(myChannelNumber, 1, lip_opened, myWriteAPIKey);
      lip_closed = true;
    }
  }

  delay(SLEEP_TIME * 1000);
  // TODO: reduce power usage
  //ESP.deepSleep(SLEEP_TIME * 1000000, WAKE_RF_DEFAULT);
}
