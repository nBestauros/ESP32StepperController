// defines pins numbers
const int trigPin = A5;
const int echoPin = A4;
// defines variables
long duration;
int distance;
const int numSamples = 25;
int samples[numSamples];
int sampleIndex = 0;
int totalDistance = 0;
int firstOver = 0;

void setup() {
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  pinMode(13, OUTPUT);//If distance is 0-10
  Serial.begin(9600); // Starts the serial communication
}

void loop() {
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  digitalWrite(13, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2;
  
  // Add the current distance to the rolling array
  totalDistance -= samples[sampleIndex];
  samples[sampleIndex] = distance;
  totalDistance += samples[sampleIndex];
  sampleIndex = (sampleIndex + 1) % numSamples;
  if(sampleIndex == 0){
    firstOver = 1; // if we have collected the first 100 samples, then we can start using this data
  }
  
  // Calculate the average distance
  int averageDistance = totalDistance / numSamples;
  
  if (firstOver == 1 && averageDistance >= 0 && averageDistance <= 75) {
    digitalWrite(13, HIGH);
  } 
  
  // Prints the average distance on the Serial Monitor
  Serial.print("Average Distance: ");
  Serial.println(averageDistance);
  
}
