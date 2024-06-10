/************************* Library Declaration *********************************/

#include <WiFi.h>
#include "WiFiClientSecure.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/************************* Pin Declaration *********************************/
#define LED 2

/************************* WiFi Access Point *********************************/

#define WLAN_SSID ""
#define WLAN_PASS ""

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
// Using port 8883 for MQTTS
#define AIO_SERVERPORT  8883

// Adafruit IO Account Configuration
// (to obtain these values, visit https://io.adafruit.com and click on Active Key)
#define AIO_USERNAME ""
#define AIO_KEY      ""

/************ Global State (you don't need to change this!) ******************/

// WiFiFlientSecure for SSL/TLS support
WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// io.adafruit.com root CA
const char* adafruitio_root_ca = \
                                 "-----BEGIN CERTIFICATE-----\n" \
                                 
                                 "-----END CERTIFICATE-----\n";
// this certificate is provided by the adafruit
/****************************** Feeds ***************************************/

// Setup a feed called 'test' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish test = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/sensor");

// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe light = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/light");

/*************************** Sketch Code ************************************/

void setup() {
  Serial.begin(115200);

    pinMode(LED, OUTPUT); // Declaring LED as OUTPUT

  Serial.println(F("Adafruit IO MQTTS (SSL/TLS) Example"));

  /*************************** WiFi Connection Code ************************************/
  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  delay(1000);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  delay(2000);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  // Set Adafruit IO's root CA
  client.setCACert(adafruitio_root_ca);//this is the new line added

  // Setup MQTT subscription for onoff feed.
  mqtt.subscribe(&light);
}

int x = 0;

void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000)))
  {
    if (subscription == &light)
    {
      // Storing the data
      char* Data = (char*) light.lastread;
      Serial.print("Got: ");
      Serial.println(Data);

      // comparing the data
      if (strstr(Data, "1"))
      {
        digitalWrite(LED, HIGH);
      }
      if (strstr(Data, "0"))
      {
        digitalWrite(LED, LOW);
      }
    }
  }


  // Now we can publish stuff!
  Serial.print("\nSending val ");
  Serial.print(x);
  Serial.print(" to sensor feed...");
  test.publish(x);
  x = x + 1;

  // wait a couple seconds to avoid rate limit
  delay(2000);

}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
    retries--;
    if (retries == 0) {
      // basically die and wait for WDT to reset me
      while (1);
    }
  }

  Serial.println("MQTT Connected!");
}