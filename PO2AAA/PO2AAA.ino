/*
  P&O 2 - AAA

  Het Automatische Afweeg Apparaat krijgt via bluetooth een bestelling korrels binnen 
  en laadt via schroeven deze bestelling in een laadbak. Er zijn 3 soorten korrels waarvan
  elk een bepaald gewicht wordt besteld. Het juiste gewicht wordt tijdens het laden afgewogen en 
  wanneer het juiste gewicht voor elk type korrel voltooid is stopt de AAA automatisch.

  Gebruikte componenten / aansluitingen:
  
  * SEN0160 gewichtssensor:  DOUT:  D2  | CLK:  D3
  * HC06 bluetooth module:   TX:    D8  | RX:   D9
  * 3x DC motor via MOSFET: PWM:   D5, D6, D10
 
  * voeding: 12v, componenten aangesloten op voeding via 12v->5v LDO's

  feb-mei 2020
  groep 311
*/

/* -- LIBRARIES -- */

//https://github.com/DFRobot/DFRobot_HX711
#include <DFRobot_HX711.h>
#include <SoftwareSerial.h>

/* -- CONSTANTS -- */

// pins:
const int

SEN0160_DOUT = 2,
SEN0160_CLK  = 3,

HC06_TX = 8,
HC06_RX = 9;

const int DC[] = {5, 6, 10};

//gewichtssensor calibratie schaal (experimenteel vastleggen?)
const float HX711_CALIB = 1992; // default?

//aantal soorten korrels
const int ORDER_TYPE_AMT = 3;

//nauwkeurigheid (g)
const int WEIGHT_PRECISION = 1;

//vanaf hoeveel g verschil tussen gemeten gewicht-besteld gewicht moet de motor trager draaien?
const int WEIGHT_SLOWDOWN_MOTOR = 20;

//motor snelheid
const int MOTOR_SPEED_FAST = 100;  // ???? moet getest worden
const int MOTOR_SPEED_SLOW = 25;   // ???? moet getest worden

/* -- GLOBAL VARS -- */

// is de bestelling al ontvangen?
bool receivedOrder = false;

// ontvangen bestelling data
String receivedOrderData = "";

// aantal gram per type korrel
float order[ORDER_TYPE_AMT];

// welk deel van de bestelling (type korrel) de AAA aan bezig is
int orderType = 0;

// gewichtssensor object
DFRobot_HX711 weightSensor(SEN0160_DOUT, SEN0160_CLK);
SoftwareSerial bluetooth(HC06_TX, HC06_RX);

void setup() {

  // start serial (voor debugging)
  Serial.begin(9600);

  //SEN0160 setup
  Serial.println("HX711 calibreren");
  // stel calibratie schaal in
  weightSensor.setCalibration(HX711_CALIB);
  // zet weegschaal op nul
  weightSensor.setOffset(weightSensor.averageValue()); 
  Serial.println("HX711 gecalibreerd, testwaarde 'lege' weegschaal printen (~0):");
  Serial.println(weightSensor.readWeight());
}

// lus
void loop() {
  if(!receivedOrder) {
    awaitReceiveOrder();
  }
  else {
    loadOrder();
  }
}


// bluetooth module checkt of er een bestelling binnenkomt
// formaat binnenkomende bestelling: "(<int>, <int>, <int>)"
void awaitReceiveOrder() {

  //de index van het onderdeel van de bestelling dat genoteerd wordt
  int recvOrderType = 0;

  //wordt er data gelezen?
  bool receiving = false;
  
  while(bluetooth.available() > 0) {

    // lees een binnenkomend karakter
    char nextChar = (char)bluetooth.read();

    // '(' = beginkarakter, start lezen bestelling
    if(!receiving && nextChar == '(') {
      receiving = true;
    }
    // ',' = separator, stop dit getal te lezen, begin het volgende getal te lezen
    else if(receiving && nextChar == ',') {
      recvOrderType++ ;
    }
    // ')' = eindkarakter, stop lezen bestelling
    else if(receiving && nextChar == ')') {
      receiving = false;
    }
    // andere karakters gaan we van uit dat het de cijfers zijn
    else if(receiving) {
      int nextDigit = atoi(nextChar); // volgend cijfer
      // voeg cijfer toe aan getal (links naar rechts), bvb: we hebben al 92, volgend cijfer is 3 -> getal wordt: 92 * 10 + 3 = 923
      order[recvOrderType] = order[recvOrderType] * 10 + nextDigit;  
    }
  }
  
}


// motoren laden de bestelling in en alles wordt gewogen & gecontroleerd door de gewichtssensor
void loadOrder() {
  
  // alle onderdelen zijn afgehandeld, reset:
  if(orderType == ORDER_TYPE_AMT) {
    orderType = 0;
    receivedOrder = false;

    Serial.println("Volledige bestelling is ingeladen");
  }
  else {
    float weight = weightSensor.readWeight();

    // dit onderdeel is afgehandeld, (
    if(order[orderType] - weight < -WEIGHT_PRECISION) {
      // WAARSCHUWING: PROGRAMMA BETER INSTELLEN, TE VEEL INGELADEN!!!
      Serial.print("WAARSCHUWING!!! -- Te veel ingeladen, stel het programma beter in!!! : ");
      Serial.println(order[orderType]-weight);
    }
    
    if(order[orderType] - weight < WEIGHT_PRECISION) {
      // STOP MOTOR, GENOEG VOOR PRECISIE
      analogWrite(DC[orderType], 0);

      // zet weegschaal offset voor volgende deel
      weightSensor.setOffset(weightSensor.averageValue());
      orderType++;
      
      Serial.print("Deel ");
      Serial.print(orderType);
      Serial.print("van de bestelling afgewerkt. (besteld: ");
      Serial.print(order[orderType]);
      Serial.print(", geleverd: ");
      Serial.println(weightSensor.readWeight());
    }
    else if(order[orderType] - weight < WEIGHT_SLOWDOWN_MOTOR) {
      // STOP MOTOR, GENOEG VOOR PRECISIE
      analogWrite(DC[orderType], MOTOR_SPEED_SLOW);
    }
    else {
      analogWrite(DC[orderType], MOTOR_SPEED_FAST);
    }
    
  }
}
