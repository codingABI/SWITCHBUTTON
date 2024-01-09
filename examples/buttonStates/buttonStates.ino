/*
 * Example to get all button states
 */

#include <SWITCHBUTTON.h>

// Button pin
#define SW_PIN 6
// Button object
SWITCHBUTTON switchButton(SW_PIN);

void setup() {
  Serial.begin(9600);
  // If you have no external pullup resistor, set pin to INPUT_PULLUP
  pinMode(SW_PIN,INPUT_PULLUP);
}

void loop() {
  static byte lastButtonState = SWITCHBUTTON::UNKNOWN;
  byte buttonState;

  // Start getButton() frequently in your loop to avoid missing button presses
  buttonState = switchButton.getButton();
  if (buttonState != lastButtonState) {
    lastButtonState = buttonState;
    switch (buttonState) {
      case SWITCHBUTTON::SHORTPRESSED:
        Serial.println("Pressed");
        break;
      case SWITCHBUTTON::LONGPRESSED:
        Serial.println("Long pressed");
        break;
      case SWITCHBUTTON::MISSED:
        Serial.println("Missed");
        break;
      case SWITCHBUTTON::INPROGRESS:
        Serial.println("In progress");
        break;
      case SWITCHBUTTON::IDLE:
        Serial.println("Idle");
        break;
      case SWITCHBUTTON::DEBOUNCING:
        Serial.println("Debouncing");
        break;
      case SWITCHBUTTON::LONGPRESSEDRELEASED:
        Serial.println("Long pressed released");
        break;
    }
  }
}