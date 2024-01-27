#include <Arduino.h>
#include <Wire.h>
#include <U8glib.h>
#include <AFMotor.h>

// U8glib OLED Display
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0 | U8G_I2C_OPT_FAST);  // Adjust the device number if needed

// Define the stepper motor connections
AF_Stepper motor(200, 1);

// Define the button pins
#define closeButtonPin 2
#define farButtonPin 3

bool testEnded = false;
int targetPosition = 0;
int currentPosition = 0;
int correctResponses = 0;
int consecutiveWrongResponses = 0;
int depthDifference = 100;
const int maxConsecutiveWrongResponses = 3;

// Function to wait for the subject's response with timeout
int waitForResponseWithTimeout() {
  unsigned long startTime = millis();  // Record the start time

  while (millis() - startTime < 10000) {  // Timeout after 10 seconds
    if (digitalRead(closeButtonPin) == LOW) {
      currentPosition = targetPosition - depthDifference / 2;
      // Add code to move the motor to the endstop
      motor.setSpeed(100); // Set the motor speed
      motor.step(-200, FORWARD, SINGLE); // Move the motor to the endstop
      return 1;  // Subject pressed "Close" button
    } else if (digitalRead(farButtonPin) == LOW) {
      currentPosition = targetPosition + depthDifference / 2;
      // Add code to move the motor to the endstop
      motor.setSpeed(100); // Set the motor speed
      motor.step(200, FORWARD, SINGLE); // Move the motor to the endstop
      return 0;  // Subject pressed "Far" button
    }
  }

  // Timeout reached, return a default value (you can adjust as needed)
  return -1;
}

void setup() {
  // Set up OLED display
  u8g.begin();
  u8g.setFont(u8g_font_unifont);  // You can change the font if needed

  // Set up stepper motor
  motor.setSpeed(100);  // Set the speed in rpm

  // Set up buttons
  pinMode(closeButtonPin, INPUT_PULLUP);
  pinMode(farButtonPin, INPUT_PULLUP);

  Serial.begin(9600);
}

void loop() {
  if (!testEnded) {
    // Move the center pole to a random position
    int waitForResponseWithTimeout(); // Declare the waitForResponseWithTimeout() function

    targetPosition = random(500, 1000);  // Larger depth values initially

    int stepsToMove = targetPosition - currentPosition;

    // Display test information on OLED
    u8g.firstPage();
    do {
      u8g.drawStr(0, 10, "Actual: ");
      u8g.drawStr(60, 10, String(currentPosition).c_str());
      u8g.drawStr(0, 25, "Target: ");
      u8g.drawStr(60, 20, String(targetPosition).c_str());
      u8g.drawStr(0, 40, String("Right: " + String(correctResponses)).c_str());
      u8g.drawStr(0, 60, String("Wrong: " + String(consecutiveWrongResponses)).c_str());
    } while (u8g.nextPage());

    // Print motor movement to serial monitor
    Serial.print("Moving ");
    Serial.print(abs(stepsToMove));
    Serial.print(" steps ");
    Serial.println((stepsToMove > 0) ? "forward" : "backward");

    // Move the center pole using the stepper motor
    motor.step(abs(stepsToMove), (stepsToMove > 0) ? FORWARD : BACKWARD, SINGLE);

    // Wait for the subject to respond
    int response = waitForResponseWithTimeout();

    // Check the correctness of the response
    if (response == (stepsToMove > 0)) {
      // If correct response, decrease the depth difference
      depthDifference = max(50, depthDifference - 10);
      consecutiveWrongResponses = 0;
    } else {
      consecutiveWrongResponses++;
      if (consecutiveWrongResponses == maxConsecutiveWrongResponses) {
        // If three consecutive wrong responses, stop the test
        Serial.println("Test Stopped. Results:");
        Serial.print("Total Correct Responses: ");
        Serial.println(correctResponses);
        Serial.print("Final Depth Perception: ");
        Serial.println(currentPosition);

        // Display end result on OLED
        u8g.firstPage();
        do {
          u8g.drawStr(0, 30, "Test Ended");
          u8g.drawStr(0, 50, "Correct: ");
          u8g.drawStr(70, 50, String(correctResponses).c_str());
          u8g.drawStr(0, 60, "Wrong: ");
          u8g.drawStr(70, 60, String(consecutiveWrongResponses).c_str());
        } while (u8g.nextPage());

        testEnded = true;
      }
    }

    // Update correct responses and move to the next iteration
    correctResponses++;

    delay(2000);  // Wait before the next iteration
  } else {
    // Display test has ended on OLED
    u8g.firstPage();
    do {
      u8g.drawStr(0, 30, "Test Ended");
      u8g.drawStr(0, 50, "Correct: ");
      u8g.drawStr(70, 50, String(correctResponses).c_str());
      u8g.drawStr(0, 60, "Wrong: ");
      u8g.drawStr(70, 60, String(consecutiveWrongResponses).c_str());
      u8g.drawStr(0, 80, "Press any button");
      u8g.drawStr(0, 90, "to restart");
    } while (u8g.nextPage());

    // Wait for any button press to reset the test
    if (digitalRead(closeButtonPin) == LOW || digitalRead(farButtonPin) == LOW) {
      testEnded = false;
      correctResponses = 0;
      consecutiveWrongResponses = 0;
      delay(1000);  // Avoid immediate button press recognition
    }
  }
}