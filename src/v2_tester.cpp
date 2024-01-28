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
int roundNumber = 0;

// Function declarations
void displayFeedback(String message);
void displayGameOver();
void resetGame();
void updateDisplay();
void moveMotor();
void checkGuess(String guess);
void decreaseDeviation();

void setup() {
  Serial.begin(9600);
  Serial.println("Initializing...");
  
  // Set up motor
  motor.setSpeed(100); // Adjust the speed as needed
  
  // Set up pins
  pinMode(endstopPin, INPUT);
  pinMode(nearButtonPin, INPUT);
  pinMode(farButtonPin, INPUT);
  
  // Seed the random number generator with an analog pin value
  randomSeed(analogRead(A0));
}

void loop() {
  Serial.println("Looping...");
  
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
  Serial.println("Moving motor...");
  
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
  Serial.println("Checking guess...");
  
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
  Serial.println("Decreasing deviation...");
  
  // Decrease the deviation for the next round
  int randomChange = random(20) - 10;  // Random value between -10 and 10
  deviation += randomChange;  // Adjust the deviation with random value
}

void resetGame() {
  Serial.println("Resetting game...");
  
  // Reset game variables
  deviation = random(100) - 50;  // Random initial deviation between -50 and 50
  correctDeviation = 0;
  wrongGuessCount = 0;
  roundNumber++;
  
  // Move the motor back to the center
  motor.step(deviation, BACKWARD, SINGLE);
  
  // Display reset message
  displayFeedback("Game Reset");
  delay(2000);  // Display for 2 seconds
}

void updateDisplay() {
  Serial.println("Updating display...");
  
  // Update OLED display with round number, wrong guesses, and feedback messages
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_profont12);
    
    // Display round number
    u8g.drawStr(0, 15, ("Round: " + String(roundNumber)).c_str());
    
    // Display wrong guesses
    u8g.drawStr(0, 30, ("Wrong Guesses: " + String(wrongGuessCount)).c_str());
    
    // Display feedback messages
    u8g.drawStr(0, 45, "Feedback:");
    u8g.drawStr(0, 60, "------------------");
    u8g.drawStr(0, 75, " ");
    u8g.drawStr(0, 90, " ");
  } while (u8g.nextPage());
}

void displayFeedback(String message) {
  Serial.println("Displaying feedback...");
  
  // Display feedback on the OLED screen
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_profont12);
    
    // Display feedback messages
    u8g.drawStr(0, 75, message.c_str());
  } while (u8g.nextPage());
  
  delay(1000);  // Display for 1 second
}

void displayGameOver() {
  Serial.println("Game over...");
  
  // Display game over message with mean value
  int meanValue = correctDeviation / 3;
  String gameOverMessage = "Game Over! Mean Value: " + String(meanValue);
  
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_profont12);
    
    // Display game over message
    u8g.drawStr(0, 75, gameOverMessage.c_str());
  } while (u8g.nextPage());
}
