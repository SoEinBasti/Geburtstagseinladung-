// Version 2.1
// Namen/Alter zentral (HOST/GUEST/AGE) + Herz-Sprite + Alters-Seite + WhatsApp-Kontakt + persoenliche Laufschrift
// Intro-Feuerwerk + Highscore-Feuerwerk endet mit Easter-Egg-Hinweis (Laufschrift)
// Easter Egg: "Sternschnuppe fangen" (Timing-Spiel, Wunsch frei) statt Bombe
// Highscore wird bei jedem Flash zurueckgesetzt (Build-Kennung in EEPROM 2-3)
// Hinweis: HD44780-Display kann keine Umlaute -> bewusst ae/oe/ue/ss verwendet
//
// Langer Druck (20 Sekunden) im Highscore-Screen = Loescht den Highscore
// Langer Druck (2 Sekunden) im Dino-Run-Screen = Geheim-Modus (Sternschnuppe fangen)
//
// Hardware Setup:
// Arduino Nano V3 Clone
// LCD 1602A HD44780 Display (16x2 characters)
// 1 Push Button (12x12mm)
//
// Pin Mapping:
// Button: Pin 8 (Other pin to GND, using INPUT_PULLUP)
// Buzzer: Pin 9 (Positive to Pin 9, Negative to GND)
// LCD RS: Pin 12
// LCD E:  Pin 11
// LCD D4: Pin 5
// LCD D5: Pin 4
// LCD D6: Pin 3
// LCD D7: Pin 2
// LCD RW: GND (Write only)
// LCD VSS: GND
// LCD VDD: 5V
// LCD V0: Center pin of a potentiometer (outer pins to 5V & GND)
// LCD A:  5V direkt (Vorwiderstand fuer die Hintergrundbeleuchtung ist onboard)
// LCD K:  GND

#include <LiquidCrystal.h>
#include <EEPROM.h>

// =====================================================================
// --- ANPASSBARE TEXTE (DEUTSCH) ---
// Hinweis: Das LCD hat 16 Zeichen pro Zeile. Strings nicht laenger machen.
// Leerzeichen sind Absicht: sie ueberschreiben alte Zeichen auf dem Screen.
// KEINE Umlaute verwenden (Display kann sie nicht) -> ae/oe/ue/ss.
// =====================================================================

// >>> PRO BAUSATZ NUR DIESE DREI ZEILEN ANPASSEN <<<
#define HOST   "Thea"      // Geburtstagskind (laedt ein)
#define GUEST  "Emilia"    // eingeladener Gast
#define AGE    "9"         // Alter, das gefeiert wird

// Trefferfenster fuer die Sternschnuppe (ms Abweichung, die noch als Erfolg zaehlt)
#define CATCH_TOLERANCE 650

// Hauptspiel & Menues
const char* txtTitle       = "   DINO RUN   ";
// Easter-Egg-Hinweis - erscheint als Belohnung im Highscore-Feuerwerk (Laufschrift)
const char* txtEggHint     = "Versuch doch mal beim Spielstart den Knopf 2 Sek zu halten und schau was passiert...";
const char* txtLevelUp     = "!!! LEVEL ";
const char* txtLevelUpEnd  = " !!!";
const char* txtBestScore   = "BESTE: ";
const char* txtPoints      = " Pkt.   ";
const char* txtLevelStr    = "LEVEL: ";
const char* txtScoreDel    = "SCORE GELOESCHT!";
const char* txtNewRecord   = "NEUER REKORD!   ";
const char* txtGameOver    = "GAME OVER!      ";
const char* txtScoreShort  = "P:";
const char* txtLevelShort  = " L:";

// Easter Egg (Sternschnuppe fangen)
const char* eeInfo1L1      = "Oh, eine        ";
const char* eeInfo1L2      = "Sternschnuppe!  ";
const char* eeInfo2L1      = "Schnell, sie     ";
const char* eeInfo2L2      = "fliegt gleich ";  // Danach folgt das Stern-Icon
const char* eeInfo3L1      = "Fang sie im     ";
const char* eeInfo3L2      = "richtigen Moment";
const char* eeInfo4L1      = "und wuensch Dir ";
const char* eeInfo4L2      = "was Schoenes!   ";
const char* eePrepL1       = "Sie kommt in:   ";
const char* eePrepSec      = " Sekunden";
const char* eeRememberL1   = "Zaehl mit und   ";
const char* eeRememberL2   = "merk die Zeit!  ";
const char* eeCountdown3   = "3...            ";
const char* eeCountdown2   = "2...            ";
const char* eeCountdown1   = "1...            ";
const char* eeBeep         = "Los geht's!     ";
const char* eeBoomL1       = "Schade, ";        // Danach Stern-Icon und " weg!"
const char* eeBoomL1End    = " weg!";
const char* eeTooSlow      = "Zu langsam :(   ";
const char* eeDiff         = "Um ";            // ergibt z.B. "Um 200 ms"
const char* eeMs           = " ms";
const char* eeTooEarly     = "zu frueh dran!  ";
const char* eeTooLate      = "zu spaet dran!  ";
const char* eePerfect      = "genau richtig!  ";
const char* eeDefusedL1    = "Gefangen!       ";
const char* eeSavedL2      = "Wuensch Dir was!";
const char* eeInaccurate   = "Fast erwischt!  ";

// Einladung (Seiten 1-11). Namen/Alter kommen aus HOST/GUEST/AGE oben.
// TODO: Datum, Uhrzeit, Ort und Zusage-Frist noch mit echten Werten fuellen!
const char* invPage1L1     = "Einladung zur   ";
const char* invPage1L2     = "Geburtstagsparty";
const char* invPage3Date   = "25.08. um 08:00 "; // TODO: echtes Datum + Uhrzeit
const char* invPage3Scroll = "      Wir holen Dich ab!   "; // Lauftext
const char* invPage4L1     = "Wir gehen ins   ";
const char* invPage4L2     = "SCHWIMMBAD      "; // TODO: Ort anpassen falls anders
const char* invPage5L1     = "Badesachen      ";
const char* invPage5L2Show = "NICHT vergessen!"; // Blink-Effekt (sichtbar)
const char* invPage5L2Hide = "      vergessen!"; // Blink-Effekt (versteckt)
const char* invPage6L1     = "Endet gegen     ";
const char* invPage6L2     = "14:00 Uhr       "; // TODO: echte End-Uhrzeit
const char* invPage7L1     = "Bitte Zusage bis";
const char* invPage7L2     = "10.08.          "; // TODO: echte Zusage-Frist
const char* invPage8L1     = "WhatsApp Zusage:"; // Kontakt-Seite (genau 16 Zeichen)
const char* invWhatsApp    = "0170-2931131";     // TODO: echte Nummer pruefen
// Persoenliche Abschluss-Botschaft (frei anpassbar) - laeuft als Laufschrift
const char* invPersonalScroll = "      Ich freu mich riesig auf Dich!   ";


// =====================================================================
// --- PINS & HARDWARE SETUP ---
// =====================================================================
const int buttonPin = 8;
const int buzzerPin = 9;
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// --- SPRITES (Custom Characters) ---
byte dinoSprite[8]    = { B00111, B00101, B00111, B10110, B11111, B01010, B01010, B00000 };
byte cactus1Sprite[8] = { B00000, B00100, B10100, B10101, B11101, B00111, B00100, B00100 };
byte cactus2Sprite[8] = { B00100, B01110, B00100, B00100, B01110, B00100, B00100, B00100 };
byte rocketSprite[8]  = { B00000, B00100, B01110, B01010, B01010, B11111, B10101, B00000 };
byte starSprite[8]    = { B00100, B00100, B10101, B01110, B01110, B10101, B00100, B00100 }; // Sternschnuppe (Easter Egg)
// Feuerwerk-Sprites (Slots 5-7)
byte fwTrail[8]       = { B00000, B00100, B00100, B00100, B00100, B00100, B00000, B00000 }; // Aufsteigende Rakete
byte fwBurst[8]       = { B00100, B10101, B01110, B11011, B01110, B10101, B00100, B00000 }; // Explosions-Stern
byte fwSpark[8]       = { B00000, B00100, B00000, B01010, B00000, B00100, B00000, B00000 }; // Verstreute Funken
// Herz fuer die Einladung. Teilt sich Slot 7 mit fwSpark:
//   - nach dem Intro-Feuerwerk wird Slot 7 auf das Herz umgestellt (setup)
//   - showFireworks() laedt vor jedem Feuerwerk kurz die Funken zurueck
byte heartSprite[8]   = { B00000, B01010, B11111, B11111, B11111, B01110, B00100, B00000 };

// --- GLOBAL VARIABLES ---
int gameMode = 0;          // 0: Invite, 1: Start, 2: Play, 3: LevelUp, 4: GameOver, 5: Highscore, 6-9: EasterEgg
int invitationPage = 1;
int playerY = 1;           // Player Y position (0 = top, 1 = bottom)
bool canJump = true;       // Prevents holding the button to float endlessly

int score = 0;
int highscore = 0;
int bestLevel = 1;
int currentLevel = 1;
int gameSpeed = 350;       // Delay between frames in milliseconds

unsigned long jumpStartTime = 0;
unsigned long lastMoveTime = 0;

// --- LEVEL PRE-CALCULATION ARRAYS ---
// Max 25 enemies per level to prevent RAM overflow on the Arduino
const int MAX_OBSTACLES = 25;
int obsX[MAX_OBSTACLES];
byte obsY[MAX_OBSTACLES];
byte obsType[MAX_OBSTACLES];
int numObstacles = 0;
int passedObstacles = 0;

// --- EASTER EGG VARIABLES ---
int eePage = 0;
unsigned long eePressStart = 0;
bool eeActive = false;
int targetTime = 0;
unsigned long bombStartTime = 0;
unsigned long pressTime = 0;
bool bombStarted = false;


// =====================================================================
// --- SOUND EFFECTS ---
// =====================================================================
void playMarioIntro() {
  int melody[] = {660, 660, 0, 660, 0, 510, 660, 0, 770};
  int duration[] = {100, 100, 100, 100, 100, 100, 100, 100, 150};
  for (int i = 0; i < 9; i++) {
    if (melody[i] == 0) delay(duration[i]);
    else { tone(buzzerPin, melody[i], duration[i]); delay(duration[i] + 20); }
  }
}

void playPokemonHeal() {
  int notes[] = {1568, 1397, 1319, 1047, 1175, 1319};
  for (int i = 0; i < 6; i++) {
    tone(buzzerPin, notes[i], 150); delay(200);
  }
}

void playPokemonTriumph() {
  int notes[] = {1047, 1047, 1047, 1047, 1175, 1319, 1397, 1568};
  for (int i = 0; i < 8; i++) {
    tone(buzzerPin, notes[i], 150); delay(180);
  }
}

void playBeep() { tone(buzzerPin, 1500, 200); }

void playVictoryJingle() {
  int notes[] = {523, 659, 784, 1047};
  for (int i = 0; i < 4; i++) { tone(buzzerPin, notes[i], 150); delay(180); }
  delay(100); tone(buzzerPin, 1047, 400);
}

// Sanftes "knapp daneben" (Sternschnuppe verpasst) - zwei absteigende Toene
void playMiss() {
  tone(buzzerPin, 392, 150); delay(180); // G
  tone(buzzerPin, 262, 300); delay(320); // C tiefer
}


// =====================================================================
// --- SETUP FUNCTION ---
// =====================================================================
void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);

  // Initialize LCD and load custom characters
  lcd.begin(16, 2);
  lcd.createChar(0, dinoSprite);
  lcd.createChar(1, cactus1Sprite);
  lcd.createChar(2, cactus2Sprite);
  lcd.createChar(3, rocketSprite);
  lcd.createChar(4, starSprite);
  lcd.createChar(5, fwTrail);
  lcd.createChar(6, fwBurst);
  lcd.createChar(7, fwSpark);

  // Seed random generator with analog noise
  randomSeed(analogRead(0));

  // Build-Kennung aus Compile-Zeit: aendert sich bei jedem Kompilieren.
  // Passt die in EEPROM (Adresse 2-3) gespeicherte Kennung nicht -> frisch
  // geflasht -> Highscore zuruecksetzen. So startet jeder neue Bausatz sauber.
  const char* bid = __DATE__ " " __TIME__;
  uint16_t buildStamp = 0;
  for (const char* p = bid; *p; p++) buildStamp = buildStamp * 31 + (uint8_t)(*p);
  uint16_t storedStamp = ((uint16_t)EEPROM.read(2) << 8) | EEPROM.read(3);

  if (storedStamp != buildStamp) {
    // Neue Firmware -> Highscore + Kennung neu schreiben
    highscore = 0;
    bestLevel = 1;
    EEPROM.write(0, 0);
    EEPROM.write(1, 1);
    EEPROM.write(2, buildStamp >> 8);
    EEPROM.write(3, buildStamp & 0xFF);
  } else {
    // Gleiche Firmware -> gespeicherten Highscore laden
    highscore = EEPROM.read(0);
    if (highscore == 255) highscore = 0; // 255 = leeres EEPROM
    bestLevel = EEPROM.read(1);
    if (bestLevel == 255) bestLevel = 1;
  }

  playMarioIntro();
  showIntro();   // Feuerwerk + "<HOST> feiert Geburtstag!" vor der Einladung

  // Intro-Feuerwerk vorbei -> Slot 7 von Funken auf Herz umstellen (fuer die Einladung)
  lcd.createChar(7, heartSprite);
}

// =====================================================================
// --- MAIN LOOP ---
// =====================================================================
void loop() {
  // State machine controlling the current screen/logic
  if (gameMode == 0) showInvitation();
  else if (gameMode == 1) showGameStart();
  else if (gameMode == 5) showHighscorePage();
  else if (gameMode == 2) gameLogic();
  else if (gameMode == 3) showLevelUp();
  else if (gameMode == 4) showGameOver();
  else if (gameMode == 6) eeIntro();
  else if (gameMode == 7) eePreparation();
  else if (gameMode == 8) eeGameLogic();
  else if (gameMode == 9) eeResult();
}

// =====================================================================
// --- GAME MECHANICS ---
// =====================================================================

// Calculates all enemies for the current level in advance
void generateLevel() {
  passedObstacles = 0;
  // Slowly increase enemy count, maxed out at MAX_OBSTACLES
  numObstacles = min(3 + (currentLevel * 2), MAX_OBSTACLES);

  int currentX = 16; // Start placing enemies slightly off-screen to the right

  for(int i = 0; i < numObstacles; i++) {
    // 1. DETERMINE TYPE AND HEIGHT
    if (currentLevel >= 4 && random(0, 10) > 7) {
      obsY[i] = 0; // Spawns at the top
      obsType[i] = 3; // Is a rocket
    } else {
      obsY[i] = 1; // Spawns at the bottom
      obsType[i] = (random(0, 3) < 2) ? 1 : 2; // Randomly cactus 1 or 2
    }

    // 2. DETERMINE X POSITION
    if (i == 0) {
      obsX[i] = currentX;
    } else {
      int spacing = 0;
      bool isRocket = (obsY[i] == 0);
      bool prevWasRocket = (obsY[i-1] == 0);

      if (isRocket || prevWasRocket) {
        // Enforce safe distance if a rocket is involved
        spacing = random(6, 10);
      } else {
        // Cactus logic: both obstacles are on the ground
        if (currentLevel >= 3 && random(0, 10) > 6) {
          // 30% chance for a double-cactus (placed right next to each other)
          if (i == 1 || (obsX[i-1] - obsX[i-2] > 2)) {
            spacing = 1; // Create double-enemy
          } else {
            // Prevent impossible 3-blocks
            if (currentLevel >= 5) spacing = random(3, 6); // Gap 3 to 5
            else spacing = random(5, 9);                   // Gap 5 to 8
          }
        } else {
          // Normal gap without double-enemies
          if (currentLevel >= 5) spacing = random(3, 6);
          else spacing = random(5, 9);
        }
      }
      currentX += spacing;
      obsX[i] = currentX;
    }
  }
}

void gameLogic() {
  static int oldScore = -1;
  int oldPlayerY = playerY;

  // --- 1. DEBOUNCE BUTTON ---
  bool rawButton = (digitalRead(buttonPin) == LOW);
  static bool buttonPressed = false;
  static unsigned long lastButtonPress = 0;

  if (rawButton) {
    buttonPressed = true;
    lastButtonPress = millis();
  } else {
    // Release button only after 50ms to debounce
    if (millis() - lastButtonPress > 50) buttonPressed = false;
  }

  // --- 2. DYNAMIC JUMP LOGIC ---
  unsigned long jumpDuration = millis() - jumpStartTime;
  unsigned long minJumpTime = 2 * gameSpeed;
  unsigned long maxJumpTime = 4 * gameSpeed;

  if (playerY == 1) { // Player is on the ground
    if (buttonPressed && canJump) {
      playerY = 0; // Move up
      jumpStartTime = millis();
      canJump = false; // Prevent holding jump
      tone(buzzerPin, 800, 30);
    }
    if (!buttonPressed) canJump = true; // Reset jump ability
  } else { // Player is in the air
    if (buttonPressed) {
      if (jumpDuration >= maxJumpTime) playerY = 1; // Fall down after max time
    } else {
      if (jumpDuration >= minJumpTime) playerY = 1; // Fall down early if button released
    }
  }

  // --- 3. MOVE GAME WORLD ---
  if (millis() - lastMoveTime > gameSpeed) {
    lastMoveTime = millis();

    // Clear old positions of visible obstacles
    for (int i = 0; i < numObstacles; i++) {
      if (obsX[i] >= 0 && obsX[i] < 16) {
        // Prevent clearing the score text in the top right corner
        if (!(obsY[i] == 0 && obsX[i] >= 13)) {
          lcd.setCursor(obsX[i], obsY[i]);
          lcd.print(" ");
        }
      }
    }

    // Move all obstacles to the left by 1
    for (int i = 0; i < numObstacles; i++) {
      obsX[i]--;

      // Score point when an obstacle passes the player (X = -1)
      if (obsX[i] == -1) {
        score++;
        passedObstacles++;
        tone(buzzerPin, 1200, 15);
      }

      // Draw new position (only if within screen bounds)
      if (obsX[i] >= 0 && obsX[i] < 16) {
        // Don't draw rockets over the score (X>=13)
        if (!(obsY[i] == 0 && obsX[i] >= 13)) {
          lcd.setCursor(obsX[i], obsY[i]);
          lcd.write(byte(obsType[i]));
        }
      }
    }

    // Check if the level is completed
    if (passedObstacles >= numObstacles) {
      currentLevel++;
      gameMode = 3;
      return;
    }
  }

  // --- 4. COLLISION DETECTION ---
  for (int i = 0; i < numObstacles; i++) {
    // Collision happens if player and obstacle share the same coordinates
    if (obsX[i] == 0 && playerY == obsY[i]) {
      gameMode = 4; // Trigger Game Over
      return;
    }
  }

  // Update player sprite on screen
  if (playerY != oldPlayerY) {
    lcd.setCursor(0, oldPlayerY); lcd.print(" ");
  }
  lcd.setCursor(0, playerY);
  lcd.write(byte(0));

  // Update score display only if changed
  if (score != oldScore) {
    lcd.setCursor(13, 0);
    if (score < 10) lcd.print(" ");
    if (score < 100) lcd.print(" ");
    lcd.setCursor(13, 0);
    lcd.print(score);
    oldScore = score;
  }
}

// =====================================================================
// --- UI SCREENS ---
// =====================================================================

void showLevelUp() {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(txtLevelUp); lcd.print(currentLevel); lcd.print(txtLevelUpEnd);

  // Play sound depending on milestone
  if (currentLevel == 5 || currentLevel == 10) playPokemonTriumph();
  else playPokemonHeal();

  // Increase speed
  if (currentLevel == 2) gameSpeed = 320;
  else if (currentLevel == 3) gameSpeed = 280;
  else { if(gameSpeed > 80) gameSpeed -= 10; }

  // Pre-calculate the next level while the screen is shown
  generateLevel();

  delay(1000); lcd.clear(); gameMode = 2;
}

void showHighscorePage() {
  static unsigned long hsPressStart = 0; // Tracks button hold duration

  lcd.setCursor(0, 0); lcd.print(txtBestScore); lcd.print(highscore); lcd.print(txtPoints);
  lcd.setCursor(0, 1); lcd.print(txtLevelStr); lcd.print(bestLevel); lcd.print("      ");

  if (digitalRead(buttonPin) == LOW) {
    if (hsPressStart == 0) {
      hsPressStart = millis(); // Button just pressed
    } else if (millis() - hsPressStart >= 20000) {
      // Button held for 20 seconds -> CLEAR EEPROM
      highscore = 0;
      bestLevel = 1;
      EEPROM.write(0, 0);
      EEPROM.write(1, 1);

      tone(buzzerPin, 500, 500);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(txtScoreDel);
      delay(1500);
      lcd.clear();
      hsPressStart = 0; // Reset timer
    }
  } else {
    // Button released
    if (hsPressStart > 0 && millis() - hsPressStart < 2000) {
      // Short press -> start game normally
      lcd.clear();
      score = 0;
      currentLevel = 1;
      gameSpeed = 350;

      generateLevel();

      gameMode = 2;
      delay(500);
    }
    hsPressStart = 0; // Safely reset timer
  }
}

void showGameOver() {
  lcd.clear();
  bool newRecord = false;
  if (score > highscore) {
    lcd.print(txtNewRecord);
    highscore = score;
    bestLevel = currentLevel;
    EEPROM.write(0, highscore);
    EEPROM.write(1, bestLevel);
    newRecord = true;
  } else {
    lcd.print(txtGameOver);
  }

  lcd.setCursor(0, 1);
  lcd.print(txtScoreShort); lcd.print(score);
  lcd.print(txtLevelShort); lcd.print(currentLevel);

  tone(buzzerPin, 150, 600);
  delay(1500);

  // Bei neuem Rekord: Feuerwerk als Belohnung
  if (newRecord) showFireworks();

  invitationPage = 0;
  gameMode = 1;
}

void showGameStart() {
  lcd.setCursor(0, 0); lcd.print(txtTitle); lcd.write(byte(0));

  // Untere Zeile wechselt alle 2s zwischen Absender und Empfaenger
  if ((millis() / 2000) % 2 == 0) {
    lcd.setCursor(0, 1); printPadded("Von " HOST);
  } else {
    lcd.setCursor(0, 1); printPadded("Fuer " GUEST);
  }

  // Activate Easter Egg if button held for 2 seconds
  if (digitalRead(buttonPin) == LOW) {
    if (eePressStart == 0) {
      eePressStart = millis();
    } else if (millis() - eePressStart >= 2000 && !eeActive) {
      eeActive = true;
      tone(buzzerPin, 2000, 300);
      lcd.clear();
      gameMode = 6;
      eePressStart = 0;
      delay(500);
      return;
    }
  } else {
    if (eePressStart > 0 && millis() - eePressStart < 2000) {
      tone(buzzerPin, 1000, 100);
      lcd.clear();
      gameMode = 5;
      delay(500);
    }
    eePressStart = 0;
  }
}

// =====================================================================
// --- EASTER EGG (STERNSCHNUPPE FANGEN) ---
// =====================================================================
void eeIntro() {
  if (eePage == 0) {
    lcd.setCursor(0, 0); lcd.print(eeInfo1L1);
    lcd.setCursor(0, 1); lcd.print(eeInfo1L2);
  } else if (eePage == 1) {
    lcd.setCursor(0, 0); lcd.print(eeInfo2L1);
    lcd.setCursor(0, 1); lcd.print(eeInfo2L2); lcd.write(byte(4)); // Stern-Icon
  } else if (eePage == 2) {
    lcd.setCursor(0, 0); lcd.print(eeInfo3L1);
    lcd.setCursor(0, 1); lcd.print(eeInfo3L2);
  } else if (eePage == 3) {
    lcd.setCursor(0, 0); lcd.print(eeInfo4L1);
    lcd.setCursor(0, 1); lcd.print(eeInfo4L2);
  }

  if (digitalRead(buttonPin) == LOW) {
    tone(buzzerPin, 800, 50);
    eePage++;
    delay(300);
    if (eePage > 3) { eePage = 0; gameMode = 7; }
    lcd.clear();
  }
}

void eePreparation() {
  int seconds = random(3, 7);
  targetTime = seconds * 1000;

  lcd.clear(); lcd.setCursor(0, 0); lcd.print(eePrepL1); lcd.setCursor(0, 1); lcd.print(seconds); lcd.print(eePrepSec); delay(2500);
  lcd.clear(); lcd.setCursor(0, 0); lcd.print(eeRememberL1); lcd.setCursor(0, 1); lcd.print(eeRememberL2); delay(1500);

  lcd.clear(); lcd.setCursor(0, 0); lcd.print(eeCountdown3); tone(buzzerPin, 800, 200); delay(1000);
  lcd.clear(); lcd.setCursor(0, 0); lcd.print(eeCountdown2); tone(buzzerPin, 800, 200); delay(1000);
  lcd.clear(); lcd.setCursor(0, 0); lcd.print(eeCountdown1); tone(buzzerPin, 800, 200); delay(1000);
  lcd.clear(); lcd.setCursor(0, 0); lcd.print(eeBeep); playBeep(); delay(500);

  lcd.clear();
  bombStartTime = millis();
  bombStarted = true;
  gameMode = 8;
}

void eeGameLogic() {
  unsigned long elapsedTime = millis() - bombStartTime;

  // Timeout: Explode if 2 seconds passed the target time
  if (elapsedTime > targetTime + 2000) {
    pressTime = 999999;
    gameMode = 9;
    return;
  }

  // Register button press
  if (digitalRead(buttonPin) == LOW) {
    pressTime = elapsedTime;
    gameMode = 9;
    delay(300);
  }
}

void eeResult() {
  lcd.clear();
  if (pressTime == 999999) {
    lcd.setCursor(0, 0); lcd.print(eeBoomL1); lcd.write(byte(4)); lcd.print(eeBoomL1End);
    lcd.setCursor(0, 1); lcd.print(eeTooSlow);
    playMiss();
  } else {
    long diff = (long)pressTime - (long)targetTime;
    unsigned long absDiff = abs(diff);

    lcd.setCursor(0, 0); lcd.print(eeDiff); lcd.print(absDiff); lcd.print(eeMs);
    lcd.setCursor(0, 1);

    if (absDiff <= CATCH_TOLERANCE) lcd.print(eePerfect); // im Trefferfenster -> positiv
    else if (diff < 0) lcd.print(eeTooEarly);
    else lcd.print(eeTooLate);

    delay(2500); lcd.clear();

    if (absDiff <= CATCH_TOLERANCE) {
      lcd.setCursor(0, 0); lcd.print(eeDefusedL1);
      lcd.setCursor(14, 0); lcd.write(byte(4)); // kleiner Stern als Belohnung
      lcd.setCursor(0, 1); lcd.print(eeSavedL2);
      playVictoryJingle();
    } else {
      lcd.setCursor(0, 0); lcd.print(eeBoomL1); lcd.write(byte(4)); lcd.print(eeBoomL1End);
      lcd.setCursor(0, 1); lcd.print(eeInaccurate);
      playMiss();
    }
  }
  delay(3000); lcd.clear(); bombStarted = false; eeActive = false; gameMode = 1;
}

// =====================================================================
// --- FIREWORKS ANIMATION (Highscore-Belohnung, zeigt den Gast) ---
// =====================================================================

// Eine einzelne Rakete: steigt in der Spalte 'col' auf und zerplatzt oben.
void launchFirework(int col) {
  // Aufstieg: Rakete wandert von unten (Zeile 1) nach oben (Zeile 0)
  for (int row = 1; row >= 0; row--) {
    lcd.setCursor(col, row);
    lcd.write(byte(5));                       // Raketen-Trail
    tone(buzzerPin, 400 + (1 - row) * 600, 60); // Pfeifton, steigt beim Hochfliegen
    delay(120);
    lcd.setCursor(col, row);
    lcd.print(" ");                           // alte Position wieder loeschen
  }

  // Explosion oben
  lcd.setCursor(col, 0); lcd.write(byte(6));  // grosser Stern
  if (col - 1 >= 0) { lcd.setCursor(col - 1, 0); lcd.write(byte(7)); } // Funken links
  if (col + 1 < 16) { lcd.setCursor(col + 1, 0); lcd.write(byte(7)); } // Funken rechts
  lcd.setCursor(col, 1); lcd.write(byte(7));  // Funken darunter
  tone(buzzerPin, 200, 150);                  // "Plopp"
  delay(220);

  // Explosion weitet sich aus
  if (col - 2 >= 0) { lcd.setCursor(col - 2, 0); lcd.print("."); }
  if (col + 2 < 16) { lcd.setCursor(col + 2, 0); lcd.print("."); }
  if (col - 1 >= 0) { lcd.setCursor(col - 1, 1); lcd.write(byte(7)); }
  if (col + 1 < 16) { lcd.setCursor(col + 1, 1); lcd.write(byte(7)); }
  delay(220);

  // Ausblenden
  lcd.setCursor(0, 0); lcd.print("                ");
  lcd.setCursor(0, 1); lcd.print("                ");
}

// Komplette Feuerwerks-Show mit Abschlussbotschaft.
void showFireworks() {
  lcd.createChar(7, fwSpark);  // Slot 7 kurz von Herz auf Funken zuruecksetzen
  lcd.clear();
  int cols[] = {4, 11, 8};                    // Raketen nacheinander an 3 Spalten
  for (int i = 0; i < 3; i++) launchFirework(cols[i]);

  // Zwischen-Botschaft mit Fanfare (Highscore-Kontext, kein Geburtstagsgruss)
  lcd.clear();
  lcd.setCursor(0, 0); printCentered("HIGHSCORE!");
  lcd.setCursor(0, 1); lcd.print(" *  .  *  +  *  ");
  playVictoryJingle();
  delay(400);

  // Grosses Finale: zwei Raketen + Triumph-Melodie
  launchFirework(3);
  launchFirework(12);
  lcd.clear();

  // Belohnung: Easter-Egg-Hinweis als Laufschrift (statt "Fuer Dich [GAST]")
  lcd.setCursor(0, 0); printCentered("Geheim-Tipp!");
  playPokemonTriumph();
  scrollOnce(txtEggHint, 1, 200);
  lcd.clear();
}

// Start-Sequenz: kurzes Feuerwerk zur Begruessung, dann die Ankuendigung.
// Wird einmalig in setup() aufgerufen, bevor die Einladung startet.
void showIntro() {
  lcd.clear();
  int cols[] = {4, 11, 8};                    // drei Begruessungs-Raketen
  for (int i = 0; i < 3; i++) launchFirework(cols[i]);

  lcd.clear();
  lcd.setCursor(0, 0); printCentered(HOST " feiert");
  lcd.setCursor(0, 1); printCentered("Geburtstag!");
  playVictoryJingle();
  delay(2500);
  lcd.clear();
}

// =====================================================================
// --- DISPLAY-HELFER ---
// =====================================================================

// Schreibt s und fuellt den Rest der 16-Zeichen-Zeile mit Leerzeichen auf.
// So werden alte Zeichen sicher ueberschrieben - unabhaengig von der Namenslaenge.
void printPadded(const char* s) {
  lcd.print(s);
  for (int i = strlen(s); i < 16; i++) lcd.print(" ");
}

// Zentriert s in der 16-Zeichen-Zeile (Annahme: s ist hoechstens 16 Zeichen).
void printCentered(const char* s) {
  int len = strlen(s);
  int left = (16 - len) / 2;
  if (left < 0) left = 0;
  for (int i = 0; i < left; i++) lcd.print(" ");
  lcd.print(s);
  for (int i = left + len; i < 16; i++) lcd.print(" ");
}

// Herzchen-Reihe in der unteren Zeile (nutzt Slot 7 = Herz waehrend der Einladung).
void printHearts() {
  for (int c = 2; c < 16; c += 3) { lcd.setCursor(c, 1); lcd.write(byte(7)); }
}

// Endlos-Laufschrift ab der aktuellen Cursor-Position (16 sichtbare Zeichen).
// 450 ms pro Schritt = zuegig lesbar.
void printScroll(const char* text) {
  String t = text;
  int pos = (millis() / 450) % t.length();
  String out = t.substring(pos) + t.substring(0, pos);
  lcd.print(out.substring(0, 16));
}

// Scrollt text EINMAL durch die angegebene Zeile (blockierend).
// Laeuft von rechts rein und links wieder raus; Knopfdruck bricht frueh ab.
void scrollOnce(const char* text, int row, int stepMs) {
  String t = "                ";  // 16 Zeichen Vorlauf
  t += text;
  t += "                ";        // Nachlauf zum sauberen Rauslaufen
  int total = t.length();
  for (int pos = 0; pos + 16 <= total; pos++) {
    lcd.setCursor(0, row);
    lcd.print(t.substring(pos, pos + 16));
    for (int w = 0; w < stepMs; w += 10) {
      if (digitalRead(buttonPin) == LOW) return; // frueh abbrechen
      delay(10);
    }
  }
}

// =====================================================================
// --- INVITATION LOGIC ---
// =====================================================================
void showInvitation() {
  if (invitationPage == 1) {
    lcd.setCursor(0, 0); lcd.print(invPage1L1);
    lcd.setCursor(0, 1); lcd.print(invPage1L2);
  }
  else if (invitationPage == 2) {
    // Absender - eigene Seite, damit sie beim Blaettern nicht uebersprungen wird
    lcd.setCursor(0, 0); lcd.print("Von: " HOST);
    printHearts();
  }
  else if (invitationPage == 3) {
    // Empfaenger - eigene Seite direkt nach dem Absender
    lcd.setCursor(0, 0); lcd.print("Fuer: " GUEST);
    printHearts();
  }
  else if (invitationPage == 4) {
    // Alters-Seite
    lcd.setCursor(0, 0); printCentered(HOST " wird");
    lcd.setCursor(0, 1); printCentered(AGE " Jahre! :)");
  }
  else if (invitationPage == 5) {
    lcd.setCursor(0, 0); lcd.print(invPage3Date);
    lcd.setCursor(0, 1); printScroll(invPage3Scroll);
  }
  else if (invitationPage == 6) {
    lcd.setCursor(0, 0); lcd.print(invPage4L1);
    lcd.setCursor(0, 1); lcd.print(invPage4L2);
  }
  else if (invitationPage == 7) {
    lcd.setCursor(0, 0); lcd.print(invPage5L1);
    // Blinking logic
    if ((millis() / 500) % 2 == 0) {
      lcd.setCursor(0, 1); lcd.print(invPage5L2Show);
    } else {
      lcd.setCursor(0, 1); lcd.print(invPage5L2Hide);
    }
  }
  else if (invitationPage == 8) {
    lcd.setCursor(0, 0); lcd.print(invPage6L1);
    lcd.setCursor(0, 1); lcd.print(invPage6L2);
  }
  else if (invitationPage == 9) {
    lcd.setCursor(0, 0); lcd.print(invPage7L1);
    lcd.setCursor(0, 1); lcd.print(invPage7L2);
  }
  else if (invitationPage == 10) {
    // Kontakt-Seite: Nummer statisch, damit sie gut ablesbar bleibt
    lcd.setCursor(0, 0); lcd.print(invPage8L1);
    lcd.setCursor(0, 1); printCentered(invWhatsApp);
  }
  else if (invitationPage == 11) {
    // Persoenlicher Abschluss: Laufschrift oben, Herzchen darunter
    lcd.setCursor(0, 0); printScroll(invPersonalScroll);
    printHearts();
  }

  // Navigation
  if (digitalRead(buttonPin) == LOW) {
    tone(buzzerPin, 1000, 50);
    invitationPage++;
    lcd.clear();
    if (invitationPage > 11) { gameMode = 1; }  // Einladung zu Ende -> direkt ins Spiel, kein Feuerwerk hier
    delay(300);
  }
}
