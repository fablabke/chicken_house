
/* TODO
    un chrono pour la durée de monté et descente de la porte
    détecter position initiale de la porte avec les capteurs à effet hall, (et lever la porte quand on boote le système si état inconnu ?)
    integrer le board au blog
    calcul levé et couhcer de soleil : https://github.com/signetica/SunRise


*/


/******************* Librairies **************************/

#include <ESP8266WiFi.h>


#include "Adafruit_MQTT.h"


#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "AdafruitIO_WiFi.h"

#include <DHT.h>
//ntp
#include <NTPClient.h>
#include <WiFiUdp.h>
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");


// INCLUDE CHRONO LIBRARY : http://github.com/SofaPirate/Chrono
#include <Chrono.h>


//objet chrono pour DHT11 && NTP (horloge)
Chrono capteur_Chrono;
Chrono horloge_Chrono;



// real time clock, module DS3231
#include <DS3231.h>
#include <Wire.h> // pour i2c

DS3231 myRTC;




// nom des pins esp8266 : https://mechatronicsblog.com/esp8266-nodemcu-pinout-for-arduino-ide/


/******************* définitions des pins ************************************/

// capteur effet hall :
const byte capteur_bas = 5;     // capteur fin de course bas de la porte (D0 sur esp8266)
const byte capteur_haut =  16;      // capteur fin de course haut de la porte (D1 sur esp8266)

// controlleur moteur
// https://passionelectronique.fr/tutoriel-l298n/

const byte moteur_a = 14; // D5 sur esp
const byte moteur_b = 0; // D3 sur esp
const byte moteur_enable = 4; // D2 sur esp


const byte sda_pin = 13; // D7 sur esp8266
const byte scl_pin = 12; // D6 sur esp8266

const byte DHTPin = 2;


/*************************** configuration ********************************************/
const int heure_ouverture = 6;
const int minute_ouverture = 00;

const int heure_fermeture = 22;
const int minute_fermeture = 00;



const int temps_porte_max = 6000; // temps ouverture/fermeture maximum en millisecondes, si dépassé, on arrête le moteur


const int bouton_a = 8; // bouton a sur pin D8
const int bouton_b = 0; // bouton a sur pin A0


/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "fablabke-4g"
#define WLAN_PASS       "nowayjustforfablabke"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "fablabke"
#define AIO_KEY         "aio_HjhX26FRFBl3ZlItSqpxq1QbTdH1"

AdafruitIO_WiFi io(AIO_USERNAME, AIO_KEY, WLAN_SSID, WLAN_PASS);




/*************************** variables ***************************************/
// horloge
int heure;
int minute;
//dht
float t, h;

// for DHT11,
//      VCC: 5V or 3V
//      GND: GND
//      DATA: 2

#define DHTType DHT11

DHT dht(DHTPin, DHTType);


bool century = false;
bool h12Flag;
bool pmFlag;



/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiClientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
/****************************** Feeds ***************************************/


// controle de la porte :
Adafruit_MQTT_Subscribe porte = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/porte");

// capteurs connectés à l'esp :
Adafruit_MQTT_Publish Temperature = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Temperature");
Adafruit_MQTT_Publish Humidity = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Humidity");
Adafruit_MQTT_Publish DoorUp = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/door_up");
Adafruit_MQTT_Publish DoorDown = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/door_down");
Adafruit_MQTT_Publish Log = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/log");




/*************************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();//<-??




void setup() {

  Serial.begin(115200);
  dht.begin();
  delay(10);



  // Start the I2C interface
  Wire.begin(sda_pin, scl_pin);


  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    ESP.wdtFeed();
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  // Setup MQTT subscription for onoff feed.
  //mqtt.subscribe(&porte);


  pinMode(capteur_bas, INPUT);
  pinMode(capteur_haut, INPUT);

  pinMode(moteur_a, OUTPUT);
  pinMode(moteur_b, OUTPUT);
  pinMode(moteur_enable, OUTPUT);

  pinMode(bouton_a, INPUT_PULLUP);
  pinMode(bouton_b, INPUT_PULLUP);

  digitalWrite(moteur_enable, LOW);



  Serial.println("Connexion a client ntp en cours");
  // set up horloge
  timeClient.begin();

  timeClient.setTimeOffset(7200);

  Serial.println("Connexion au serfeur mqtt en cours");
  MQTT_connect();

  Log.publish("Chicken controler booté");

  delay(1000);


  // utiliser ce bloc pour ajuster l'heure :
  /*
    myRTC.setClockMode(false);
    myRTC.setYear(22);
    myRTC.setMonth(5);
    myRTC.setDate(19);
    myRTC.setDoW(4);
    myRTC.setHour(12);
    myRTC.setMinute(14);
    myRTC.setSecond(0);
  */


}








void loop() {


  Serial.println(digitalRead(bouton_a));
  delay(100);
  ESP.wdtFeed();



  // code de test de la porte:
  /*
    while (1)
    {
    porte_monte();
    delay(8000);
    porte_descend();
    delay(8000);
    }
  */

  MQTT_connect();
  delay(10);



  // toutes les 60 secondes, on envoi les donénes des capteurs
  if (capteur_Chrono.hasPassed(60000) ) {
    capteur_Chrono.restart();  // restart the crono
    envoiCapteurs();
  }



  // toutes les 30 secondes, on check l'horloge
  if (horloge_Chrono.hasPassed(30000) ) {
    horloge_Chrono.restart();  // restart the crono so that it triggers again later
    horloge();
  }




  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &porte) {
      Serial.print(F("Got: "));
      Serial.println((char *)porte.lastread);


      if (strcmp((char *)porte.lastread, "haut") == 0) {
        porte_monte();
      }
      if (strcmp((char *)porte.lastread, "bas" ) == 0) {
        porte_descend();
      }

    }


  }

  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds

  if (! mqtt.ping()) {
    mqtt.disconnect();
  }

  ESP.wdtFeed();


}




void porte_monte() {

  Serial.println("Demande ouverture porte");

  // active le moteur

  digitalWrite(moteur_enable, HIGH);


  int temps = millis();


  // tant que le capter haut n'est pas proche d'un aimant, faire tourner le moteur
  while (digitalRead(capteur_haut) == HIGH && digitalRead(capteur_haut) == HIGH && digitalRead(capteur_haut) == HIGH)
  {
    //Serial.println("Capteur haut inactif");
    digitalWrite(moteur_a, HIGH);
    digitalWrite(moteur_b, LOW);


    // on vérifie si ça ne fait pas trop longtemps que cette porte essaie de bouger :-)
    if (millis() > temps + temps_porte_max) {
      Serial.println("Capteur haut inactif depuis trop longtemps, moteur coupé!");
      Log.publish("Capteur haut inactif depuis trop longtemps, moteur coupé!");
      break;

    }

    delay(5);
  }

  // frein moteur
  digitalWrite(moteur_a, LOW);
  digitalWrite(moteur_b, LOW);


  envoiCapteurs();

  Log.publish("Porte montée");


  delay(500);

  // couper le moteur
  digitalWrite(moteur_enable, LOW);




}

void porte_descend() {

  Serial.println("Demande fermeture porte");

  // active le moteur
  digitalWrite(moteur_enable, HIGH);

  int temps = millis();

  // tant que le capter bas n'est pas proche d'un aimant, faire tourner le moteur
  while (digitalRead(capteur_bas) == HIGH && digitalRead(capteur_bas) == HIGH && digitalRead(capteur_bas) == HIGH)
  {
    Serial.println("Capteur bas inactif");
    digitalWrite(moteur_a, LOW);
    digitalWrite(moteur_b, HIGH);

    // on vérifie si ça ne fait pas trop longtemps que cette porte essaie de bouger :-)
    if (millis() > temps + temps_porte_max) {
      Serial.println("Capteur bas inactif depuis trop longtemps, moteur coupé!");
      Log.publish("Capteur bas inactif depuis trop longtemps, moteur coupé!");
      break;
    }

    delay(5);
  }


  // frein moteur
  digitalWrite(moteur_a, LOW);
  digitalWrite(moteur_b, LOW);


  envoiCapteurs();


  delay(500);

  // couper le moteur
  digitalWrite(moteur_enable, LOW);

  envoiCapteurs();
  Log.publish("Porte descendue");


}


/**
   Function to connect and reconnect as necessary to the MQTT server.
  Should be called in the loop function and it will take care if connecting.
*/
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




/**
   Envoi les données des capteurs à adafruit io
*/
void envoiCapteurs() {

  if (! Temperature.publish(dht.readTemperature())) {
    Serial.println(F("Temperature Failed to send to IO"));
  }

  if (! Humidity.publish(dht.readHumidity())) {
    Serial.println(F("Humidity Failed to send to IO"));
  }

  if (! DoorUp.publish(!digitalRead(capteur_haut))) {
    Serial.println(F("Door up Failed to send to IO"));
  }


  if (! DoorDown.publish(!digitalRead(capteur_bas))) {
    Serial.println(F("Door down Failed to send to IO"));
  }


  Serial.print("Capteur humidité : ");
  Serial.println(dht.readHumidity());

  Serial.print("Capteur temperature : ");
  Serial.println(dht.readTemperature());



  Serial.print("Capteur haut : ");
  Serial.println(!digitalRead(capteur_haut));


  Serial.print("Capteur bas : ");
  Serial.println(!digitalRead(capteur_bas));

}
void horloge() {


  // ancien code qui utilisait un client NTP :
  /*
    timeClient.update();
    String formattedTime = timeClient.getFormattedTime();
    Serial.print("Formatted Time: ");
    Serial.println(formattedTime);
    heure = timeClient.getHours();
    minute = timeClient.getMinutes();

    Serial.print("HEURE: ");
    Serial.println(heure);

    Serial.print("Minute: ");
    Serial.println(minute);

  */



  Serial.println("Heure RTC DS3132 : ");
  Serial.print("Date: ");
  Serial.print(myRTC.getDate(), DEC);
  Serial.print("/");
  Serial.print(myRTC.getMonth(century), DEC);
  Serial.print("/");
  Serial.print(myRTC.getYear(), DEC);
  Serial.print("\t Hour : ");
  Serial.print(myRTC.getHour(h12Flag, pmFlag), DEC);
  Serial.print(":");
  Serial.print(myRTC.getMinute(), DEC);
  Serial.print(".");
  Serial.println(myRTC.getSecond(), DEC);




  // maintenant on utilise un circuit horloge en temps réel :

  heure = myRTC.getHour(h12Flag, pmFlag);
  minute = myRTC.getMinute();

  if (heure == heure_ouverture && minute == minute_ouverture) {
    Log.publish("C'est l'heure d'ouverture");
    porte_monte();
  }

  if (heure == heure_fermeture && minute == minute_fermeture) {
    Log.publish("C'est l'heure de fermeture");
    porte_descend();
  }

}
