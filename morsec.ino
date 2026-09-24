#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "I2S.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SAMPLE_RATE 16000
#define RECORD_SECONDS 5

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const int dotButtonPin = 18;
const int dashButtonPin = 19;
const int selectButtonPin = 23;
const int ledPin = 2;
const int buzzerPin = 25;

I2S microphone(ICS43434);

int mode = 0;
bool modeConfirmed = false;
bool waitingForSerialConfirm = false;
String morseInput = "";
String translatedText = "";
String serialInput = "";
unsigned long lastPressTime = 0;

char translateMorse(String morse) {
  if (morse == ".-") return 'A';
  if (morse == "-...") return 'B';
  if (morse == "-.-.") return 'C';
  if (morse == "-..") return 'D';
  if (morse == ".") return 'E';
  if (morse == "..-.") return 'F';
  if (morse == "--.") return 'G';
  if (morse == "....") return 'H';
  if (morse == "..") return 'I';
  if (morse == ".---") return 'J';
  if (morse == "-.-") return 'K';
  if (morse == ".-..") return 'L';
  if (morse == "--") return 'M';
  if (morse == "-.") return 'N';
  if (morse == "---") return 'O';
  if (morse == ".--.") return 'P';
  if (morse == "--.-") return 'Q';
  if (morse == ".-.") return 'R';
  if (morse == "...") return 'S';
  if (morse == "-") return 'T';
  if (morse == "..-") return 'U';
  if (morse == "...-") return 'V';
  if (morse == ".--") return 'W';
  if (morse == "-..-") return 'X';
  if (morse == "-.--") return 'Y';
  if (morse == "--..") return 'Z';
  if (morse == "-----") return '0';
  if (morse == ".----") return '1';
  if (morse == "..---") return '2';
  if (morse == "...--") return '3';
  if (morse == "....-") return '4';
  if (morse == ".....") return '5';
  if (morse == "-....") return '6';
  if (morse == "--...") return '7';
  if (morse == "---..") return '8';
  if (morse == "----.") return '9';
  return '?';
}

String getMorseForChar(char c) {
  c = toupper(c);
  switch (c) {
    case 'A': return ".-";
    case 'B': return "-...";
    case 'C': return "-.-.";
    case 'D': return "-..";
    case 'E': return ".";
    case 'F': return "..-.";
    case 'G': return "--.";
    case 'H': return "....";
    case 'I': return "..";
    case 'J': return ".---";
    case 'K': return "-.-";
    case 'L': return ".-..";
    case 'M': return "--";
    case 'N': return "-.";
    case 'O': return "---";
    case 'P': return ".--.";
    case 'Q': return "--.-";
    case 'R': return ".-.";
    case 'S': return "...";
    case 'T': return "-";
    case 'U': return "..-";
    case 'V': return "...-";
    case 'W': return ".--";
    case 'X': return "-..-";
    case 'Y': return "-.--";
    case 'Z': return "--..";
    case '0': return "-----";
    case '1': return ".----";
    case '2': return "..---";
    case '3': return "...--";
    case '4': return "....-";
    case '5': return ".....";
    case '6': return "-....";
    case '7': return "--...";
    case '8': return "---..";
    case '9': return "----.";
    case ' ': return " ";
    default: return "";
  }
}

void showModeSelection() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Select Mode:");
  display.setCursor(0, 20);
  display.println(mode == 0 ? "> Morse -> Text" : "  Morse -> Text");
  display.setCursor(0, 40);
  display.println(mode == 1 ? "> Voice -> Morse" : "  Voice -> Morse");
  display.display();
}

void sendAudioToPython() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println("Speak!");
  display.display();

  Serial.println("START_AUDIO");
  Serial.flush();
  delay(300);

  const int totalSamples = SAMPLE_RATE * RECORD_SECONDS;
  int samplesSent = 0;
  int32_t i2sBuffer[256];

  while (samplesSent < totalSamples) {
    int bytesRead = microphone.Read((char*)i2sBuffer, sizeof(i2sBuffer));
    int valuesRead = bytesRead / sizeof(int32_t);

    for (int i = 0; i + 1 < valuesRead && samplesSent < totalSamples; i += 2) {
      int32_t raw = i2sBuffer[i];
      int16_t pcm = (int16_t)(raw >> 16);
      Serial.write((uint8_t*)&pcm, sizeof(pcm));
      samplesSent++;
    }
  }

  Serial.flush();
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Processing...");
  display.display();
}

void waitForPython() {
  serialInput = "";

  while (serialInput.length() == 0) {
    if (Serial.available()) {
      serialInput = Serial.readStringUntil('\n');
      serialInput.trim();
    }
  }

  if (serialInput == "NO_SPEECH") {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("No speech");
    display.println("detected.");
    display.display();
    delay(2000);
    modeConfirmed = false;
    showModeSelection();
    return;
  }

  waitingForSerialConfirm = true;
}

void playTextAsMorse() {
  waitingForSerialConfirm = false;
  translatedText = "";

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println("Sending...");
  display.display();
  delay(1000);

  for (char c : serialInput) {
    String morse = getMorseForChar(c);
    translatedText += c;

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Text:");
    display.setTextSize(2);
    display.println(translatedText);
    display.display();

    if (c == ' ') {
      delay(1400);
      continue;
    }

    for (char mc : morse) {
      digitalWrite(ledPin, HIGH);
      tone(buzzerPin, 1000);
      delay(mc == '.' ? 200 : 600);
      digitalWrite(ledPin, LOW);
      noTone(buzzerPin);
      delay(200);
    }

    delay(600);
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println("Done.");
  display.display();
  delay(1500);

  modeConfirmed = false;
  translatedText = "";
  morseInput = "";
  serialInput = "";
  waitingForSerialConfirm = false;
  showModeSelection();
}

void setup() {
  Serial.begin(921600);

  pinMode(dotButtonPin, INPUT_PULLUP);
  pinMode(dashButtonPin, INPUT_PULLUP);
  pinMode(selectButtonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (true) delay(1000);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.display();
  showModeSelection();
}

void loop() {
  if (!modeConfirmed) {
    if (digitalRead(dotButtonPin) == LOW) {
      mode++;
      if (mode > 1) mode = 0;
      showModeSelection();
      delay(300);
    }

    if (digitalRead(selectButtonPin) == LOW) {
      modeConfirmed = true;
      delay(300);

      if (mode == 0) {
        translatedText = "";
        morseInput = "";
        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.println("Enter Morse:");
        display.display();
      } else if (mode == 1) {
        sendAudioToPython();
        waitForPython();
      }
    }

    return;
  }

  if (mode == 0) {
    bool buttonPressed = false;

    if (digitalRead(dotButtonPin) == LOW) {
      morseInput += ".";
      buttonPressed = true;
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(0, 0);
      display.println(morseInput);
      display.display();
      delay(250);
    }

    if (digitalRead(dashButtonPin) == LOW) {
      morseInput += "-";
      buttonPressed = true;
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(0, 0);
      display.println(morseInput);
      display.display();
      delay(250);
    }

    if (buttonPressed) lastPressTime = millis();

    if (millis() - lastPressTime > 1500 && morseInput.length() > 0) {
      translatedText += translateMorse(morseInput);
      morseInput = "";
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(0, 0);
      display.println(translatedText);
      display.display();
    }
  } else if (mode == 1 && waitingForSerialConfirm) {
    playTextAsMorse();
  }
}
