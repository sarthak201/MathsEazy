#include <LiquidCrystal.h>
#include <Wire.h>
#include <MPU6050.h>
#include <MFRC522.h>

#define NOTE_CORRECT_NOTE 300
#define NOTE_WRONG_NOTE 40

#define RFID_CARD "A0 E7 C3 25"
#define RFID_TAG1 "13 75 CB 1B"
#define RFID_TAG2 "0B 94 54 D3"
#define SS_PIN 10
#define RST_PIN 9
#define buttonPin 8
#define buzzorPin 0

const float initialX = 0.4; // Initial X acceleration (adjust as needed)
const float initialY = 0.4; // Initial Y acceleration (adjust as needed)

MFRC522 mfrc522(SS_PIN, RST_PIN);
LiquidCrystal lcd(6, 7, 5, 4, 3, 2);

String question;
String option1, option2, option3, option4;
int correctOption;
int correctAnswer;
int totalPoints = 0;

MPU6050 mpu;
int buttonState = 0;
int lastButtonState = 0;
bool gameActive = false; // Variable to track whether the game is active

void setup() {
  Serial.begin(9600);
  Wire.begin();
  mpu.initialize();
  pinMode(buttonPin, INPUT_PULLUP);
  // pinMode(buzzorPin, OUTPUT);
  SPI.begin();
  mfrc522.PCD_Init();
  lcd.begin(20, 4);
  Serial.println("RFID Reader Initialized");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Welcome To Math Game");
  lcd.setCursor(0, 1);
  lcd.print("Scan your tag to");
  lcd.setCursor(0, 2);
  lcd.print("start the Game");
}

void displayQuestionAndOptions(String question, String option1, String option2, String option3, String option4, int hoverOption) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(question);
  
  lcd.setCursor(8, 1);
  lcd.print("1. " + option1);
  lcd.setCursor(2, 2);
  lcd.print("2. " + option2);
  
  lcd.setCursor(15, 2);
  lcd.print("3. " + option3);
  lcd.setCursor(8, 3);
  lcd.print("4. " + option4);
  
  if (hoverOption == 1)
  {
    lcd.setCursor(6, 1);
    lcd.print("> 1. " + option1);
  }
  else if (hoverOption == 2)
  {
    lcd.setCursor(0, 2);
    lcd.print("> 2. " + option2);
  }
  else if (hoverOption == 3)
  {
    lcd.setCursor(13, 2);
    lcd.print("> 3. " + option3);
  }
  else if (hoverOption == 4)
  {
    lcd.setCursor(6, 3);
    lcd.print("> 4. " + option4);
  }
}

int getSelectedOption() {
  int16_t ax, ay, az;
  int16_t gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  
  float angularVelX = (float)ax / 131.0; // Sensitivity scale factor is 131 LSB/°/s
  float angularVelY = (float)ay / 131.0;

  float angleX = angularVelX - initialX;
  float angleY = angularVelY - initialY;
  
  if (angularVelY > 50) {
    return 3;
  } else if (angularVelY < -50) {
    return 2;
  } else if (angularVelX < -50) {
    return 1;
  } else if (angularVelX > 50) {
    return 4;
  } else {
    return 0;
  }
}

void playCorrectSound() {
  tone(buzzorPin, NOTE_CORRECT_NOTE); // Play correct answer sound
  delay(500); // Delay for sound duration
  noTone(buzzorPin); // Stop sound
}

void playWrongSound() {
  tone(buzzorPin, NOTE_WRONG_NOTE); // Play wrong answer sound
  delay(500); // Delay for sound duration
  noTone(buzzorPin); // Stop sound
}

void generateQuestion() {
  int num1 = random(1, 21);
  int num2 = random(1, 21);
  int operatorChar = random(0, 4);
  
  switch (operatorChar) {
    case 0:
      question = String(num1) + " + " + String(num2) + " = ?";
      correctAnswer = num1 + num2;
      break;
    case 1:
      question = String(num1) + " - " + String(num2) + " = ?";
      correctAnswer = num1 - num2;
      break;
    case 2:
      question = String(num1) + " * " + String(num2) + " = ?";
      correctAnswer = num1 * num2;
      break;
    case 3:
      question = String(num1) + " / " + String(num2) + " = ?";
      correctAnswer = num1 / num2;
      break;
  }

  // Generate three incorrect answers
  int incorrectAnswer1 = correctAnswer + random(1, 11);
  int incorrectAnswer2 = correctAnswer - random(1, 11);
  int incorrectAnswer3 = correctAnswer * random(1, 4);

  if (incorrectAnswer1 == correctAnswer)
  {
    incorrectAnswer1 += random(-21,-1);
  }
  else if (incorrectAnswer2 == correctAnswer)
  {
    incorrectAnswer2 += random(-4,7);
  }
  else if (incorrectAnswer3 == correctAnswer)
  {
    incorrectAnswer3 += random(1,21);
  }

  // Randomly assign the correct answer to one of the four options
  correctOption = random(1, 5);
  switch (correctOption) {
    case 1:
      option1 = String(correctAnswer);
      option2 = String(incorrectAnswer1);
      option3 = String(incorrectAnswer2);
      option4 = String(incorrectAnswer3);
      break;
    case 2:
      option1 = String(incorrectAnswer1);
      option2 = String(correctAnswer);
      option3 = String(incorrectAnswer2);
      option4 = String(incorrectAnswer3);
      break;
    case 3:
      option1 = String(incorrectAnswer1);
      option2 = String(incorrectAnswer2);
      option3 = String(correctAnswer);
      option4 = String(incorrectAnswer3);
      break;
    case 4:
      option1 = String(incorrectAnswer1);
      option2 = String(incorrectAnswer2);
      option3 = String(incorrectAnswer3);
      option4 = String(correctAnswer);
      break;
  }
}

void loop() {
  if (gameActive &&  ! mfrc522.PICC_ReadCardSerial()) {
    displayQuestionAndOptions(question, option1, option2, option3, option4, getSelectedOption());
    
    Serial.println(getSelectedOption());

    buttonState = digitalRead(buttonPin);
    // Serial.println(buttonState);
    if (buttonState == HIGH) {
      int selectedOption = getSelectedOption();
      
      lcd.clear();
      if (selectedOption == correctOption)
      {
        // playCorrectSound();
        lcd.println("Correct Answer!");
        delay(1000);
        generateQuestion();
      }
      else
      {
        // playWrongSound();
        lcd.println("Wrong Answer!");
        delay(1000);
        generateQuestion();
      }
    }
  }
  // Look for new RFID cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Check if the card is one of the allowed tags
  String tag = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    tag.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    tag.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  tag.toUpperCase();

  if (tag.substring(1) == RFID_TAG1 || tag.substring(1) == RFID_TAG2 || tag.substring(1) == RFID_CARD) {
    Serial.println("Hello");
    if (!gameActive) {
      gameActive = true;
      generateQuestion();
    } else {
      gameActive = false;
      lcd.clear();
      lcd.println("Game ended");
    }
  }

      delay(2000);

}