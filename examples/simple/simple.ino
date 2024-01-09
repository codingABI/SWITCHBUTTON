/*
 * Example to get the final button press state
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
  // Start getButton() frequently in your loop to avoid missing button presses
  switch (switchButton.getButton()) {
    case SWITCHBUTTON::SHORTPRESSED:
      Serial.println("Pressed");
      break;
    case SWITCHBUTTON::LONGPRESSED:
      Serial.println("Long pressed");
      break;
  }
}