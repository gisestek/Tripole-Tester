#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>
#include <vector>   // Added for storing results
#include <string>   // Added for string manipulation (optional but good practice)
#include <sstream>  // Added for CSV generation

// Jukka Pajula
// 2025
// Jääkäriprikaati, OPOS
// Versio 2.2 - Korjattu puuttuvat aaltosulut

// WiFi Access Point credentials
const char *ssid = "Stereovisiontest";
const char *password = "12345678"; // Käytä vahvempaa salasanaa tarvittaessa

// Web server on port 80
WebServer server(80);

// Servo setup
Servo servo;
const int servoPin = 13;

// --- Test Variables ---
int centerPosition = 90;  // Servon keskikohta (astetta)
double deflection = 30.0; // Alkupoikkeama (astetta) - Käytetään double tarkkuutta varten
int wrongGuesses = 0;
const int maxWrongGuesses = 3;
bool gameOver = false;
bool gameStarted = false;
int currentPosition; // Servon nykyinen asento (astetta)
const double maxDeflection = 37.0; // Maksimipoikkeama keskikohdasta (astetta)
String deflectionHistory = "";
String currentSubjectName = ""; // Nykyisen koehenkilön nimi

// --- Data Storage ---
struct ResultData {
  String name;
  double finalDeflection;
  String history;
  // Voit lisätä aikaleiman tarvittaessa: unsigned long timestamp;
};
std::vector<ResultData> allResults; // Vektori kaikkien testien tuloksille

// --- HTML & CSS ---
const char* HTML_HEADER = "<!DOCTYPE html><html><head>"
                          "<meta charset='UTF-8'>"
                          "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
                          "<title>Stereonäkötesti</title>"
                          "<style>"
                          "body { font-family: Arial, sans-serif; text-align: center; margin: 0; padding: 20px; background-color: #f4f4f4; }"
                          "h1, h2, h3 { color: #333; }"
                          "p { color: #555; }"
                          ".container { max-width: 600px; margin: auto; background-color: #fff; padding: 20px; border-radius: 10px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); }"
                          "button, input[type=submit] { font-size: 24px; padding: 15px 20px; margin: 10px 5px; width: calc(80% - 10px); max-width: 300px; border: none; border-radius: 10px; background-color: #007BFF; color: white; cursor: pointer; transition: background-color 0.3s ease; }"
                          "button:hover, input[type=submit]:hover { background-color: #0056b3; }"
                          "button.large-btn { width: calc(90% - 10px); padding: 25px 20px; font-size: 30px; }" // Larger buttons for subject
                          "button.secondary-btn { background-color: #6c757d; }"
                          "button.secondary-btn:hover { background-color: #5a6268; }"
                          "button.download-btn { background-color: #28a745; }"
                          "button.download-btn:hover { background-color: #218838; }"
                          "input[type=text] { font-size: 20px; padding: 10px; margin: 10px 0; width: calc(80% - 22px); max-width: 280px; border: 1px solid #ccc; border-radius: 5px; }"
                          "table { width: 100%; margin-top: 20px; border-collapse: collapse; }"
                          "th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }"
                          "th { background-color: #007BFF; color: white; }"
                          "tr:nth-child(even) { background-color: #f2f2f2; }"
                          ".status { font-weight: bold; margin: 15px 0; font-size: 1.1em; }"
                          ".error { color: red; font-weight: bold; }"
                          "</style></head><body><div class='container'>";

const char* HTML_FOOTER = "</div></body></html>";


// --- Function Declarations ---
void handleRoot();
void handleAdmin();
void handleSetName();
void handleStartGame();
void handleGuess();
void handleDownload();
void resetGame(bool keepName = false);
void setServo();
void setResetMovement();
String getAdminPageContent(String extraMessage = "");
String getSubjectPageContent();
String generateCSV();

// --- Setup ---
void setup() {
    Serial.begin(115200);
    Serial.println("Starting Stereovision Test Server...");

    // Servo setup
    servo.attach(servoPin);
    servo.write(centerPosition); // Aseta servo keskelle alussa
    delay(500); // Pieni viive servon asettumiselle

    // WiFi setup
    WiFi.softAP(ssid, password);
    Serial.print("Access Point SSID: ");
    Serial.println(ssid);
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());

    // Web server routes
    server.on("/", HTTP_GET, handleRoot);              // Koehenkilön sivu
    server.on("/admin", HTTP_GET, handleAdmin);        // Valvojan sivu
    server.on("/setname", HTTP_POST, handleSetName);   // Nimen asetus (valvoja)
    server.on("/start", HTTP_GET, handleStartGame);    // Testin aloitus (koehenkilö)
    server.on("/guess", HTTP_GET, handleGuess);        // Arvauksen käsittely (koehenkilö)
    server.on("/download", HTTP_GET, handleDownload);  // Tulosten lataus (valvoja)

    server.begin();
    Serial.println("HTTP server started");
    resetGame(true); // Nollaa pelitila, mutta ei välttämättä nimeä alussa
}

// --- Main Loop ---
void loop() {
    server.handleClient();
}

// --- Game Logic ---

// Nollaa testin tila
void resetGame(bool keepName /*= false*/) {
    Serial.println("Resetting game state...");
    deflection = 30.0; // Palauta alkupoikkeama
    wrongGuesses = 0;
    gameOver = false;
    gameStarted = false;
    deflectionHistory = "";
    if (!keepName) {
        currentSubjectName = ""; // Nollaa nimi vain jos keepName on false
        Serial.println("Current subject name cleared.");
    }
    servo.write(centerPosition); // Servo keskelle
    Serial.println("Game reset complete.");
}

// Aloittaa testin (kun koehenkilö painaa Aloita)
void handleStartGame() {
    if (currentSubjectName == "") {
        // Estä aloitus jos nimeä ei ole asetettu
        server.send(200, "text/html", String(HTML_HEADER) + "<h1>Odota</h1><p class='error'>Kokeen valvojan täytyy asettaa nimesi ensin /admin-sivulla.</p>" + HTML_FOOTER);
        return;
    }
    if (!gameStarted) {
        Serial.println("Starting game for: " + currentSubjectName);
        gameStarted = true;
        gameOver = false;
        wrongGuesses = 0;
        deflection = 30.0; // Varmista alkupoikkeama
        deflectionHistory = "";
        setResetMovement(); // Suorita nollausliike
        setServo();         // Aseta ensimmäinen testiasento
        Serial.println("Game started. Initial deflection: " + String(deflection));
    }
    // Ohjaa käyttäjä takaisin pääsivulle (joka näyttää nyt arvauspainikkeet)
    server.sendHeader("Location", "/");
    server.send(303);
}

// Siirtää servon kauas eteen/taakse ennen uutta asentoa
void setResetMovement() {
    int resetPosition = centerPosition + (random(2) ? maxDeflection : -maxDeflection);
    Serial.println("Performing reset movement to: " + String(resetPosition));
    servo.write(resetPosition);
    delay(500); // Anna servon liikkua
    servo.write(centerPosition); // Palaa keskelle hetkeksi (valinnainen, selkeyttää liikettä)
    delay(300);
}

// Siirtää servon uuteen testiasentoon
void setServo() {
    if (gameOver) {
        currentPosition = centerPosition; // Servo keskelle testin päätyttyä
    } else {
        // Randomly choose front or back
        bool goFront = random(2) == 1;
        currentPosition = centerPosition + (goFront ? (int)round(deflection) : -(int)round(deflection));
        // Varmista, ettei mennä yli rajojen (vaikka maxDeflection pitäisi estää tämä)
        currentPosition = constrain(currentPosition, centerPosition - maxDeflection, centerPosition + maxDeflection);
    }
    servo.write(currentPosition);
    Serial.println("Servo moved to: " + String(currentPosition) + " (Deflection: " + String(deflection) + ")");
}

// Käsittelee koehenkilön arvauksen
void handleGuess() {
    if (gameOver || !gameStarted) {
        server.sendHeader("Location", "/");
        server.send(303);
        return;
    }

    String answer = server.arg("answer"); // "front" tai "back"
    bool isFront = currentPosition > centerPosition; // Oliko sauva oikeasti edessä?

    Serial.print("Subject guessed: " + answer + ". Actual position was " + (isFront ? "front" : "back"));

    bool correct = (isFront && answer == "front") || (!isFront && answer == "back");

    if (correct) {
        Serial.println(" -> Correct!");
        deflection *= 0.75; // Pienennä poikkeamaa
        deflection = max(0.5, deflection); // Minimipoikkeama 0.5 astetta
    } else {
        Serial.println(" -> Incorrect!");
        wrongGuesses++;
        deflection *= 1.5; // Suurenna poikkeamaa
        deflection = min(maxDeflection, deflection); // Älä ylitä maksimipoikkeamaa
        Serial.println("Wrong guesses: " + String(wrongGuesses));
        if (wrongGuesses >= maxWrongGuesses) {
            gameOver = true;
            Serial.println("Game Over!");
            // Tallenna tulos
            ResultData result;
            result.name = currentSubjectName;
            result.finalDeflection = deflection; // Tallenna viimeisin (tai ehkä edellinen on parempi?) poikkeama
            result.history = deflectionHistory + String(deflection, 2); // Lisää viimeinen arvo historiaan
            allResults.push_back(result);
            Serial.println("Result saved for " + currentSubjectName + ". Final deflection approx: " + String(deflection));
            setServo(); // Siirrä servo keskelle pelin loputtua
        }
    }

    if (!gameOver) {
        deflectionHistory += String(deflection, 2) + " -> "; // Lisää historiaan (kahden desimaalin tarkkuudella)
        setResetMovement(); // Nollausliike
        setServo();         // Uusi asento
    }

    // Ohjaa takaisin pääsivulle
    server.sendHeader("Location", "/");
    server.send(303);
}


// --- Web Page Handlers ---

// Koehenkilön pääsivu (/)
void handleRoot() {
    server.send(200, "text/html", getSubjectPageContent());
}

// Valvojan sivu (/admin)
void handleAdmin() {
    server.send(200, "text/html", getAdminPageContent());
}

// Käsittelee nimen asettamisen valvojan sivulta
void handleSetName() {
    if (server.hasArg("subjectName")) {
        String newName = server.arg("subjectName");
        newName.trim(); // Poista ylimääräiset välilyönnit

        currentSubjectName = newName; // Aseta nimi (voi olla tyhjäkin, jos halutaan nollata)

        if (newName.length() > 0) {
             Serial.println("Admin set subject name to: " + currentSubjectName);
        } else {
             Serial.println("Admin cleared subject name.");
        }

        resetGame(true); // Nollaa pelitila, mutta pidä (mahdollisesti tyhjä) nimi

        // Ohjaa admin takaisin admin-sivulle näyttämään uusi tila
        server.sendHeader("Location", "/admin");
        server.send(303);
        return;

    } else {
        // Jos subjectName-argumenttia ei jostain syystä tule POST-pyynnössä
        String errorMsg = "<p class='error'>Virhe nimen käsittelyssä!</p>";
        server.send(200, "text/html", getAdminPageContent(errorMsg));
    }
}


// Luo ja lähettää CSV-tiedoston
void handleDownload() {
    Serial.println("Generating CSV file for download...");
    String csvData = generateCSV();
    server.sendHeader("Content-Disposition", "attachment; filename=stereotest_results.csv");
    server.send(200, "text/csv", csvData);
    Serial.println("CSV file sent.");
}

// --- HTML Generation ---

// Luo sisältö koehenkilön sivulle
String getSubjectPageContent() {
    String page = HTML_HEADER;
    page += "<h1>Stereonäkötesti</h1>";

    if (currentSubjectName == "") {
        page += "<p class='status'>Odota, että kokeen valvoja asettaa nimesi järjestelmään /admin-sivulla.</p>";
    } else if (!gameStarted) {
        page += "<p>Testi valmis alkamaan sinulle: <strong>" + currentSubjectName + "</strong></p>";
        page += "<form action='/start' method='get'><button type='submit' class='large-btn'>Aloita testi</button></form>";
    } else if (gameOver) {
        page += "<h2>Testi päättynyt!</h2>";
        // Näytetään viimeisin (pienin saavutettu tai suurin virheellinen) poikkeama
        page += "<p class='status'>Lopullinen tarkkuus: noin " + String(deflection, 2) + " astetta</p>";
        // page += "<p>Historia: " + deflectionHistory + "</p>"; // Voit näyttää historian halutessasi
        page += "<p>Odota kokeen valvojan ohjeita.</p>";
    } else { // Peli käynnissä
        page += "<p>Koehenkilö: <strong>" + currentSubjectName + "</strong></p>";
        page += "<p>Onko keskimmäinen sauva lähempänä vai kauempana kuin reunimmaiset sauvat?</p>";
        // Käytetään formia GET-pyynnön sijaan selkeyden vuoksi, toimii samoin
        page += "<form action='/guess' method='get' style='display: inline-block; width: 100%;'>";
        page += "<input type='hidden' name='answer' value='front'>";
        page += "<button type='submit' class='large-btn'>Lähellä</button>";
        page += "</form><br>";
        page += "<form action='/guess' method='get' style='display: inline-block; width: 100%;'>";
        page += "<input type='hidden' name='answer' value='back'>";
        page += "<button type='submit' class='large-btn secondary-btn'>Kaukana</button>";
        page += "</form>";
        // page += "<p style='font-size: 0.8em; color: #888;'>Poikkeama: " + String(deflection, 2) + "</p>"; // Debug: Näytä poikkeama
    }

    page += HTML_FOOTER;
    return page;
}

// Luo sisältö valvojan sivulle
String getAdminPageContent(String extraMessage /*= ""*/) { // Parametri määritelty täällä
    String page = HTML_HEADER;
    page += "<h1>Stereonäkötesti - Valvoja</h1>";
    page += extraMessage; // Näytä mahdolliset virheet (esim. tyhjä nimi)

    // --- Tilanteen mukaan näytettävä sisältö ---
    if (gameStarted && !gameOver) {
        // Testi käynnissä
        page += "<h2>Testi käynnissä</h2>";
        page += "<p class='status'>Koehenkilö: <strong>" + currentSubjectName + "</strong></p>";
        page += "<p>Nykyinen poikkeama: " + String(deflection, 2) + " astetta</p>";
        page += "<p>Väärät arvaukset: " + String(wrongGuesses) + "/" + String(maxWrongGuesses) + "</p>";
        page += "<p>Historia: " + deflectionHistory + "</p>";
        page += "<p><i>Sivu ei päivity automaattisesti. Päivitä selain nähdäksesi viimeisimmän tilanteen.</i></p>";
        // Voit lisätä "Keskeytä"-napin tänne tarvittaessa
        // page += "<form action='/abort' method='post'><button type='submit' class='secondary-btn'>Keskeytä nykyinen testi</button></form>"; // Vaatisi /abort reitin

    } else if (gameOver) {
        // Testi juuri päättynyt
        page += "<h2>Testi päättynyt</h2>";
        page += "<p class='status'>Koehenkilö: <strong>" + currentSubjectName + "</strong></p>";
        // Hae viimeisin tallennettu tulos (jos löytyy)
        if (!allResults.empty() && allResults.back().name == currentSubjectName) {
             page += "<p>Tulos (pienin/viimeinen poikkeama): " + String(allResults.back().finalDeflection, 2) + " astetta</p>";
             page += "<p>Historia: " + allResults.back().history + "</p>";
        } else {
             page += "<p>Tulos (viimeinen poikkeama): " + String(deflection, 2) + " astetta</p>"; // Näytä nykyinen jos tallennus epäonnistui
             page += "<p class='error'>Huom: Viimeisintä tulosta ei ehkä tallennettu oikein listaan.</p>";
        }

        // Seuraavat toiminnot
        page += "<h3>Seuraava vaihe:</h3>";
         page += "<form action='/setname' method='post'>";
         page += "<input type='hidden' name='subjectName' value=''>"; // Lähetä tyhjä nimi nollataksesi ja palataksesi nimen syöttöön
         page += "<button type='submit' class='secondary-btn'>Syötä uusi koehenkilö (Nollaa)</button>";
         page += "</form>";
         page += "<form action='/download' method='get'><button type='submit' class='download-btn'>Lataa kaikki tulokset (CSV)</button></form>";


    } else if (currentSubjectName != "") {
         // Nimi asetettu, odottaa koehenkilön aloitusta
        page += "<h2>Odotetaan aloitusta</h2>";
        page += "<p class='status'>Testi valmis aloitettavaksi koehenkilölle: <strong>" + currentSubjectName + "</strong></p>";
        page += "<p>Ohjeista koehenkilö yhdistämään laitteen WiFi-verkkoon (" + String(ssid) + ") ja menemään selaimella laitteen IP-osoitteeseen (tai http://" + String(ssid) + ".local, jos mDNS toimii).</p>";
        page += "<p>Koehenkilön näkymä on osoitteessa <a href='/' target='_blank'>/</a>.</p>";
        page += "<p><i>Päivitä tämä sivu nähdäksesi testin etenemisen, kun koehenkilö aloittaa.</i></p>";

         page += "<form action='/setname' method='post'>";
         page += "<input type='hidden' name='subjectName' value=''>"; // Lähetä tyhjä nimi nollataksesi
         page += "<button type='submit' class='secondary-btn'>Vaihda / Nollaa koehenkilö</button>";
         page += "</form>";

    } else {
        // Ei nimeä asetettu, näytä lomake
        page += "<h2>Aseta koehenkilön nimi</h2>";
        page += "<form action='/setname' method='post'>";
        page += "<label for='subjectName'>Koehenkilön nimi:</label><br>";
        page += "<input type='text' id='subjectName' name='subjectName' required><br>";
        page += "<input type='submit' value='Aseta nimi ja valmistaudu'>";
        page += "</form>";
        if (!allResults.empty()) {
             page += "<br><form action='/download' method='get'><button type='submit' class='download-btn'>Lataa tähänastiset tulokset (CSV)</button></form>";
        }
    }


    // --- Näytä kaikkien testien tulokset ---
    if (!allResults.empty()) {
        page += "<h2>Tallennetut tulokset</h2>";
        page += "<table><thead><tr><th>Nimi</th><th>Tulos (asteina)</th><th>Historia</th></tr></thead><tbody>";
        // Käydään tulokset läpi käänteisessä järjestyksessä (uusin ensin)
        for (int i = allResults.size() - 1; i >= 0; i--) {
            page += "<tr>";
            page += "<td>" + allResults[i].name + "</td>";
            page += "<td>" + String(allResults[i].finalDeflection, 2) + "</td>";
            page += "<td>" + allResults[i].history + "</td>";
            page += "</tr>";
        }
        page += "</tbody></table>";
    } else {
         page += "<p>Ei vielä tallennettuja tuloksia.</p>";
    }


    page += HTML_FOOTER;
    return page;
}

// --- CSV Generation ---
String generateCSV() {
    std::stringstream ss;
    // Header row
    ss << "KoehenkiloNimi,LopputulosAsteina,PoikkeamaHistoria\n";

    // Data rows
    for (const auto& result : allResults) { // <- Brace 1 opens
        // Korvataan pilkut ja rivinvaihdot nimestä ja historiasta (varmuuden vuoksi)
        String name = result.name;
        name.replace(",", ";");
        name.replace("\n", " ");
        String history = result.history;
        history.replace(",", ";");
        history.replace("\n", " | ");

        ss << name.c_str() << ","; // Käytä c_str() tai Stringin metodeja
        ss << String(result.finalDeflection, 2).c_str() << ","; // Muunna double stringiksi ja sitten c_str()
        ss << "\"" << history.c_str() << "\"\n"; // Laita historia lainausmerkkeihin, jos se sisältää erikoismerkkejä
    } // <- *** LISÄTTY PUUTTUVA SULKU for-loopille ***

    // Käytä std::string väliaikaisesti, koska ESP String ei välttämättä toimi suoraan stringstreamin kanssa helposti
    std::string std_string = ss.str();
    return String(std_string.c_str());
} // <- *** LISÄTTY PUUTTUVA SULKU funktiolle ***
