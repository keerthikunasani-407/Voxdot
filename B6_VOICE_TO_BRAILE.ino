#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

SoftwareSerial bt(2, 3);  // Bluetooth module

int totalColumns = 16;
int totalRows = 2;
LiquidCrystal_I2C lcd(0x27, totalColumns, totalRows);  // LCD

// Stepper motor pin definitions
#define dirPinHorizontal 5   // Horizontal motor direction
#define stepPinHorizontal 6  // Horizontal motor step
#define dirPinVertical 7     // Vertical motor direction
#define stepPinVertical 8    // Vertical motor step
#define solenoidPin 4        // Pin for solenoid (activating dots)

// Motor step definitions
float stepsPerRevolutionHorizontal1 = 227.8;  // Horizontal motor 1st revolution
float stepsPerRevolutionHorizontal2 = 406.2;  // Horizontal motor for 4th dot
float stepsPerRevolutionVertical = 185.1;     // Vertical motor
float stepsPerRevolutionVerticalLong = 944.2;  // Long vertical motor movement
float stepsPerRevolutionHorizontalLong = 20790; // Horizontal long movement

// Distance per step (in mm)
float distancePerStepHorizontal = 0.05;  // Example: 0.05 mm per step for horizontal motor
float distancePerStepVertical = 0.1;     // Example: 0.1 mm per step for vertical motor

// Braille patterns for "A" to "Z" and some punctuation
int braillePatterns[31][6] = {
  {1, 0, 0, 0, 0, 0},  // A
  {1, 1, 0, 0, 0, 0},  // B
  {1, 0, 0, 0, 0, 1},  // C
  {1, 0, 0, 0, 1, 1},  // D
  {1, 0, 0, 0, 1, 0},  // E
  {1, 1, 0, 0, 0, 1},  // F
  {1, 1, 0, 0, 1, 1},  // G
  {1, 1, 0, 0, 1, 0},  // H
  {0, 1, 0, 0, 0, 1},  // I
  {0, 1, 0, 0, 1, 1},  // J
  {1, 0, 1, 0, 0, 0},  // K
  {1, 1, 1, 0, 0, 0},  // L
  {1, 0, 1, 0, 0, 1},  // M
  {1, 0, 1, 0, 1, 0},  // N
  {1, 0, 1, 0, 1, 0},  // O
  {1, 1, 1, 0, 0, 1},  // P
  {1, 1, 1, 0, 1, 1},  // Q
  {1, 1, 1, 0, 1, 0},  // R
  {0, 1, 1, 0, 0, 1},  // S
  {0, 1, 1, 0, 1, 1},  // T
  {1, 0, 1, 1, 0, 0},  // U
  {1, 1, 1, 1, 0, 0},  // V
  {0, 1, 0, 1, 1, 1},  // W
  {1, 0, 1, 1, 0, 1},  // X
  {1, 0, 1, 1, 1, 1},  // Y
  {1, 0, 1, 1, 1, 0},  // Z
  {0, 1, 0, 1, 1, 0},  // .
  {0, 1, 0, 0, 0, 0},  // ,
  {0, 1, 0, 0, 1, 0},  // :
  {0, 1, 1, 0, 0, 0},  // ;
  {0, 1, 1, 1, 0, 0}   // ?
};

int iterationCount = 0;  // To track iterations
int currentLetter = 0;   // To track the current letter to print
int s = 0;  // Bluetooth signal state
int charactersPrinted = 0;  // Track how many characters have been printed on the current line

void setup() {
  lcd.init();
  lcd.backlight();
  
  bt.begin(9600);  // Initialize Bluetooth communication
  Serial.begin(9600);  // Start serial communication for debugging

  lcd.setCursor(0, 0);
  lcd.print("Bluetooth Ready");
  lcd.setCursor(0, 1);
  lcd.print("Waiting for data...");

  // Initialize motor control pins
  pinMode(stepPinHorizontal, OUTPUT);
  pinMode(dirPinHorizontal, OUTPUT);
  pinMode(stepPinVertical, OUTPUT);
  pinMode(dirPinVertical, OUTPUT);
  pinMode(solenoidPin, OUTPUT);
}

void loop() {
  if (bt.available()) {
    String incomingByte = bt.readString();  // Read the incoming data

    // Process each character in the received string
    for (int i = 0; i < incomingByte.length(); i++) {
      char currentChar = incomingByte.charAt(i);  // Get the current character

      if (currentChar >= 'A' && currentChar <= 'Z') {
        s = 1;  // Valid character is received
      } else if (currentChar == ' ') {
        s = 2;  // Space character received
      } else {
        s = 0;  // Invalid character received, ignore it
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Invalid char!");
      }

      // If valid character is received
      if (s == 1) {
        // Print the received character as Braille
        moveHorizontal(stepsPerRevolutionHorizontal2);
        printBraille(currentChar);
        s = 0;  // Reset the state after printing
      }

      // If space is received
      if (s == 2) {
        // Move to the next character position for space
        moveHorizontal(1040.2);  // Simulate space between letters
        s = 0;  // Reset the state after handling space
      }

      // After completing 32 characters, move vertically and horizontally more steps
      // Check if 4 or more characters have been printed
if (charactersPrinted >= 31) {
   
    // Move vertically upward (vertical motor)
    digitalWrite(dirPinVertical, LOW);  // Set direction for vertical movement (assuming upward is low)
    for (int i = 0; i < stepsPerRevolutionVerticalLong; i++) {
      digitalWrite(stepPinVertical, HIGH);   // Create a step pulse for vertical motor
      delayMicroseconds(2000);               // Control speed by adjusting delay
      digitalWrite(stepPinVertical, LOW);    // End the step pulse
      delayMicroseconds(2000);               // Control speed by adjusting delay
    }

    delay(1000);  // Wait for 1 second after completing vertical movement

    // Move horizontally backward (horizontal motor)
    digitalWrite(dirPinHorizontal, HIGH);  // Set direction to backward (horizontal)
    for (int i = 0; i < stepsPerRevolutionHorizontalLong; i++) {
      digitalWrite(stepPinHorizontal, HIGH);   // Create a step pulse for horizontal motor
      delayMicroseconds(2000);                 // Control speed by adjusting delay
      digitalWrite(stepPinHorizontal, LOW);    // End the step pulse
      delayMicroseconds(2000);                 // Control speed by adjusting delay
    }

    delay(1000);
    charactersPrinted = 0;  // Reset characters printed
    }
   }
  }
  
  
}

// Function to print the Braille pattern for a specific character
void printBraille(char letter) {
  int letterIndex = letter - 'A';  // Map letter to Braille pattern index

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Printing: ");
  lcd.print(letter);

  // Loop through Braille pattern for the current letter
  for (int dot = 0; dot < 6; dot++) {
    if (braillePatterns[letterIndex][dot] == 1) {
      activateDot();  // Activate the dot
    }

    digitalWrite(dirPinHorizontal, LOW);
    
    // Move motors to print each dot
    if (dot == 0) {
      moveVertical(-stepsPerRevolutionVertical);  // Move to the 1st dot
    } else if (dot == 1) {
      moveVertical(-stepsPerRevolutionVertical);  // Move to the 2nd dot
    } else if (dot == 2) {
      moveHorizontal(stepsPerRevolutionHorizontal1);  // Move to 3rd dot
    } else if (dot == 3) {
      moveVertical(stepsPerRevolutionVertical);  // Move to 4th dot
    } else if (dot == 4) {
      moveVertical(stepsPerRevolutionVertical);  // Move to 5th dot
    } else if (dot == 5) {
      //moveHorizontal(stepsPerRevolutionHorizontal2);  // Move to 6th dot
    }
  }

  delay(1000);  // Wait 1 second before printing next letter
  charactersPrinted++;  // Increment the count of characters printed
}

// Function to move the horizontal motor by the specified number of steps
void moveHorizontal(float steps) {
  for (int i = 0; i < steps; i++) {
    digitalWrite(stepPinHorizontal, HIGH);
    delayMicroseconds(1000);  // Controls the speed of the motor
    digitalWrite(stepPinHorizontal, LOW);
    delayMicroseconds(1000);  // Controls the speed of the motor
  }
}

// Function to move the vertical motor by the specified number of steps
void moveVertical(float steps1) {
  digitalWrite(dirPinVertical, steps1 > 0 ? HIGH : LOW);  // Set direction (positive for upward, negative for downward)
  
  for (int i = 0; i < abs(steps1); i++) {
    digitalWrite(stepPinVertical, HIGH);
    delayMicroseconds(1000);  // Controls the speed of the motor
    digitalWrite(stepPinVertical, LOW);
    delayMicroseconds(1000);  // Controls the speed of the motor
  }
}

// Function to activate a specific dot (solenoid)
void activateDot() {
  digitalWrite(solenoidPin, HIGH);  // Activate solenoid for the current dot
  delay(200);  // Simulate activation time
  digitalWrite(solenoidPin, LOW);   // Deactivate solenoid for the current dot
  delay(200);  // Simulate delay for next dot
}
