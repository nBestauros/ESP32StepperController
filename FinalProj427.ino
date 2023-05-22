#define I2C_ADDRESS 0x23
#define I2C_BUFFER 128

#define WHEELBASE 10 // cm
#define WHEEL_DIAMETER 5 // cm

#define MAX_FORWARD 12 // initial forward speed
#define STEPS_PER_REVOLUTION_LEFT 2200
#define STEPS_PER_REVOLUTION_RIGHT 2048 

#define TERMINATOR 0 // Ascii 0 is end of string.

#include <Wire.h>
#include <AccelStepper.h>
#include <MultiStepper.h>

#define PI 3.14159
// in1, in3, in2, in4
AccelStepper myStepperLeft(AccelStepper::HALF4WIRE, 9, 11, 10, 12);
// in1, in3, in2, in4
AccelStepper myStepperRight(AccelStepper::HALF4WIRE, A3, A1, A2, A0);

MultiStepper multiStepper;

long numStepsLeft = 0;
long numStepsRight = 0;
long posLeft = 0;
long posRight = 0;
long posList[2] = {0, 0};

volatile boolean isFreshI2C = true; // goes true whenever finishedReceivingI2C=true is handled. basicaly the next I2C byte is going to be the beginning of a new message.
volatile boolean finishedReceivingI2C = false; // goes true when the terminator character is received.
volatile byte lengthI2C = 0; // length of the spi message, not including EOT byte.
volatile byte i2cIncomingChars[I2C_BUFFER]; // store up to I2C_BUFFER bytes of info from I2C.

volatile byte oldSREG;

float wheelDiameterTimesPi;
int wheelBase;

float rads;
float turnStepsFactor;


void setup() {
  Wire.begin(I2C_ADDRESS);
  Wire.onReceive(handleI2C);
  Serial.begin(115200);
  myStepperLeft.setMaxSpeed(1000);
  myStepperRight.setMaxSpeed(1000);
  multiStepper.addStepper(myStepperLeft);
  multiStepper.addStepper(myStepperRight);

  posList[0] = 5000;
  posList[1] = 5000;

  multiStepper.moveTo(posList);

  numStepsLeft = 30000;
  numStepsRight = 30000;
  wheelDiameterTimesPi=WHEEL_DIAMETER * PI;
  wheelBase = WHEELBASE;
  interrupts();
}

void handleI2C(int byteCount) {
  noInterrupts();
  Serial.print("New Data Available: ");
  if(isFreshI2C) {
    lengthI2C = 0; // "reset" the i2c
  }
  isFreshI2C = false;
  while(Wire.available()) {
    char c = Wire.read();
    // Serial.println(c, HEX);
    
    i2cIncomingChars[lengthI2C] = c; // save the byte to the char array
    lengthI2C +=1;
    if(lengthI2C == 6) {
      finishedReceivingI2C = true;
    }
  }
  interrupts();
}

void calculatePos(long numSteps, int8_t degreesNum, int8_t direction) {
  rads = degreesNum * PI / 180;
  turnStepsFactor = rads * wheelBase / wheelDiameterTimesPi;

  long turnSteps = (long) (numSteps * turnStepsFactor);

  numStepsLeft = numSteps + turnSteps / 2;
  numStepsRight = numSteps - turnSteps / 2;

  if(direction == 1) {
    myStepperLeft.setCurrentPosition(0);
    myStepperRight.setCurrentPosition(0);
    myStepperLeft.setMaxSpeed(1000);
    myStepperRight.setMaxSpeed(1000);
    posList[0] = numStepsLeft;
    posList[1] = -1 * numStepsRight;
    multiStepper.moveTo(posList);
  }
  else if(direction == -1) {
    myStepperLeft.setCurrentPosition(0);
    myStepperRight.setCurrentPosition(0);
    myStepperLeft.setMaxSpeed(1000);
    myStepperRight.setMaxSpeed(1000);
    posList[0] = -1 * numStepsLeft;
    posList[1] = numStepsRight;
    multiStepper.moveTo(posList);
  }
}

void loop() {
  // Serial.println(digitalRead(16));
  if(finishedReceivingI2C) {
    // oldSREG = SREG;
    
    noInterrupts(); // dont try writing some new i2c stuff until this is done.
    // Serial.print("Finished String: ");
    // Serial.print(i2cIncomingChars[0], HEX);
    // Serial.print(i2cIncomingChars[1], HEX);
    // Serial.print(i2cIncomingChars[2], HEX);
    // Serial.print(i2cIncomingChars[3], HEX);
    // Serial.print(i2cIncomingChars[4], HEX);
    // Serial.print(i2cIncomingChars[5], HEX);

    // byte i = 0;
    // while(i<I2C_BUFFER && i2cIncomingChars[i] != 0x0) {
    //   Serial.print(i2cIncomingChars[i]);
    //   i=i+1;
    // }

    long dist;
    int8_t angle;

    switch(i2cIncomingChars[0]) {
      case 0x00: // forward case
        dist = *(long*)&i2cIncomingChars[1];
        myStepperLeft.setCurrentPosition(0);
        myStepperRight.setCurrentPosition(0);
        myStepperLeft.setMaxSpeed(1000);
        myStepperRight.setMaxSpeed(1000);
        posList[0] = -1 * dist;
        posList[1] = dist;
        multiStepper.moveTo(posList);
        break;
      
      case 0x01: // reverse case
        dist = *(long*)&i2cIncomingChars[1];
        myStepperLeft.setCurrentPosition(0);
        myStepperRight.setCurrentPosition(0);
        myStepperLeft.setMaxSpeed(1000);
        myStepperRight.setMaxSpeed(1000);
        posList[0] = dist;
        posList[1] = -1 * dist;
        multiStepper.moveTo(posList);
        break;
      
      case 0x02: // turning case forward
        dist = *(long*)&i2cIncomingChars[1];
        angle = i2cIncomingChars[5]; // -90 to +90
        calculatePos(dist, angle, 1);
        break;

      case 0x03: // turning case backward
        dist = *(long*)&i2cIncomingChars[1];
        angle = i2cIncomingChars[5]; // -90 to +90
        calculatePos(dist, angle, -1);
        break;
      default:
        Serial.println("Ruh roh, default in the i2c switch case");
    }

    Serial.println();

    // TODO: read the message here and actually do something about its contents
    // direction = direction * -1; // for now, any i2c message just reverses direction

    isFreshI2C = true; 
    finishedReceivingI2C = false;
    // SREG = oldSREG;
    interrupts();
  }
  // Serial.println("hi");
  multiStepper.run(); // Calls runSpeed() on all the managed steppers that have not acheived their target position.
}
