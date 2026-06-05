#include <Adafruit_CircuitPlayground.h>
#include <Wire.h>
#include <U8g2lib.h>

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, SCL, SDA, U8X8_PIN_NONE); // Constructs the OLED display

// declarations:

int fsrPins[] = {A0, A1, A3}; // Define the pin and the respective output
int maxForce[3]; // Define the maximum force for each sensor
int thresholdValue[3]; // Define the maximum force for each sensor
int maxForceValue = 0; // Define the maximum force for a particular reading
const int numSensors = 3; // Define the amount of sensors there are
unsigned long startTime = millis(); // Define the start time of a timer
bool leftButtonPressed = false; // Determine if the left button is pressed
bool lastLeftButtonPressed = false; // Remembers previous state
bool rightButtonPressed = false; // Determine if the right button is pressed
bool lastRightButtonPressed = false; // Remembers previous state
int currentStep = 0; // Used in programRuntime to ensure case is reset when the program is ran again
int amountOfFingers[3]; //What fingers are chosen
int howManyFingers = 0; // How many fingers are chosen
int position = 5; // Initial position for when counting up to 5
int currentFinger = 0; // What finger is it on
int currentDifficulty = 0; // Shows the current difficulty being chosen
int repCounts[] = {2, 5, 10}; // reps for each difficulty
const char* difficultyNames[] = {"Easy", "Medium", "Hard"};
int confirmedDifficulty = 0; // Confirms the diffculty
int reps = repCounts[confirmedDifficulty]; // Confirms the amount of reps
int totals[3] = {0, 0, 0}; //Used in calculating averages
int count = 0; // Used to find the average
int noiseBaseline[3] = {0, 0, 0}; // used in baseline calculations

// function declarations here:
void showCountdownScreen(const char* message, int fingers[], int fingerCount, int secondsLeft);
void findMaxForce();
void clearMaxForce();
void findForceThresholdValue();
void areTheyTouchingThePad ();
void programRuntime();
void selectDifficulty();
void findAverageOnEachFinger(const char* message);
void removingSystematicErrors();
bool checkRequiredFingers();
bool checkUnrequiredFingers();

// function declarations for calibrating screen:
void initialiseScreen(void (*drawingFunction)());
void welcomeScreen();
void calibratingScreen();
void exerciseScreen();
void completionScreen();
void nextFingerScreen();
void completedExerciseScreen();
void showRetryScreen() ;
void finishScreen();
void difficultyScreen();
void removingSystematicErrorsScreen();
void pressLeftToContinueScreen();

void setup() {
  Serial.begin(9600);
  CircuitPlayground.begin();
  Wire.begin(); 
  randomSeed(analogRead(A7)); // Makes a random seed from random noise from the A7 port
}

void loop() { // Program is run by calling programRuntime 
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

// Responsible for showing a countdown screen to display a certain message, amount of fingers and how many seconds left determines by the function
void showCountdownScreen(const char* message, int fingers[], int fingerCount, int secondsLeft) { 
  u8g2.clearBuffer(); // Clear screen
  u8g2.setFont(u8g2_font_ncenB08_tr); //Set font
  u8g2.drawStr(5, 20, message); //Draw the message e.g. Press fingers! or Don't press anything!
  int x = 5; // Sets the position for the timer
  for (int i = 0; i < fingerCount; i++) { //Change the positions 
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
  bool completionSuccess = false; //Assume copmetionSuccess is false when function is called
  while (!completionSuccess){ //WHile completionSuccess is false

    clearMaxForce(); // Clear the maximum force if there was one to begin with

    for (int i = 0; i < numSensors; i++) { // For each sensor
      currentFinger = i + 1; // As current finger goes from 1-4 and numSensors is 0-3, must be done to ensure consistency and general readbility
      nextFingerScreen(); // Changes screen to show the next finger
      delay(4000);

      unsigned long fingerStart = millis(); // resets to "now" for each finger
      int lastSecondShown = -1;

      while (millis() - fingerStart < 5000) { // Start a 5 second time
        unsigned long elapsed = millis() - fingerStart; //Sets the timer
        int secondsLeft = 5 - (elapsed / 1000); // Calculates seconds left
        int fingerArr[] = {i + 1}; // Shows which fingers need to be used (used in showing a countdown screen)
        showCountdownScreen("Squeeze Fingers:", fingerArr, 1, secondsLeft, lastSecondShown); // Show the countdown screen

        int effectiveMax = 1023 - noiseBaseline[i]; // Sets the effective Max so that the proportions are the same for each finger
        int raw = analogRead(fsrPins[i]); // Read the readings from the pin
        int reading = map(raw, noiseBaseline[i], 1023, 0, effectiveMax); // Map the readings to 0 to 1023, with the systematic error removed
        reading = constrain(reading, 0, effectiveMax); // Accounts for if raw is BELOW the noiseBaseline
        if (reading > maxForce[i]) { // If the reading is bigger than the maximum force
          maxForce[i] = reading; // New reading replaces the old maximum force
        }
      delay(100); // Makes sure arduino isn't overloaded
      }
    } 
  completionSuccess = true; // // Assume that the copmletion success is true

    for (int i = 0; i < numSensors; i++) { // For each sensor, check if the maximum force is an actual reading 
      if (maxForce[i] < 50){ // As 50 is considered a VERY ligh touch, its easy to assume that anything below 50 means that the sensor isn't working
        completionSuccess = false; // Therefore, completion success should be false
        showRetryScreen() ; // Show a retry screen
        break;
      }
    }
    if (completionSuccess) {
    completedExerciseScreen();
  }
  }

// Assuming that completion Sucess is true and ready to carry on, 

  pressLeftToContinueScreen();
  while (!CircuitPlayground.leftButton()); // wait for press
  while (CircuitPlayground.leftButton());  // wait for release
} 

void findForceThresholdValue(){ // Finding the threshold value
  for (int i = 0; i < numSensors; i++) {
  thresholdValue[i]  = maxForce[i] * 0.2; // for demonstration purposes, threshold value is chosen for 0.2, SHOULD BE UPDATED
  }
}

// function declerations for screens:  
void initialiseScreen(void (*drawingFunction)()) { // Drawing function - What should the screen says
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  drawingFunction();
  u8g2.sendBuffer();
}

void welcomeScreen(){ 
  u8g2.drawStr(5, 20, "Welcome to...");
  u8g2.drawStr(5, 40, "Press LEFT Button");
}

void calibratingScreen(){ 
  u8g2.drawStr(5, 20, "Calibrating...");
  u8g2.drawStr(5, 40, "Squeeze each finger");
}

void removingSystematicErrorsScreen(){ 
  u8g2.drawStr(5, 20, "Calibrating...");
  u8g2.drawStr(5, 40, "Don't touch anything!");
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

void pressLeftToContinueScreen(){
  u8g2.drawStr(5, 20, "Press LEFT Button");
  u8g2.drawStr(5, 40, "to continue:");
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

void difficultyScreen() {
  u8g2.drawStr(5, 20, "Difficulty:");
  u8g2.drawStr(5, 40, difficultyNames[currentDifficulty]);
}

// functions for exercising the fingers

// Function that generates the set of random fingers
void generateSet() {
  memset(amountOfFingers, 0, sizeof(amountOfFingers)); // Reset all values in amountOfFingers
  howManyFingers = random(1, 4); // picks how many fingers

  // shuffle fingers
  int pool[3] = {1, 2, 3};
  for (int i = 2; i > 0; i--) {
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

void selectDifficulty(){ // This selects the difficulty
  if (currentStep != 3) return; // this ONLY works when the currenStep is equal to 3 / state = 3. 

  if (rightButtonPressed == true && lastRightButtonPressed == false){ //state machine control
    switch (currentDifficulty){
      case 0:
      // show easy screen
      currentDifficulty = 1; // increment difficulty to 1 so when right button is rpessed, moves onto case 1. 
      initialiseScreen(difficultyScreen);
      break;
      case 1:
      // show medium screen
      currentDifficulty = 2;
      initialiseScreen(difficultyScreen);
      break;
      case 2:
      // show hard screen
      currentDifficulty = 0;
      initialiseScreen(difficultyScreen);
      break;

    }
  }
}

void areTheyTouchingThePad() { // Well..... are they touching the pad? 
  bool completionSuccess = false; // Assume they're not touching the pad
  while (!completionSuccess) { // While compeltion sucess = false,
    completionSuccess = true; // Assume a success
    findAverageOnEachFinger("Press fingers:"); // find the average force on each finger

    for (int i = 0; i < howManyFingers; i++) { // For every singer
      int whatFingers = amountOfFingers[i] - 1; // Changes the index from 1-4 to 0-3
      int raw = (count > 0) ? totals[i] / count : 0; // Calculate the average
      int reading = constrain(map(raw, noiseBaseline[whatFingers], 1023, 0, 1023), 0, 1023); //Map and constrain readings
    }

    if (!checkRequiredFingers()) { // look at each finger that SHOULD be pressed, if the return value is false:
      completionSuccess = false; // return false
      initialiseScreen(showRetryScreen); 
    // retry again
    } else if (!checkUnrequiredFingers()) { // look at each finger that SHOULDN'T be pressed, if the return value is false:
      completionSuccess = false; // return false
      initialiseScreen(showRetryScreen); // retry again
    }
  }

  initialiseScreen(completionScreen); // If ALL conditions are satifised, exercises are completed and good
  delay(4000);
  initialiseScreen(pressLeftToContinueScreen);

  while (!CircuitPlayground.leftButton()); // only continue if the left button is continue
  while (CircuitPlayground.leftButton()); 
}

void programRuntime() { // what the program runs
  initialiseScreen(selectDifficulty); // select the difficulty, only runs on case 3 (has to be initialised now becuase it uses right button and not left)

  if (leftButtonPressed == true && lastLeftButtonPressed == false){ //state machine control, only continues when left button is pressed
    switch (currentStep){ // Starts a switch so that it only carries on when the leftButton is pressed
      case 0:
      // program first runs... show screen 
        initialiseScreen(welcomeScreen);
        currentStep = 1; //increment step
        break;
      case 1:
        // set the screen to say "time to calibrate"
        initialiseScreen(removingSystematicErrorsScreen); //to calibrate the force
        delay(4000);
        removingSystematicErrors(); // Remove systematic errors
        delay(4000);
      // Calibrates the finger force, iterates through each of the fingers and calculates the maximum force

        initialiseScreen(calibratingScreen); //to calibrate the force
        delay(4000);

        findMaxForce();
      // also want to call force threshold value
        findForceThresholdValue();  
        currentStep = 2; // increment the steps
        break;

      case 2: 
      // Choose the difficulty screen
        initialiseScreen(difficultyScreen);
        currentStep = 3;
        break;
        
      case 3:
      // Confirm the difficulty, and then start the exercises
        confirmedDifficulty = currentDifficulty;  
        reps = repCounts[confirmedDifficulty];
        for (int i = 0; i < reps; i++){
          generateSet();
          initialiseScreen(exerciseScreen);  
          areTheyTouchingThePad ();
        }

        currentStep = 4;
        break;
        
      case 4:
      // complete the exercise

        initialiseScreen(finishScreen);
        currentStep = 0;
        break;

    }
  }

  lastLeftButtonPressed = leftButtonPressed; // state machine control
  lastRightButtonPressed = rightButtonPressed;
  delay(10); 
}

void removingSystematicErrors(){   // Remove all systematic errors
  howManyFingers = numSensors; // how many fingers = the amount of sensors
  for (int i = 0; i < howManyFingers; i++) { // for each sensor:
    amountOfFingers[i] = i + 1; // Change the index
  }
  findAverageOnEachFinger("Don't touch!"); //Find the average for each finger

  for (int i = 0; i < howManyFingers; i++) { // for each finger
    int avg = (count > 0) ? totals[i] / count : 0; // calculate the average force
    noiseBaseline[i] = (avg * 0.8 > 200) ? 0 : avg * 0.8; // take 80% of average so genuine presses aren't suppressed
  }
}

void findAverageOnEachFinger(const char* message){
  memset(totals, 0, sizeof(totals)); // reset before each use
  unsigned long fingerStart = millis(); // resets to "now" for each finger    
  count = 0; // change count to 0
  int lastSecondShown = -1; // Make sure timer onyl displays when the second is changed

  while (millis() - fingerStart < 5000) { // Start a 5 second time
    unsigned long elapsed = millis() - fingerStart; // Start a timer
    int secondsLeft = 5 - (elapsed / 1000); // Figure out the seconds left
    showCountdownScreen(message, amountOfFingers, howManyFingers, secondsLeft);

    for (int i = 0; i < howManyFingers; i++) { // For each finger:
        int whatFingers = amountOfFingers[i] - 1; // Change the index
        totals[i] += analogRead(fsrPins[whatFingers]); // Calculate the average
      }
      count++; // caluating average
      delay(100);
  }

}


bool checkRequiredFingers() { // Check the fingers that are meant to be pressed 
  for (int i = 0; i < howManyFingers; i++) { // For each finger:
    int whatFingers = amountOfFingers[i] - 1;  // Change the index (fingers are 1-4, what finger array index is 0-3)
    int raw = (count > 0) ? totals[i] / count : 0; //Get the average reading
    int reading = map(raw, noiseBaseline[whatFingers], 1023, 0, 1023); //Remove noise
    reading = constrain(reading, 0, 1023); //Clamps negatives to zeros 
    if (reading < thresholdValue[whatFingers]) { // If the readings are below the threshold value
      return false; // returns a false
    }
  }
  return true; // else, if the readings are above, return true
}

bool checkUnrequiredFingers() { // Check the fingers that aren't meant to be pressed
  for (int i = 0; i < numSensors; i++) { // for each sensor
    bool isRequired = false; // Assume every sensor is not required
    for (int j = 0; j < howManyFingers; j++) { // For each sensor
      if (amountOfFingers[j] - 1 == i) { isRequired = true; break; } // If the sensor is required, break
    }
    if (!isRequired) { // If it is not recording
      int raw = analogRead(fsrPins[i]); // take a live reading instead of using totals
      int reading = constrain(map(raw, noiseBaseline[i], 1023, 0, 1023), 0, 1023); // Map and constrain the readings
      if (reading > thresholdValue[i]) return false; // If the sensor is recording a genuine value, value = false
    } 
  }
  return true; // if every other condition is met, return true (so all sensors that are not required don't return a value)
}