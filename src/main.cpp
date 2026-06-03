#include <Adafruit_CircuitPlayground.h>
#include <U8g2lib.h>
#include <Wire.h>

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, SCL, SDA, U8X8_PIN_NONE); // Constructs the OLED display
// declarations:

int fsrPins[] = {A0, A1, A2, A3}; // Define the pin and the respective output
int maxForce[4]; // Define the maximum force for each sensor
int maxForceValue = 0; // Define the maximum force for a particular reading
const int numSensors = 4; // Define the amount of sensors there are
unsigned long startTime = millis(); // Define the start time of a timer
bool leftButtonPressed = false; // Determine if the left button is pressed
bool lastLeftButtonPressed = false; // Remembers previous state
bool rightButtonPressed = false; // Determine if the right button is pressed
bool lastRightButtonPressed = false; // Remembers previous state
int currentStep = 0;
int requiredNumbers[4];
int requiredCount = 0;

// function declarations here:
void findMaxForce();
void clearMaxForce();
void findForceThresholdValue();
void programRuntime();
void welcomeScreen();

// calibrating screen
void calibratingScreen();
void excerciseScreen();
void CompletionScreen();



void setup() {
Serial.begin(9600);
}

void loop() {
  leftButtonPressed = CircuitPlayground.leftButton();
  rightButtonPressed = CircuitPlayground.rightButton();
}

// function definitions here:

void calibratingScreen(){
  u8g2.drawStr(5, 20, "Calibrating...");
  u8g2.drawStr(5, 40, "Squeeze each finger");
}

void exerciseScreen(){
  u8g2.drawStr(5, 20, "Put Down fingers:");
  int x = 5;
  for (int i = 0; i < requiredCount; i++) {
    u8g2.setCursor(x, 40);
    u8g2.print(requiredNumbers[i]);
    x += 15;
  }
}

void completionScreen(){
  u8g2.drawStr(5, 20, "Great job!");
  u8g2.drawStr(5, 40, "Exercise complete");
}

void clearMaxForce() { // Resets the maximum force for each sensor
  for (int i = 0; i < numSensors; i++) { // For each sensor
  maxForce[i] = 0; // Reset the force back to zero
  }
}

void findMaxForce(){ // Find the maximum force through the sensor 
  clearMaxForce(); // Clear the maximum force if there was one to begin with
  for (int i = 0; i < numSensors; i++) { // For each sensor
    while (millis() -  startTime - 5000*i < 5000){ // Start a 5 second time
      int reading = analogRead(fsrPins[i]); // Read the force from the sensor
      if (reading > maxForce[i]) { // If the reading is bigger than the maximum force
        maxForce[i] = reading; // New reading replaces the old maximum force
        Serial.print(maxForce[i]);
      }
    }
  }
}


void findForceThresholdValue(){ // Finding the threshold value
  for (int i = 0; i < numSensors; i++) {
  maxForce[i]  = maxForce[i] * 0.8;
  }
}

void initialiseScreen(void (*drawingFunction)()) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  drawingFunction();
  u8g2.sendBuffer();
}

void welcomeScreen(){
  u8g2.drawStr(5, 20, "Welcome to...");
  u8g2.drawStr(5, 40, "Press LEFT Button");
}

void generateSet() {
  memset(requiredNumbers, 0, sizeof(requiredNumbers));
  requiredCount = random(1, 5);
  int pool[4] = {1, 2, 3, 4};
  for (int i = 3; i > 0; i--) {
    int j = random(0, i + 1);
    int t = pool[i];
    pool[i] = pool[j];
    pool[j] = t;
  }
  for (int i = 0; i < requiredCount; i++) {
    requiredNumbers[i] = pool[i];
  }
}

void programRuntime(){ // what the program runs
  if (leftButtonPressed == true && lastLeftButtonPressed == false){ //state machine control
    switch (currentStep){
      case 0:
      // program first runs... show screen 
        initialiseScreen (welcomeScreen);

        currentStep = 1; //increment step
        break;
      case 1:
      // set the screen to say "time to calibrate"
      initialiseScreen(calibratingScreen); //to calibrate the force
        
      // Calibrates the finger force, iterates through each of the fingers and calculates the maximum force
        findMaxForce();
      // also want to call force threshold value
        findForceThresholdValue();


        currentStep = 2; // increment the steps
        break;

      case 2:
      // do the exercise
        initialiseScreen(excerciseScreen);
        currentStep = 3;
        break;

      case 3:
      // complete the exercise
        initialiseScreen(completionScreen);
        currentStep = 4;
        break;

    lastLeftButtonPressed = leftButtonPressed; // state machine control
    delay(10); 
  }
}

}
