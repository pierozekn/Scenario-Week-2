  #include <Arduino.h>

  // number declarations:


  int fsrPins[] = {A0, A1, A2, A3}; // Define the pin and the respective output
  int maxForce[4]; // Define the maximum force for each sensor
  int maxForceValue = 0; // Define the maximum force for a particular reading
  const int numSensors = 4; // Define the amount of sensors there are
  unsigned long startTime = millis(); // Define the start time of a timer
  bool leftButtonPressed;
  bool rightButtonPressed;

  // function declarations here:
  void findMaxForce(); 
  void clearMaxForce();


  void setup() {
    Serial.begin(9600);
    leftButtonPressed = CircuitPlayground.leftButton();
    rightButtonPressed = CircuitPlayground.rightButton();
    if leftButtonPressed {
      findMaxForce();
      for (int i = 0; i < numSensors; i++) {
        Serial.println(maxForce[i]);
      }
    }
  }

  void loop() {
  }

  // function definitions here:

  void clearMaxForce() { // Resets the maximum force for each sensor
    for (int i = 0; i < numSensors; i++) { // For each sensor
      maxForce[i] = 0; // Reset the force back to zero
    }
  }

  void findMaxForce(){ // Find the maximum force through the sensor 
    clearMaxForce(); // Clear the maximum force if there was one to begin with
      for (int i = 0; i < numSensors; i++) { // For each sensor
          while (millis() -  startTime < 1000){ // Start a 5 second time
            int reading = analogRead(fsrPins[i]); // Read the force from the sensor
            if (reading > maxForce[i]) { // If the reading is bigger than the maximum force
              maxForce[i] = reading; // New reading replaces the old maximum force
        }
      }
    }
  }

  void findForceThresholdValue(){ // Finding the threshold value
    for (int i = 0; i < numSensors; i++) {
      maxForce[i]  = maxForce[i] * 0.8;
    }
  }

