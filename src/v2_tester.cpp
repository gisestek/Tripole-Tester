#include <Arduino.h>
#include <Wire.h>
#include <AFMotor.h>
#include <U8glib.h>

// Define the motor shield
AF_Stepper motor(200, 1);

// Define the OLED display
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);

// Define variables
const int endstopPin = A1;      // Endstop switch pin (analog pin A1)
const int nearButtonPin = A2;   // Near button pin (analog pin A2)
const int farButtonPin = A3;    // Far button pin (analog pin A3)

int deviation = 0;
int correctDeviation = 0;
int wrongGuessCount = 0;

void setup() {
  Serial.begin(9600);
  
  // Set up motor
  motor.setSpeed(100); // Adjust the speed as needed
  
  // Set up pins
  pinMode(endstopPin, INPUT);
  pinMode(nearButtonPin, INPUT);
  pinMode(farButtonPin, INPUT);
}

void loop() {
  // Reset the game if both buttons are pressed
  if (analogRead(nearButtonPin) < 100 && analogRead(farButtonPin) < 100) {
    resetGame();
  }
  
  // Check for near button press
  if (analogRead(nearButtonPin) < 100) {
    checkGuess("near");
  }
  
  // Check for far button press
  if (analogRead(farButtonPin) < 100) {
    checkGuess("far");
  }
  
  // Move the motor and update the OLED display
  moveMotor();
  updateDisplay();
}

void moveMotor() {
  if (deviation != 0) {
    // Move the stepper motor based on the deviation
    motor.step(deviation, FORWARD, SINGLE);
    
    // Check for the endstop switch
    if (digitalRead(endstopPin) == LOW) {
      // If endstop is reached, reset deviation and give feedback
      deviation = 0;
      displayFeedback("Reached End");
    }
  }
}

void checkGuess(String guess) {
  if (guess == "near" && deviation > 0) {
    // Correct guess for near
    correctDeviation = deviation;
    displayFeedback("Correct!");
    decreaseDeviation();
  } else if (guess == "far" && deviation < 0) {
    // Correct guess for far
    correctDeviation = -deviation;
    displayFeedback("Correct!");
    decreaseDeviation();
  } else {
    // Wrong guess
    wrongGuessCount++;
    displayFeedback("Wrong Guess");
    
    if (wrongGuessCount >= 3) {
      // Game over, display mean value of three smallest right answers
      displayGameOver();
      delay(5000);  // Display for 5 seconds
      resetGame();
    }
  }
}

void decreaseDeviation() {
  // Decrease the deviation for the next round
  if (deviation > 0) {
    deviation -= 10;  // Adjust the decrement as needed
  } else if (deviation < 0) {
    deviation += 10;  // Adjust the decrement as needed
  }
}

void resetGame() {
  // Reset game variables
  deviation = 100;  // Starting with a larger deviation
  correctDeviation = 0;
  wrongGuessCount = 0;
  
  // Move the motor back to the center
  motor.step(deviation, BACKWARD, SINGLE);
  
  // Display reset message
  displayFeedback("Game Reset");
  delay(2000);  // Display for 2 seconds
}

void updateDisplay() {
  // Add your OLED display update code here
  // This function should update the display based on the current state
  // of the game, motor position, and feedback messages.
}

void displayFeedback(String message) {
  // Display feedback on the OLED screen
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_profont12);
    u8g.drawStr(0, 15, message.c_str());
  } while (u8g.nextPage());
  
  delay(1000);  // Display for 1 second
}

void displayGameOver() {
  // Display game over message with mean value
  int meanValue = correctDeviation / 3;
  String gameOverMessage = "Game Over! Mean Value: " + String(meanValue);
  
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_profont12);
    u8g.drawStr(0, 15, gameOverMessage.c_str());
  } while (u8g.nextPage());
}
