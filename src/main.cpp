#include <Adafruit_CircuitPlayground.h>
#include <U8g2lib.h>
#include <Wire.h>

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, SCL, SDA, U8X8_PIN_NONE); // Constructs the OLED display

// declarations:

int fsrPins[] = {A0, A1, A3, A6}; // Define the pin and the respective output
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
int howManyFingers = 0; // How many fingers are chosen
int position = 5; // Initial position for when counting up to 5
int currentFinger = 0; // What finger is it on
int difficulty = 0; // Showcase the amount of times it loops
int currentDifficulty = 0; // Shows the current difficulty being chosen
int repCounts[] = {2, 5, 10}; // reps for each difficulty
const char* difficultyNames[] = {"Easy", "Medium", "Hard"};
int confirmedDifficulty = 0; // Confirms the diffculty
int reps = repCounts[confirmedDifficulty]; // Confirms the amount of reps
int totals[4] = {0, 0, 0, 0}; //Used in calculating averages
int count = 0; // Used to find the average
int noiseBaseline[4] = {0, 0, 0, 0}; // used in baseline calculations

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
void difficultyScreen();
void removingSystematicErrorsScreen();


void setup() {
  Serial.begin(9600);
  CircuitPlayground.begin();
  Wire.begin(); 
  u8g2.begin(); 
  randomSeed(analogRead(A7)); // 
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

void showCountdownScreen(const char* message, int fingers[], int fingerCount, int secondsLeft) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(5, 20, message);
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
        showCountdownScreen("Squeeze Fingers:", fingerArr, 1, secondsLeft);

        int raw = analogRead(fsrPins[i]);
        int reading = map(raw, noiseBaseline[i], 1023, 0, 1023);
        reading = constrain(reading, 0, 1023);
        if (reading > maxForce[i]) { // If the reading is bigger than the maximum force
          maxForce[i] = reading; // New reading replaces the old maximum force
        }
      delay(100);
      }
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

void selectDifficulty(){
  if (currentStep != 3) return;

  if (rightButtonPressed == true && lastRightButtonPressed == false){ //state machine control
    switch (currentDifficulty){
      case 0:
      // easy screen, controlled in programRuntime
      currentDifficulty = 1;
      initialiseScreen(difficultyScreen);
      break;
      case 1:
      // medium screen, controlled in programRuntime

      currentDifficulty = 2;
      initialiseScreen(difficultyScreen);
      break;
      case 2:
      // hard screen, controlled in programRuntime
      currentDifficulty = 0;
      initialiseScreen(difficultyScreen);
      break;

    }
  }
}

void areTheyTouchingThePad() {
  bool completionSuccess = false;
  while (!completionSuccess) {
    completionSuccess = true;
    findAverageOnEachFinger("Press fingers:");

    if (!checkRequiredFingers()) {
      completionSuccess = false;
      showRetryScreen();
    } else if (!checkUnrequiredFingers()) {
      completionSuccess = false;
      showRetryScreen();
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

  while (!CircuitPlayground.leftButton());
  while (CircuitPlayground.leftButton());
}

void programRuntime() { // what the program runs
  selectDifficulty();

  if (leftButtonPressed == true && lastLeftButtonPressed == false){ //state machine control
    switch (currentStep){ // Starts a switch so that it only carries on when the leftButton is pressed
      case 0:
      // program first runs... show screen 
        initialiseScreen (welcomeScreen);
        currentStep = 1; //increment step
        break;
      case 1:
        // set the screen to say "time to calibrate"
        initialiseScreen(calibratingScreen); //to calibrate the force
        delay(4000);
        initialiseScreen(removingSystematicErrorsScreen); //to calibrate the force
        delay(4000);

        removingSystematicErrors();
        delay(4000);
      // Calibrates the finger force, iterates through each of the fingers and calculates the maximum force
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

void removingSystematicErrors(){  
  howManyFingers = 4;
  for (int i = 0; i < howManyFingers; i++) {
    amountOfFingers[i] = i + 1;
  }
  findAverageOnEachFinger("Don't touch!");

  for (int i = 0; i < howManyFingers; i++) {
    noiseBaseline[i] = (count > 0) ? totals[i] / count : 0;
  }
}

void findAverageOnEachFinger(const char* message){
  memset(totals, 0, sizeof(totals)); // reset before each use
  unsigned long fingerStart = millis(); // resets to "now" for each finger    
  count = 0;

  while (millis() - fingerStart < 5000) { // Start a 5 second time
    unsigned long elapsed = millis() - fingerStart;
    int secondsLeft = 5 - (elapsed / 1000);
    showCountdownScreen(message, amountOfFingers, howManyFingers, secondsLeft);

    for (int i = 0; i < howManyFingers; i++) {
        int whatFingers = amountOfFingers[i] - 1;
        totals[i] += analogRead(fsrPins[whatFingers]);
      }
      count++;
      delay(100);
  }

}

// TODO: Make sure the correct fingers are pushed down

bool checkRequiredFingers() {
  for (int i = 0; i < howManyFingers; i++) {
    int whatFingers = amountOfFingers[i] - 1;
    int raw = (count > 0) ? totals[i] / count : 0;
    int reading = map(raw, noiseBaseline[whatFingers], 1023, 0, 1023);
    reading = constrain(reading, 0, 1023);
    if (reading < thresholdValue[whatFingers]) {
      return false;
    }
  }
  return true;
}

bool checkUnrequiredFingers() {
  for (int i = 0; i < numSensors; i++) {
    bool isRequired = false;
    for (int j = 0; j < howManyFingers; j++) {
      if (amountOfFingers[j] - 1 == i) {
        isRequired = true;
        break;
      }
    }
    if (!isRequired) {
      int raw = analogRead(fsrPins[i]);
      int reading = map(raw, noiseBaseline[i], 1023, 0, 1023);
      reading = constrain(reading, 0, 1023);
      if (reading > thresholdValue[i]) {
        return false;
      }
    }
  }
  return true;
}