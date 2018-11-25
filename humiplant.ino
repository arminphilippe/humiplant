
//Analog Pins
const int waterLevelPin = A0;
const int soilPin = A1;
const int waterLevelMosFetPin = 9;

//digital Pins
const int redLedPin = 10;
const int greenLedPin = 11;
const int blueLedPin = 12;
const int buttonPin = 2;
const int onlyGreenLEDPin = 13;

//time constants
const int sensorConvergenceTime_ms = 6;
const int measurementInterval_s = 30;
const int communicationInterval_s = 10;
int serialCtr = 0;

// water level and humidity parameters
const int waterLevelEmptyAnalogVal = 50; //0 would actually be totally dry
const int soilTooDryThreshold = 400;


// colors
int red[3] = {150, 0, 0};
int green[3] = {0, 150, 0};
int blue[3] = {0, 0, 150};
int yellow[3] = {75, 75, 0};

// Function Declaations
//void redBlinkAllGood(const int);
//void redBlinkWaterLevelLow(const int);
//void redBlinkEmergency(const int);
//void redBlinkWatering(const int);
void blinkLED(const int);
int getAnalogHumiditySensorReading(const int, const int, const int);
void communicateSystemStatus (int systemStatus, const int redLedPin);
void blinkColorLED(const int, const int, const int, const int []);



//------------------------------------------------------------------------------------------------------------
void setup(){
  pinMode(waterLevelPin, INPUT);
  pinMode(soilPin, INPUT);
  pinMode(waterLevelMosFetPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  pinMode(onlyGreenLEDPin, OUTPUT);
  
  digitalWrite(waterLevelMosFetPin, LOW);

  Serial.begin(9600);
 
}


//------------------------------------------------------------------------------------------------------------
int currentWaterLevelAnalogVal;
int soilHumidity;
int systemStatus = 1; // capture current status between measurements to decouple loops
// 1: all good
// 2: humidity too low but reservoir ok -> watering
// 3: humidity okay but reservoir too low
// 4: emergency: humidity too low and reservoir empty
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
int buttonPressed = 0;


void loop(){

  currentMillis = millis();

  if (currentMillis - previousMillis > measurementInterval_s*1000) { //only measure infrequently but story system status

    if (serialCtr ==0) {
      //log header only once
      Serial.print("water level,soil humidity,measuring_0_1,system status,wateredPlantManually\n");
      serialCtr++;
    }
    previousMillis = currentMillis; // set counting back

    // check reservoir water level
   
    //read out analog humidity sensor (with mosfet to safe lifetime) 
    currentWaterLevelAnalogVal = getAnalogHumiditySensorReading(waterLevelPin, waterLevelMosFetPin, sensorConvergenceTime_ms);

    // check soil moisture
    soilHumidity = getAnalogHumiditySensorReading(soilPin, waterLevelMosFetPin, sensorConvergenceTime_ms);
    
    // check system status and store 
    if (soilHumidity < soilTooDryThreshold) { // soil humidity too low
      if (currentWaterLevelAnalogVal > waterLevelEmptyAnalogVal){ // if reservoir okay
        systemStatus = 2;
      }
      else { //blink emergency -> humidity too low and reservoir empty
        systemStatus = 4;
      }
    }
    else { // soil humidity okay
      if (currentWaterLevelAnalogVal < waterLevelEmptyAnalogVal) { // reservoir too low -> blink indication
        systemStatus = 3;
      }
      else { //else all good
        systemStatus = 1;
      }
    }
    Serial.print(currentWaterLevelAnalogVal);
    Serial.print(",");
    Serial.print(soilHumidity);
    Serial.print(",");
    Serial.print(1);
    Serial.print(",");
    Serial.print(systemStatus);
    Serial.print(",");
    Serial.print(buttonPressed);
    Serial.print("\n");
  }

  //blink LED for system status 
  communicateSystemStatus ( systemStatus, redLedPin); 

  Serial.print(currentWaterLevelAnalogVal);
  Serial.print(",");
  Serial.print(soilHumidity);
  Serial.print(",");
  Serial.print(0);
  Serial.print(",");
  Serial.print(systemStatus);
  Serial.print(",");
  Serial.print(buttonPressed);
  Serial.print("\n");


  //check for button pressing
  if (digitalRead(buttonPin) == HIGH) {
    buttonPressed = 1;

    //if button pressed and recorded, blink green LED
    blinkLED(onlyGreenLEDPin);
    
  }
  else {
    buttonPressed = 0;  
  }
  
  

  
  
  // only measure once in a while, slow system
  delay(communicationInterval_s*1000);
  
}


int getAnalogHumiditySensorReading(const int pinToRead, const int sensorMosFetPin, const int sensorConvergenceTime_ms) {

    // wake up sensor
    //Serial.print("\nwaking up sensor \n");
    digitalWrite(sensorMosFetPin, HIGH);
    
    //wait until converged
    delay(sensorConvergenceTime_ms);

    // measure
    int analogSensorReading= analogRead(pinToRead);
        
    //Serial.print("analog sensor level value: ");
    //Serial.print("\n");   
    //Serial.print(analogSensorReading);
    //Serial.print("\n");   
   
    // send sensor to sleep
    //Serial.print("send senor to sleep \n");
    digitalWrite(sensorMosFetPin, LOW);

    return analogSensorReading;
    
}



void communicateSystemStatus ( int systemStatus, const int redLedPin) {
  switch (systemStatus) {
     case 1:
       //redBlinkAllGood ( redLedPin );
       blinkColorLED (redLedPin, greenLedPin, blueLedPin, green);
       //Serial.print("all good: reservoir full and plant soil humid\n");
       break;
      case 2:
       //redBlinkWatering( redLedPin );
       blinkColorLED (redLedPin, greenLedPin, blueLedPin, blue);
       //Serial.print("watering the plant\n");
       break;
     case 3:
       //redBlinkWaterLevelLow( redLedPin );
       blinkColorLED (redLedPin, greenLedPin, blueLedPin, yellow);
       //Serial.print("soil okay but reservoir empty\n");
       break;
     case 4:
       blinkColorLED (redLedPin, greenLedPin, blueLedPin, red);
       //Serial.print("emergency: soil too dry and reservoir empty\n");
       break;
  }
}

void blinkColorLED(const int redPin, const int greenPin, const int bluePin, const int color[]) {
 
  int timeMs = 100;
  
  for (int ctr = 1; ctr <=2; ctr++) {
    analogWrite(redPin, color[0]);   
    analogWrite(greenPin, color[1]);   
    analogWrite(bluePin, color[2]);   
    delay(timeMs);                      
    analogWrite(redPin, 0);   
    analogWrite(greenPin, 0);   
    analogWrite(bluePin, 0);     
    delay(timeMs);
  } 
}

void blinkLED(const int pin) {
  int timeMs = 100;
  for (int ctr = 1; ctr <=3; ctr++) {
    digitalWrite(pin, HIGH);   
    delay(timeMs);                      
    digitalWrite(pin, LOW);    
    delay(timeMs);
  }
}

////unused, only red LED
//
//void redBlinkWaterLevelLow(const int pin) {
//  int timeMs = 200;
//  for (int ctr = 1; ctr <=3; ctr++) {
//    digitalWrite(pin, HIGH);   
//    delay(timeMs);                      
//    digitalWrite(pin, LOW);    
//    delay(timeMs);
//  } 
//}
//
//void redBlinkEmergency(const int pin) {
//  int timeMs = 200;
//  for (int ctr = 1; ctr <=4; ctr++) {
//    digitalWrite(pin, HIGH);   
//    delay(timeMs);                      
//    digitalWrite(pin, LOW);    
//    delay(timeMs);
//  }
//}
//
//
//void redBlinkAllGood(const int pin) {
//  int timeMs = 200;
//  for (int ctr = 1; ctr <=1; ctr++) {
//    digitalWrite(pin, HIGH);   
//    delay(timeMs);                      
//    digitalWrite(pin, LOW);    
//    delay(timeMs);
//  }
//}
//
//
//void redBlinkWatering(const int pin) {
//  int timeMs = 200;
//  for (int ctr = 1; ctr <=2; ctr++) {
//    digitalWrite(pin, HIGH);   
//    delay(timeMs);                      
//    digitalWrite(pin, LOW);    
//    delay(timeMs);
//  }
//}
