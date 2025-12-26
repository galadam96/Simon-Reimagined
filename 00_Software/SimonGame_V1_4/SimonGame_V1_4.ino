//DATE: 2025.10.08
//Author: Gal Adam
// ========================================
// SIMON GAME - Arduino Nano
// ========================================
// A memory game where players repeat an increasingly long sequence of colors.
// - 4 colored buttons with LEDs
// - Buzzer for audio feedback
// - Green button (index 2) is used to start the game

// ----- PIN CONFIGURATION -----
const int BUTTON_PINS[4] = {10, 9, 8,7};  // Button inputs (with internal pullup)
const int LED_PINS[4] = {3, 4, 5, 6};      // LED outputs for each color
const int BUZZER_PIN = 11;                 // Buzzer output

// ----- AUDIO TONES -----
const int TONES[4] = {310, 209, 415, 252}; // Unique tone for each color

// ----- GAME VARIABLES -----
const int MAX_SEQUENCE = 100;              // Maximum sequence length
int sequence[MAX_SEQUENCE];                // Stores the color sequence
int sequenceLength = 0;                    // Current sequence length
int userInputIndex = 0;                    // Tracks user's position in sequence

// ----- GAME STATES -----
enum GameState {
  WELCOME,        // Show startup animation
  WAIT_FOR_START, // Wait for player to press start button
  PLAY_SEQUENCE,  // Display the sequence to memorize
  USER_INPUT,     // Wait for player to repeat the sequence
  GAME_OVER       // Show game over animation
};
GameState gameState = WELCOME;

// ----- DEBOUNCE TIMING -----
unsigned long lastButtonPress = 0;         // Last button press timestamp
const unsigned long debounceDelay = 200;   // Minimum time between button presses (ms)

// ========================================
// SETUP
// ========================================
void setup() {
  // Configure button pins with internal pullup resistors
  for (int i = 0; i < 4; i++) {
    pinMode(BUTTON_PINS[i], INPUT_PULLUP);
    pinMode(LED_PINS[i], OUTPUT);
  }
  
  pinMode(BUZZER_PIN, OUTPUT);
  
}

// ========================================
// MAIN LOOP
// ========================================
void loop() {
  switch (gameState) {
    case WELCOME:
      welcomeSequence();
      gameState = WAIT_FOR_START;
      break;

    case WAIT_FOR_START:
      if (waitForStartButton()) {
        sequenceLength = 0;
        digitalWrite(LED_PINS[2], LOW);  // Turn off blinking LED
        delay(500);  // Short pause after start button press
        gameState = PLAY_SEQUENCE;
      }
      break;

    case PLAY_SEQUENCE:
      addToSequence();
      playSequence();
      userInputIndex = 0;
      lastButtonPress = millis();  // Reset debounce timer
      gameState = USER_INPUT;
      break;

    case USER_INPUT:
      {
        int input = readButton();
        if (input != -1) {
          playColor(input);
          
          // Check if input matches the sequence
          if (input != sequence[userInputIndex]) {
            gameState = GAME_OVER;
          } else {
            userInputIndex++;
            // Check if entire sequence was entered correctly
            if (userInputIndex >= sequenceLength) {
              delay(1000); //Delay after succesful level completion
              gameState = PLAY_SEQUENCE;
            }
          }
        }
      }
      break;

    case GAME_OVER:
      gameOver();
      gameState = WAIT_FOR_START;
      break;
  }
}

// ========================================
// ADD NEW COLOR TO SEQUENCE
// ========================================
// Adds a random color to the sequence.
// Ensures first color is never green (to avoid confusion with start button).
void addToSequence() {
  if (sequenceLength < MAX_SEQUENCE) {
    int newColor;
    do {
      newColor = random(0, 4);
    } while (sequenceLength == 0 && newColor == 2);  // First color cannot be green
    
    sequence[sequenceLength++] = newColor;
  }
}

// ========================================
// PLAY ENTIRE SEQUENCE
// ========================================
// Plays back the current sequence for the player to memorize.
void playSequence() {
  for (int i = 0; i < sequenceLength; i++) {
    playColor(sequence[i]);
    delay(300);  // Pause between colors
  }
}

// ========================================
// PLAY SINGLE COLOR
// ========================================
// Lights up LED and plays tone for a single color.
void playColor(int index) {
  int duration = 500;

  
  digitalWrite(LED_PINS[index], HIGH);
  tone(BUZZER_PIN, TONES[index], duration);
  delay(duration);
  digitalWrite(LED_PINS[index], LOW);
}

// ========================================
// READ BUTTON INPUT
// ========================================
// Non-blocking button read with debounce.
// Returns button index (0-3) or -1 if no button pressed.
int readButton() {
  // Debounce: ignore inputs that are too close together
  if (millis() - lastButtonPress < debounceDelay) {
    return -1;
  }
  
  // Check each button
  for (int i = 0; i < 4; i++) {
    if (digitalRead(BUTTON_PINS[i]) == LOW) {
      lastButtonPress = millis();
      
      // Wait for button release
      while (digitalRead(BUTTON_PINS[i]) == LOW) {
        delay(10);
      }
      
      return i;
    }
  }
  return -1;
}

// ========================================
// GAME OVER ANIMATION
// ========================================
// Flashes all LEDs and plays error sound.
void gameOver() {
  delay(300);  // Wait for button release
  
  // Flash all LEDs 3 times with low buzzer tone
  for (int i = 0; i < 3; i++) {
    tone(BUZZER_PIN, 100, 200);
    for (int j = 0; j < 4; j++) {
      digitalWrite(LED_PINS[j], HIGH);
    }
    delay(200);
    noTone(BUZZER_PIN);
    for (int j = 0; j < 4; j++) {
      digitalWrite(LED_PINS[j], LOW);
    }
    delay(200);
  }
  
  delay(1000);  // Pause before allowing restart
}

// ========================================
// WELCOME ANIMATION
// ========================================
// Plays startup sequence: cascade effect followed by synchronized flashes.
void welcomeSequence() {
  int duration = 150;

  // Cascade: each color lights up in sequence (2 times)
  for (int repeat = 0; repeat < 2; repeat++) {
    for (int i = 0; i < 4; i++) {
      digitalWrite(LED_PINS[i], HIGH);
      tone(BUZZER_PIN, TONES[i], duration);
      delay(duration);
      digitalWrite(LED_PINS[i], LOW);
      noTone(BUZZER_PIN);
      delay(100);
    }
  }

  // Synchronized flash: all colors at once (4 times)
  for (int repeat = 0; repeat < 4; repeat++) {
    for (int i = 0; i < 4; i++) {
      digitalWrite(LED_PINS[i], HIGH);
    }
    tone(BUZZER_PIN, 400, duration);
    delay(duration);
    noTone(BUZZER_PIN);
    for (int i = 0; i < 4; i++) {
      digitalWrite(LED_PINS[i], LOW);
    }
    delay(150);
  }
}

// ========================================
// WAIT FOR START BUTTON
// ========================================
// Non-blocking function that blinks green LED while waiting for start button.
// Returns true when start button (green, index 2) is pressed.
bool waitForStartButton() {
  static unsigned long previousMillis = 0;
  const long interval = 300;  // Blink interval (ms)
  static bool ledState = false;

  unsigned long currentMillis = millis();

  // Toggle LED at regular intervals
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    ledState = !ledState;
    digitalWrite(LED_PINS[2], ledState ? HIGH : LOW);
  }

  // Check if start button is pressed
  if (digitalRead(BUTTON_PINS[2]) == LOW) {
    while (digitalRead(BUTTON_PINS[2]) == LOW) {
      delay(10);  // Wait for button release
    }
    return true;
  }

  return false;
}