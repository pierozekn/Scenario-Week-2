#include <Adafruit_CircuitPlayground.h>
#include <U8g2lib.h>
#include <Wire.h>

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, SCL, SDA, U8X8_PIN_NONE); // Constructs the OLED display
// declarations:

int fsrPins[] = {A0, A1, A2, A3}; // Define the pin and the respective output
int maxForce[4]; // Define the maximum force for each sensor
int thresholdValue[4]; // Define the maximum force for each sensor
int maxForceValue = 0; // Define the maximum force for a particular reading
const int numSensors = 4; // Define the amount of sensors there are
unsigned long startTime = millis(); // Define the start time of a timer
bool leftButtonPressed = false; // Determine if the left button is pressed
bool lastLeftButtonPressed = false; // Remembers previous state
bool rightButtonPressed = false; // Determine if the right button is pressed
bool lastRightButtonPressed = false; // Remembers previous state
int currentStep = 0; // Used in programRuntime to ensure case is reset when the program is ran again
int amountOfFingers[4]; //What fingers are chosen
int howManyFingers = 0; // Same for requiredNumber
int position = 5; // Initial position for when counting up to 5
int currentFinger = 0; // What finger is it on


// function declarations here:
void showCountdownScreen(int fingerNumber, int secondsLeft);
void findMaxForce();
void clearMaxForce();
void findForceThresholdValue();
void areTheyTouchingThePad ();
void programRuntime();

// function declarations for calibrating screen:
void welcomeScreen();
void calibratingScreen();
void exerciseScreen();
void CompletionScreen();
void nextFinger();
void fingersToPutDown();
void completedExerciseScreen();
void showRetryScreen() ;
void finishScreen();

void setup() {
  Serial.begin(9600);
  CircuitPlayground.begin();
  Wire.begin(); 
  u8g2.begin(); 
  randomSeed(analogRead(A6)); // 
}

void loop() {
  leftButtonPressed = CircuitPlayground.leftButton();
  rightButtonPressed = CircuitPlayground.rightButton();
  programRuntime();
}

// function definitions here:

void clearMaxForce() { // Resets the maximum force for each sensor
  for (int i = 0; i < numSensors; i++) { // For each sensor
  maxForce[i] = 0; // Reset the force back to zero
  }
}

void showCountdownScreen(int fingers[], int fingerCount, int secondsLeft) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(5, 20, "Press fingers:");
  int x = 5;
  for (int i = 0; i < fingerCount; i++) {
    u8g2.setCursor(x, 40);
    u8g2.print(fingers[i]);
    x += 15;
  }
  u8g2.setCursor(70, 40);
  u8g2.print(secondsLeft);
  u8g2.drawStr(80, 40, "s left");
  u8g2.sendBuffer();
}

void findMaxForce() { // Find the maximum force through the sensor 
  bool completionSuccess = false;
  while (!completionSuccess){

    clearMaxForce(); // Clear the maximum force if there was one to begin with

    for (int i = 0; i < numSensors; i++) { // For each sensor
      currentFinger = i + 1;
      initialiseScreen(nextFinger);
      delay(4000);

      unsigned long fingerStart = millis(); // resets to "now" for each finger
      
      while (millis() - fingerStart < 5000) { // Start a 5 second time
        unsigned long elapsed = millis() - fingerStart;
        int secondsLeft = 5 - (elapsed / 1000);
        int fingerArr[] = {i + 1};
        showCountdownScreen(fingerArr, 1, secondsLeft);


        int reading = analogRead(fsrPins[i]); // Read the force from the sensor
        if (reading > maxForce[i]) { // If the reading is bigger than the maximum force
          maxForce[i] = reading; // New reading replaces the old maximum force
        }
      delay(100);
      }
    Serial.println(maxForce[i]);
    } 
    initialiseScreen(completedExerciseScreen);
  completionSuccess = true;

    for (int i = 0; i < numSensors; i++) { // For each sensor
      if (maxForce[i] < 50){
        completionSuccess = false;
        showRetryScreen() ;
        break;
      }
    }
  }


  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(5, 20, "Press LEFT Button");
  u8g2.drawStr(5, 40, "to continue:");
  u8g2.sendBuffer();

  while (!CircuitPlayground.leftButton());         // wait for press
  while (CircuitPlayground.leftButton());          // wait for release
} 


void findForceThresholdValue(){ // Finding the threshold value
  for (int i = 0; i < numSensors; i++) {
  thresholdValue[i]  = maxForce[i] * 0.2;
  }
}

void initialiseScreen(void (*drawingFunction)()) { // Drawing function - What should the screen says
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  drawingFunction();
  u8g2.sendBuffer();
}

// function declerations for screens: 
void welcomeScreen(){ 
  u8g2.drawStr(5, 20, "Welcome to...");
  u8g2.drawStr(5, 40, "Press LEFT Button");
}

void calibratingScreen(){ 
  u8g2.drawStr(5, 20, "Calibrating...");
  u8g2.drawStr(5, 40, "Squeeze each finger");
}

void exerciseScreen(){
  u8g2.drawStr(5, 20, "Put Down fingers:");
  int position = 5;
  for (int i = 0; i < howManyFingers; i++) {
    u8g2.setCursor(position, 40);
    u8g2.print(amountOfFingers[i]);
    position += 15;
  }
}

void completionScreen(){
  u8g2.drawStr(5, 20, "Great job!");
  u8g2.drawStr(5, 40, "Exercise complete");
}

void nextFinger() { 
  u8g2.drawStr(5, 20, "Now press finger:");
  u8g2.setCursor(5, 40);
  u8g2.print(currentFinger);
}

void completedExerciseScreen(){
  u8g2.drawStr(5, 20, "Great job!");
  u8g2.drawStr(5, 40, "Calibration complete");
}

void finishScreen(){
  u8g2.drawStr(5, 20, "you have great touch!");
  u8g2.drawStr(5, 40, "Tap LEFT to retry!");
}

void showRetryScreen() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(5, 20, "Please retry:");
  u8g2.sendBuffer();
  delay(4000);
}

// function for exercising the fingers

void generateSet() {
  memset(amountOfFingers, 0, sizeof(amountOfFingers)); // Reset all values in amountOfFingers
  howManyFingers = random(1, 5); // picks how many fingers

  // shuffle fingers
  int pool[4] = {1, 2, 3, 4}; 
  for (int i = 3; i > 0; i--) {
    int j = random(0, i + 1);
    int t = pool[i];
    pool[i] = pool[j];
    pool[j] = t;
  }
  // pick the fingers
  for (int i = 0; i < howManyFingers; i++) {
    amountOfFingers[i] = pool[i];
  }
}


void areTheyTouchingThePad (){
  bool completionSuccess = false;
    while (!completionSuccess){
      completionSuccess = true;

        int totals[4] = {0, 0, 0, 0};
        int count = 0;
        unsigned long fingerStart = millis(); // resets to "now" for each finger    
        
        while (millis() - fingerStart < 5000) { // Start a 5 second time
          unsigned long elapsed = millis() - fingerStart;
          int secondsLeft = 5 - (elapsed / 1000);
          showCountdownScreen(amountOfFingers, howManyFingers, secondsLeft);

          for (int i = 0; i < howManyFingers; i++) {
                  int whatFingers = amountOfFingers[i] - 1;
                  totals[i] += analogRead(fsrPins[whatFingers]);
                }
                count++;
                delay(100);
      }

    for (int i = 0; i < howManyFingers; i++) {
      int whatFingers = amountOfFingers[i] - 1;
      int reading = (count > 0) ? totals[i] / count : 0;
      if (reading < thresholdValue[whatFingers]) {
        completionSuccess = false;
        showRetryScreen();
        break;
      }
    }
  

  }
  initialiseScreen(completionScreen);
  delay(4000);

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(5, 20, "Press LEFT Button");
  u8g2.drawStr(5, 40, "to continue:");
  u8g2.sendBuffer();
  delay(4000);

  while (!CircuitPlayground.leftButton());         // wait for press
  while (CircuitPlayground.leftButton());          // wait for release
}

void programRuntime() { // what the program runs
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


      delay(4000);
      // Calibrates the finger force, iterates through each of the fingers and calculates the maximum force
        findMaxForce();


      // also want to call force threshold value
        findForceThresholdValue();


        currentStep = 2; // increment the steps
        break;

      case 2: 
      // do the exercise
        generateSet();
        initialiseScreen(exerciseScreen);  
        areTheyTouchingThePad ();
        currentStep = 3;
        break;

      case 3:
      // complete the exercise
        initialiseScreen(finishScreen);
        currentStep = 0;
        break;

    }
  }
  lastLeftButtonPressed = leftButtonPressed; // state machine control
  delay(10); 
}


