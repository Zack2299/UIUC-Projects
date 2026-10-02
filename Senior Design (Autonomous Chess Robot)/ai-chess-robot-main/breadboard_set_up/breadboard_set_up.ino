#include <MultiStepper.h>
#include <AccelStepper.h>
//#include <Scheduler.h>


#define nsleepPin 5
#define dirXPin 7
#define resetPin 4
#define stepXPin 6
#define stepY1Pin 16
#define dirY1Pin 15
#define stepY2Pin 18
#define dirY2Pin 17
#define redLed 14 // Both LEDs are -1 off for some reason, actually in GPIO 9 and 13
#define greenLed 10
#define magnetPin 12

int totalStepWidthY = 1910;
int totalStepWidthX = 1910;
long start = 0;

long stepsPerSpaceY = totalStepWidthY / 16;
long stepsPerSpaceX = totalStepWidthX / 16;
int destX = 0;
int destY1 = 0; // Y1 has orange wire attached to red
int destY2 = 0;

int pathArray[225];
bool readDone = false;
bool cheating = false;
bool capture = false;
bool promotion = false;
bool setPromotion = false;
bool moveDone = false;
int constOffset = 19;
int counter = 0;
char tempCoords[] = "00"; 
int coordPlace = 0;

AccelStepper stepperX(AccelStepper::DRIVER, stepXPin, dirXPin);
AccelStepper stepperY1(AccelStepper::DRIVER, stepY1Pin, dirY1Pin);
AccelStepper stepperY2(AccelStepper::DRIVER, stepY2Pin, dirY2Pin);
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(nsleepPin, OUTPUT);
  digitalWrite(nsleepPin, HIGH);
  stepperX.setMaxSpeed(500.0);
  stepperX.setSpeed(150.0);
  stepperX.setAcceleration(100.0);

  stepperY1.setMaxSpeed(500.0);
  stepperY1.setSpeed(150.0);
  stepperY1.setAcceleration(100.0);

  stepperY2.setMaxSpeed(500.0);
  stepperY2.setSpeed(150.0);
  stepperY2.setAcceleration(100.0);
  delay(10);

  pinMode(redLed, OUTPUT);
  pinMode(magnetPin, OUTPUT);
  pinMode(greenLed, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  readData();
  if(cheating){
    cheatingDetected();
  }
  else{
    movePiece();
    delay(10);
    if(promotion){
      promotePiece();    
    }
    readDone = false;
    if(moveDone){
      Serial.write("Done\n");
    }
    moveDone = false;
    
  } 
}

void promotePiece(){
//  Serial.println("in promotion");
  digitalWrite(redLed, HIGH);
  delay(600);
  digitalWrite(redLed, LOW);
  delay(600);
}

void cheatingDetected(){
//  Serial.println("Red On");
  digitalWrite(redLed, HIGH);
  delay(100);
  digitalWrite(redLed, LOW);
//  Serial.println("Red Off");
  delay(100);
//  Serial.println("Green On");
  digitalWrite(greenLed, HIGH);
  delay(100);
  digitalWrite(greenLed, LOW);
//  Serial.println("Green Off");
  delay(100);
}

void readData(){
  if(Serial.available() > 0){
    cheating = false; //Reset everytime new data comes through
    promotion = false;
//    Serial.print("incoming data");
    char inByte = Serial.read();
//    Serial.println(inByte);
    if(inByte == '>'){
      pathArray[counter] = inByte;
      counter = 0;
      readDone = true;
      if(setPromotion)
        promotion = true;
      setPromotion = false;
//      delay(2000);
      // Serial.println(promotion);
    }
    else if(inByte == '?'){
      cheating = true;
//      delay(20);
    }
    else if(inByte == '^'){
      setPromotion = true;
//      delay(20);
    }
    else if(inByte == '<'){
      capture = true;
      pathArray[counter] = inByte;
      counter++;
    }
    else if(inByte == '@'){
      // do nothing (stop flashing lights)
      return;
    }
    else if(inByte != ';'){
      tempCoords[coordPlace] = inByte;
      coordPlace++;
    }
    else{
      int tensPlace = tempCoords[0] - '0';
      int onesPlace = tempCoords[1] - '0';
//      Serial.println(tensPlace);
//      Serial.println(onesPlace);
      if(coordPlace == 2){
        tensPlace *= 10;
      }
      int finalNum = tensPlace + onesPlace;
//      Serial.println(finalNum);
      pathArray[counter] = finalNum;
      coordPlace = 0;
      tempCoords[0] = '0';
      tempCoords[1] = '0';
      counter++;
    }
  }
}

void movePiece(){
  if(readDone){ 
    bool secondHalf = false;
    bool posX = false;
    bool posY = false;
   for(int x = 0; x < (sizeof(pathArray) / sizeof(pathArray[0])); x++){
//     Serial.println(pathArray[x]);
    //  Serial.print(',');
   }
    
    for(int x = 0; x < (sizeof(pathArray) / sizeof(pathArray[0])); x+=2){
      if(pathArray[x] == '>' || (pathArray[x] == 0 && pathArray[x+1] == 62)){
//        Serial.println("break condition");
        break;
      }
      if(pathArray[x] == '<'){
        digitalWrite(magnetPin, LOW);
        secondHalf = true;
        delay(500);
        continue;
      }
      int yOffset = pathArray[x+1];
      int xOffset = pathArray[x];
      if(xOffset == 0 && yOffset == 0){
        destX = -10;
        destY1 = -100;
        destY2 = 100;
      }
      else if(x != 0 && !secondHalf){
        if(pathArray[x] - pathArray[x-2] > 0){
          posX = true;
          destX = (stepsPerSpaceX * xOffset) + constOffset;
        }
        else if(pathArray[x] - pathArray[x-2] < 0){
          posX = false;
          destX = (stepsPerSpaceX * xOffset) - constOffset;
        }
        else{
          if(posX){
           destX = (stepsPerSpaceX * xOffset) + constOffset;   
          }
          else{
            destX = (stepsPerSpaceX * xOffset) - constOffset;
          }
         
        }
        if(pathArray[x+1] - pathArray[x-1] > 0){
          posY = true;
          destY1 = (stepsPerSpaceY * yOffset) + constOffset;
          destY2 = -(stepsPerSpaceY * yOffset) - constOffset;
        }
        else if(pathArray[x+1] - pathArray[x-1] < 0){
          posY = false;
          destY1 = (stepsPerSpaceY * yOffset) - constOffset;
          destY2 = -(stepsPerSpaceY * yOffset) + constOffset;
        }
        else{
          if(posY){
           destY1 = stepsPerSpaceY * yOffset + constOffset;
           destY2 = -stepsPerSpaceY * yOffset - constOffset; 
          }
          else{
           destY1 = stepsPerSpaceY * yOffset - constOffset;
           destY2 = -stepsPerSpaceY * yOffset + constOffset;
          }
            
        }
      }
      else{
        destX = stepsPerSpaceX * xOffset; // grab first pair of x,y coords
        destY1 = stepsPerSpaceY * yOffset;
        destY2 = -stepsPerSpaceY * yOffset;
      }
      stepperX.moveTo(destX); // set destination in absoulte coords
      stepperY1.moveTo(destY1);
      stepperY2.moveTo(destY2);
      int count = 0;
      while(stepperX.distanceToGo() != 0 || stepperY1.distanceToGo() != 0 || stepperY2.distanceToGo() != 0){ // Check if all 3 have reached their destination. if not, keep going
        count++;
        stepperX.run();
        stepperY1.run();
        stepperY2.run();
        if (count > 600) {
          vTaskDelay(1);
          count = 0;
        }  
      }
      if(x == 0 || secondHalf == true){
        digitalWrite(magnetPin, HIGH);
        secondHalf = false;
        delay(500);
      }
    }
    digitalWrite(magnetPin, LOW);
    delay(500);
    stepperX.moveTo(start); // move motors back home after moving pieces
    stepperY1.moveTo(start);
    stepperY2.moveTo(start);
    int count = 0;
    while(stepperX.distanceToGo() != 0 || stepperY1.distanceToGo() != 0 || stepperY2.distanceToGo() != 0){ // Check if all 3 have reached their destination. if not, keep going
      stepperX.run();
      stepperY1.run();
      stepperY2.run();
      count++;
      if (count > 600) {
        vTaskDelay(1);
        count = 0;
      }
    } 
    capture = false; // piece has been captured
//    Serial.println("before flag");  
    moveDone = true;
    memset(pathArray, 0, sizeof(pathArray)); // clearing array
  }
}
