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
const int PRECISION_WEIGHT = 1;

/* -- GLOBAL VARS -- */

// is de bestelling al ontvangen?
bool receivedOrder = false;

// aantal gram per type korrel
float order[ORDER_TYPE_AMT];

// welk deel van de bestelling (type korrel) de AAA aan bezig is
int orderType = 0;

// gewichtssensor object
DFRobot_HX711 weightSensor(SEN0160_DOUT, SEN0160_CLK);


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

void loop() {

  if(!receivedOrder) {
    awaitReceiveOrder();
  }
  
  else {
    loadOrder();
  }

}


// bluetooth module checkt of er een bestelling binnenkomt
void awaitReceiveOrder() {
  
  // TODO: bluetooth lezen implementeren 
  
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
    if(order[orderType] - weight < -PRECISION_WEIGHT) {
      // TODO: STOP MOTOR + PROGRAMMA BETER INSTELLEN, TE VEEL!!!

      Serial.print("WAARSCHUWING!!! -- Te veel ingeladen, werk het programma bij!!! : ");
      Serial.println(order[orderType]-weight);
    }
    else if(order[orderType] - weight < WEIGHT_PRECISION) {
      // TODO: STOP MOTOR, GENOEG VOOR PRECISIE

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
    else if(order[orderType] - weight < WEIGHT_SLOWDOWN_PROXIMITY) {
      // TODO: VERTRAAG MOTOR
    }
    else {
      // TODO: DRAAI MOTOR
    }
    
  }
}
