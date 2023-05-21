#define I2C_ADDRESS 0x23
#define I2C_BUFFER 128

#define WHEELBASE 10 // cm
#define WHEEL_DIAMETER 5 // cm

#define MAX_FORWARD 12 // initial forward speed
#define STEPS_PER_REVOLUTION_LEFT 2200
#define STEPS_PER_REVOLUTION_RIGHT 2048 

#define TERMINATOR 0 // Ascii 0 is end of string.

#include <Stepper.h>
#include <Wire.h>

#define PI 3.14159
// in1, in3, in2, in4
Stepper stepperLeft = Stepper(STEPS_PER_REVOLUTION_LEFT, A3, A1, A2, A0);
// in1, in3, in2, in4
Stepper stepperRight = Stepper(STEPS_PER_REVOLUTION_RIGHT, 10, 6, 9, 5);
long numStepsLeft = 0;
long numStepsRight = 0;

volatile boolean isFreshI2C = true; // goes true whenever finishedReceivingI2C=true is handled. basicaly the next I2C byte is going to be the beginning of a new message.
volatile boolean finishedReceivingI2C = false; // goes true when the terminator character is received.
volatile byte lengthI2C = 0; // length of the spi message, not including EOT byte.
volatile byte i2cIncomingChars[I2C_BUFFER]; // store up to I2C_BUFFER bytes of info from I2C.

volatile short direction = 1; // Direction = 1 is forward, direction = -1 is backward.

volatile byte oldSREG;

float wheelDiameterTimesPi;
int wheelBase;

float rads;
float turnStepsFactor;


void setup() {
  Wire.begin(I2C_ADDRESS);
  Wire.onReceive(handleI2C);
  Serial.begin(115200);
  stepperLeft.setSpeed(MAX_FORWARD);
  stepperRight.setSpeed(MAX_FORWARD);
  numStepsLeft = 30000;
  numStepsRight = 30000;
  wheelDiameterTimesPi=WHEEL_DIAMETER * PI;
  wheelBase = WHEELBASE;
  interrupts();
}

void handleI2C(int byteCount) {
  noInterrupts();
  // Serial.print("New Data Available: ");
  if(isFreshI2C) {
    lengthI2C = 0; // "reset" the i2c
  }
  isFreshI2C = false;
  while(Wire.available()) {
    char c = Wire.read();
    // Serial.println(c, HEX);
    // if (c == TERMINATOR) {
    //   finishedReceivingI2C = true;
    //   // dont do an else here, we could also use the terminator to find the end of the string.
    // }
    
    i2cIncomingChars[lengthI2C] = c; // save the byte to the char array
    lengthI2C +=1;
    if(lengthI2C == 6) {
      finishedReceivingI2C = true;
    }
  }


  interrupts();
}

void calculateStepsAndRPM(long numSteps, int8_t degreesNum) {
  rads = degreesNum * PI / 180;
  turnStepsFactor = rads * wheelBase / wheelDiameterTimesPi;

  long turnSteps = (long) (numSteps * turnStepsFactor);

  numStepsLeft = numSteps + turnSteps / 2;
  numStepsRight = numSteps - turnSteps / 2;

  Serial.print("numStepsLeft: ");
  Serial.println(numStepsLeft);
  Serial.print("numStepsRight: ");
  Serial.println(numStepsRight);

  if(numStepsLeft <= numStepsRight) {
    //turning left
    double rpmLeft = ((double)numStepsLeft / numStepsRight) * MAX_FORWARD;
    stepperLeft.setSpeed(rpmLeft);
    Serial.print("RPMLeft: ");
    Serial.println(rpmLeft);
    stepperRight.setSpeed(MAX_FORWARD);
  }
  else {
    //turning right
    double rpmRight = ((double)numStepsRight / numStepsLeft) * MAX_FORWARD;
    stepperRight.setSpeed(rpmRight);
    Serial.print("RPMRight: ");
    Serial.println(rpmRight);
    stepperLeft.setSpeed(MAX_FORWARD);
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
        direction = 1;
        dist = *(long*)&i2cIncomingChars[1];
        // Serial.println(dist);
        stepperLeft.setSpeed(MAX_FORWARD);
        stepperRight.setSpeed(MAX_FORWARD);
        numStepsLeft = dist;
        numStepsRight = dist;
        break;
      
      case 0x01: // reverse case
        direction = -1; // go backwards
        dist = *(long*)&i2cIncomingChars[1];
        stepperLeft.setSpeed(MAX_FORWARD);
        stepperRight.setSpeed(MAX_FORWARD);
        numStepsLeft = dist;
        numStepsRight = dist;
        break;
      
      case 0x02: // turning case forward
        direction = 1;
        dist = *(long*)&i2cIncomingChars[1];
        angle = i2cIncomingChars[5]; // -90 to +90
        calculateStepsAndRPM(dist, angle);
        break;

      case 0x03: // turning case backward
        direction = -1;
        dist = *(long*)&i2cIncomingChars[1];
        angle = i2cIncomingChars[5]; // -90 to +90
        calculateStepsAndRPM(dist, angle);
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
  if(numStepsLeft > 0) {
    stepperLeft.step(1 * direction);
    numStepsLeft -= 1;
  }
  if(numStepsRight > 0) {
    stepperRight.step(direction); // might need to add a -1 here
    numStepsRight -= 1;
  }

  // delay(50);
  // stepperLeft.step(200);
  // delay(50);
  // stepperLeft.step(-200);
  // delay(50);
  // stepperRight.step(200);
  // delay(50);
  // stepperRight.step(-200);
  //   Serial.println("hey");

}
