/*
 * Example to get the final button press state while using sleep modes
 */

#include <avr/sleep.h>
#include <SWITCHBUTTON.h>

// Button pin
#define SW_PIN 6
/** Button object */
SWITCHBUTTON switchButton(SW_PIN);

// Enable pin change interrupt
void pciSetup(byte pin) {
  *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin)); // enable pin
  PCIFR  |= bit (digitalPinToPCICRbit(pin)); // clear any outstanding interrupt
  PCICR  |= bit (digitalPinToPCICRbit(pin)); // enable interrupt for the group
}

// ISR to handle pin change interrupt for D0 to D7 here
ISR (PCINT2_vect) {
  // Empty, only as a wakeup trigger
}

void setup() {
  Serial.begin(9600);

  // If you have no external pullup resistor for the button, set pin to INPUT_PULLUP
  pinMode(SW_PIN,INPUT_PULLUP);

  // Set pin change interrupt for momentary switch
  pciSetup(SW_PIN);
}

void loop() {
  // Default sleep mode (SWITCHBUTTON library allows everytime SLEEP_MODE_IDLE)
  byte selectedSleepMode = SLEEP_MODE_IDLE;

  // Start getButton() frequently in your loop to avoid missing button presses
  switch (switchButton.getButton()) {
    case SWITCHBUTTON::SHORTPRESSED:
      Serial.println("Short pressed");
      break;
    case SWITCHBUTTON::LONGPRESSED:
      Serial.println("Long pressed");
      break;
    case SWITCHBUTTON::IDLE:
      // Set deeper sleep mode, because now we need no millis()&Co
      selectedSleepMode = SLEEP_MODE_PWR_DOWN;
      break;
  }
  Serial.flush();
  set_sleep_mode(selectedSleepMode);
  sleep_mode();
}