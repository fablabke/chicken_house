/****************** Hardware **************************

    arduino uno / nano
    capteur hall > capteur à placer pour la fermeture (new)
    DC + driver
    clock DS3231
    bouton poussoir

****************** Librairies **************************/


// INCLUDE CHRONO LIBRARY : http://github.com/SofaPirate/Chrono
#include <Chrono.h>


//objet chrono pour DHT11 && NTP (horloge)
Chrono capteur_Chrono;
Chrono horloge_Chrono;



// Date and time functions using a DS3231 RTC connected via I2C and Wire lib
#include "RTClib.h"
RTC_DS3231 rtc;



/******************* définitions des pins ************************************/

// capteur effet hall :
const byte capteur_bas = 8;
const byte capteur_haut =  7;

// controlleur moteur
// https://passionelectronique.fr/tutoriel-l298n/
const byte moteur_a = 3; //  D3 nano
const byte moteur_b = 4; //  D4 nano

const byte bouton_UPandDOWN = 9;
//const byte bouton_DOWN = 10

// heures couché et levé du soleil adapté en heure d'été voir bas du code
const byte sunUp[12][2] =  {10, 0, 9, 30, 8, 30, 7, 30, 6, 30, 6, 00, 6, 00, 6, 30, 7, 00, 8, 00, 8, 30, 9, 30};
const byte sunDown[12][2] =  {18, 0, 18, 45, 19, 30, 20, 30, 21, 0, 22, 30, 22, 00, 21, 30, 20, 30, 19, 30, 18, 30, 18, 00};

/*************************** configuration ********************************************/

const int temps_porte_max = 6000; // temps ouverture/fermeture maximum en millisecondes, si dépassé, on arrête le moteur

/*************************** variables ***************************************/
byte heure_ouverture = 6;
byte minute_ouverture = 30;

byte heure_fermeture = 22;
byte minute_fermeture = 30;

// horloge
byte heure;
byte minute;

byte heure_maintenant;
byte  minute_maintenant;
byte mois_maintenant;

//dht
float t, h;

// for DHT11,
//      VCC: 5V or 3V
//      GND: GND
//      DATA: 2



bool century = false;
bool h12Flag;
bool pmFlag;

int  etat_porte = 0; // 1 = levée / 0 = baissée
bool verification_ouverture_avant_fermeture = 0;



void setup() {


  Serial.begin(115200);


  Serial.println("-------------------- Chicken controller -------------");
  Serial.println("startup");

  // ajuster l'heure et l'heure d'ouverture et fermeture :
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();

  } else {
    sunUpSunDown();
    serial_date();

  }

  delay(10);

  // Start the I2C interface
  Wire.begin();

  pinMode(capteur_bas, INPUT);
  pinMode(capteur_haut, INPUT);

  pinMode(moteur_a, OUTPUT);
  pinMode(moteur_b, OUTPUT);


  pinMode(bouton_UPandDOWN, INPUT_PULLUP);
  // pinMode(bouton_b, INPUT_PULLUP);



  delay(1000);


  /*** reglage heure ***/
  // s'écrit ainsi rtc.adjust(DateTime(année, mois, jour, heure, minute, second));
  // rtc.adjust(DateTime(2022, 5, 29, 18, 00, 00));


  horloge();

}

void loop() {

  /*** code de test de la porte***/
  //test_ouverture_fermeture_porte_en_continue();

  btn();

  //toutes les 30 secondes, on check l'horloge
  if (horloge_Chrono.hasPassed(5000) ) {
    horloge_Chrono.restart();  // restart the crono so that it triggers again later
    horloge();
  }


  delay(100);
  serial_date();

}



void btn () {
  if ( digitalRead(bouton_UPandDOWN) == LOW ) {
    Serial.print("bouton est pressé ");
    //si la porte est ouverte elle se ferme
    if (etat_porte == 1) {
      porte_monte();
    }
    // si fermée elle s'ouvre
    else if (etat_porte == 0) {
      porte_descend();
    }
  }
}


void porte_monte() {



  Serial.println("Demande ouverture porte");

  // active le moteur




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
      break;

    }

    delay(5);
  }

  // frein moteur
  digitalWrite(moteur_a, LOW);
  digitalWrite(moteur_b, LOW);





  delay(500);

  // couper le moteur


  etat_porte = 1;

}

void porte_descend() {

  Serial.println("Demande fermeture porte");

  // active le moteur


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
      break;
    }

    delay(5);
  }


  // frein moteur
  digitalWrite(moteur_a, LOW);
  digitalWrite(moteur_b, LOW);



  delay(500);

  // couper le moteur


  etat_porte = 0;

}


void envoiCapteurs() {

  Serial.print("Capteur haut : ");
  Serial.println(!digitalRead(capteur_haut));


  Serial.print("Capteur bas : ");
  Serial.println(!digitalRead(capteur_bas));

}
void horloge() {

  DateTime now = rtc.now();
  heure_maintenant = now.hour();
  minute_maintenant = now.minute();
  delay(10);
  // wake up !!!! verification que la porte est bien ouverte + verif heure levé et couché du soleil
  if (heure_maintenant == heure_ouverture && minute_maintenant == minute_ouverture ) {
    porte_monte();
    sunUpSunDown();
  }
  //time to sleep chickens <3 // verification que la porte est fermée
  else if ( heure_maintenant == heure_fermeture && minute_maintenant == minute_fermeture) {
    verification_ouverture_avant_fermeture;
    porte_descend();
  }
  //verification toutes les heures piles que la porte est bien ouverte
  else if ( heure_maintenant = heure_fermeture && minute_maintenant == 00 && verification_ouverture_avant_fermeture) {
    !verification_ouverture_avant_fermeture;
    porte_monte();
  }
}

void sunUpSunDown() {
  DateTime now = rtc.now();
  if (mois_maintenant != now.month()) {

    mois_maintenant = now.month();
    heure_ouverture = sunUp [mois_maintenant][0];
    minute_ouverture = sunUp [mois_maintenant][1];

    heure_fermeture = sunDown [mois_maintenant][0];
    minute_fermeture = sunDown [mois_maintenant][1];
  }
}

void serial_date() {
  DateTime now = rtc.now();
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
  Serial.print("heure ouverture : ");
  Serial.print(heure_ouverture);
  Serial.print("H");
  Serial.println(minute_ouverture);
  Serial.print("heure fermeture : ");
  Serial.print(heure_fermeture);
  Serial.print("H");
  Serial.println(minute_fermeture);

}



/*

                                                         &@@@@&(
                                ,@@@@@@@&@/        ,@@@@@@&@@&@@@@@
                              @@@@@(. .(@@@@@@&@@@@@&@%         *@@@@.
                             &@@@.         #@&&&@&.               *@@@&
                             @@@&                                   @&&%
                       /@@@@@@@@@                                   (&@@
                     #@@@@@@@@@@%                                   ,&@&/
                    #@@@(                                           %@@@
                    @@@@                                           &&@&(
                    @&@@                                         @&@@@.
                    #@@&,                                 ,(&@@@@@@&
                     @@@&                 *&@@@@@@@&@@&@@@@@@@@#
                     ,&@@&          ,&@@@@@@@#*,.     .*#&@@@@@&%
                      .&&@@      &@@@@@,                      .&@@@@*
                        @@@@( ,@@@@#                              /@@@@
                          &@@@@@&,      ,@%.(                        %@@@.
                           @@@@#      *@@@&#@@&                        (@@&
                          @@@@@@@,    (@@@@@@@@                          &@&,
                        #@@@@   @&*     (@@@&.                            /@&*
                      .@@@@,     @@                       /%               /@&,
                     (&@@@       *@@.                     @                 #@@
                    (&@@@@@@@@@@@@@@@@.                 *&,                  @@%
                    @@@@@@/.     &@@@@@@@.            /&&                    /@&
                                #@@@( &@@@@@@@%##%@@@@(                       @@.
                               (&@&%   @@@&,,,,,.                             &@,
                              @@&&,    &@@@                                   %&*
                             @@@@      @@@&                                   #@(
                            #@@@     .&@@@.                                   /@#
                             @@@@@&@@@@@&/                                     *
                                ,*,.#@@@,
                                   %&@@.
                                  @@@@
                                 @@@&
                               .&@@%
                                /%.

  1 janvier 2022   09H00   17h00   +1 min.
  1 fevrier 2022  08h30   17h45   +3 min.
  1 mars 2022   07h30   18h30   +3 min.
  +1 heure
  1 avril 2022  07h30   20h30   +4 min.
  1 mai 2022  06h30   21h00   +3 min.
  1 juin 2022   06H00   22h30   +2 min.
  1 juillet 2022  06H00   22H   -1 min.
  1 aout 2022   06h30   21h30   -2 min.
  1 septembre 2022  07h00   20h30
  1 octobre 2022  08H   19h30   -4 min.
  - 1HEURE
  1 novembre 2022   07h30   17h30   -4 min.
  1 decembre 2021   08h30   17H   -1 min.

  convertion sur base de l'heure d'été
  1 janvier 2022  10H00   18h00   +1 min.
  1 fevrier 2022  09h30   18h45   +3 min.
  1 mars 2022   08h30   19h30   +3 min.
  +1 heure
  1 avril 2022  07h30   20h30   +4 min.
  1 mai 2022  06h30   21h00   +3 min.
  1 juin 2022   06H00   22h30   +2 min.
  1 juillet 2022  06H00   22H   -1 min.
  1 aout 2022   06h30   21h30   -2 min.
  1 septembre 2022  07h00   20h30
  1 octobre 2022  08H   19h30   -4 min.
  - 1HEURE
  1 novembre 2022   08h30   18h30   -4 min.
  1 decembre 2021   09h30   18H   -1 min.
*/
