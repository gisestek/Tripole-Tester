#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

// Jukka Pajula
// 2025
// Jääkäriprikaati, OPOS

// WiFi Access Point credentials
const char *ssid = "Stereovisiontest";
const char *password = "12345678";

// Web server on port 80
WebServer server(80);

// Servo setup
Servo servo;
const int servoPin = 13;

// Game variables
int centerPosition = 90;  // Center position of the servo
int deflection = 30;       // Initial large deflection
int wrongGuesses = 0;
const int maxWrongGuesses = 3;
bool gameOver = false;
bool gameStarted = false;
int currentPosition;
const int maxDeflection = 37; // Maximum deflection from center (total 75 degrees range)
String deflectionHistory = "";

void setup() {
    Serial.begin(115200);
    WiFi.softAP(ssid, password); // Create WiFi Access Point
    server.on("/guess", handleGuess); // Setup game route
    server.on("/", handleRoot); // Serve game page
    server.on("/restart", handleRestart); // Restart game
    server.on("/start", handleStartGame); // Start game
    server.begin();
    servo.attach(servoPin);
    resetGame();
}

void loop() {
    server.handleClient();
}

// Resets the game to the initial state
void resetGame() {
    deflection = 30; // Start with large deflection
    wrongGuesses = 0;
    gameOver = false;
    gameStarted = false;
    deflectionHistory = "";
    servo.write(centerPosition); // Reset servo to center on startup
}

void handleStartGame() {
    gameStarted = true;
    setResetMovement();
    setServo();
    server.sendHeader("Location", "/");
    server.send(303);
}

// Moves the servo to a far front or far back position before setting new position
void setResetMovement() {
    int resetPosition = centerPosition + (random(2) ? maxDeflection : -maxDeflection); // Large deflection reset
    servo.write(resetPosition);
    delay(500);
}

// Moves the servo to a new random position either in front or back from the center
void setServo() {
    if (gameOver) {
        currentPosition = centerPosition; // Reset servo to center at game over
    } else {
        currentPosition = centerPosition + (random(2) ? deflection : -deflection);
    }
    servo.write(currentPosition);
    Serial.println("Servo moved to: " + String(currentPosition));
}

// Serves the HTML page with a mobile-friendly UI in Finnish
void handleRoot() {
    String page = "<html><head>";
    page += "<meta charset='UTF-8'>"; // Enable Finnish characters
    page += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    page += "<style>body { font-family: Arial, sans-serif; text-align: center; margin: 0; padding: 20px; background-color: #f4f4f4; }";
    page += "h1 { color: #333; }";
    page += "button { font-size: 24px; padding: 15px; margin: 10px; width: 80%; max-width: 300px; border: none; border-radius: 10px; background-color: #007BFF; color: white; cursor: pointer; }";
    page += "button:hover { background-color: #0056b3; }";
    page += "</style></head><body>";
    page += "<h1>Stereonäkötesti</h1>";
    
    if (!gameStarted) {
        page += "<button onclick=\"location.href='/start'\">Aloitetaan</button>";
    } else if (gameOver) {
        page += "<h2>Testi ohi! Lopullinen poikkeama: " + String(deflection) + " yksikköä</h2>";
        page += "<button onclick=\"location.href='/restart'\">Aloita uusi peli</button>";
    } else {
        page += "<p>Onko keskimmäinen pylväs lähempänä vai kauempana kuin reunimmaiset pylväät?</p>";
        page += "<button onclick=\"location.href='/guess?answer=front'\">Lähellä</button>";
        page += "<button onclick=\"location.href='/guess?answer=back'\">Kaukana</button>";
    }
    page += "<p>" + deflectionHistory + "</p>";
    page += "</body></html>";
    server.send(200, "text/html", page);
}

// Handles player's guess and updates game state
void handleGuess() {
    if (gameOver || !gameStarted) {
        server.sendHeader("Location", "/");
        server.send(303);
        return;
    }
    
    String answer = server.arg("answer");
    bool correct = (currentPosition > centerPosition && answer == "front") ||
                   (currentPosition < centerPosition && answer == "back");
    
    if (correct) {
        deflection = max(0.5, deflection * 0.75); // Reduce deflection on correct guess down to sub-degree level
    } else {
        wrongGuesses++;
        deflection = min((double)maxDeflection, deflection * 1.5); // Increase deflection on wrong guess
        if (wrongGuesses >= maxWrongGuesses) {
            gameOver = true; // End game after max wrong guesses
            setServo(); // Move servo to center at game over
        }
    }
    deflectionHistory += String(deflection) + " → ";
    setResetMovement(); // Perform reset movement before next guess
    setServo(); // Move servo to a new position
    server.sendHeader("Location", "/");
    server.send(303);
}

// Handles restarting the game
void handleRestart() {
    resetGame();
    server.sendHeader("Location", "/");
    server.send(303);
}
