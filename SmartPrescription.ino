#include <SPI.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <MFRC522.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "font.h"

#define SS_PIN 2  //D4
#define RST_PIN 0 //D3
MFRC522 mfrc522(SS_PIN, RST_PIN);   //object

/*
 SDA(SS)---------> D4
 SCK---------> D5
 MOSI--------> D7
 MISO--------> D6
 RST---------> D3
 */


/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "nissan"
#define WLAN_PASS       "password"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "nishanchettri"
#define AIO_KEY         "ac6926a368534677bf50379ba383e6d5"

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish tx = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/see");

// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe rx = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/send");

/*************************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();


/************OLED****************/
#define oledReset LED_BUILTIN
Adafruit_SSD1306 display(oledReset);

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif
/***********end/*************/
void setup() {
  Serial.begin(115200);
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C,false);   // initialize with the I2C addr 0x3C (for the 128x64)
  display.clearDisplay(); 
  display.setTextSize(1);
  display.setTextColor(WHITE);
  
  //display.setCursor(0,10);
  display.setFont(&Open_Sans_Regular_10);  
  delay(10);
  display.display();
  //....................................
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);
//......................................
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); 
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: "); 
  Serial.println(WiFi.localIP());
  // Setup MQTT subscription for onoff feed.
  mqtt.subscribe(&rx);
       if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Select one of the cards
  
}

uint32_t x=0;

void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.

  MQTT_connect();


   
    Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &rx) {
      Serial.print(F("Got: "));
      Serial.println((char *)rx.lastread);
      
    }
      if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }
  //Show UID on serial monitor
  Serial.println();
  Serial.print(" UID tag :");
  String content= "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();
 if (content.substring(1) == "80 27 79 89") //change UID of the card that you want to give access
  {
    tx.publish((char *)rx.lastread);
    Serial.println((char *)rx.lastread);
    display.clearDisplay();
    display.setCursor(0,10);
    display.println("hhh");
    display.display();
  }
  }

  /*
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
  */
}

void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT...");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");

}
