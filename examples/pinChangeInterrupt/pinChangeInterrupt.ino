/*
 * Example to get the final button press state with a "bad", blocking loop 
 * (for example a delay or long running function) by using pin change interrupts
 */
 
#include <SWITCHBUTTON.h>

// Button pin
#define SW_PIN 6
// Button object
SWITCHBUTTON switchButton(SW_PIN);

// Button states
volatile bool v_shortPressed = false;
volatile bool v_longPressed = false;

// Enable pin change interrupt
void pciSetup(byte pin) {
  *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));  // enable pin
  PCIFR  |= bit (digitalPinToPCICRbit(pin)); // clear any outstanding interrupt
  PCICR  |= bit (digitalPinToPCICRbit(pin)); // enable interrupt for the group
}

// Update button states (will be used in ISR and loop)
void updateSwitchStates() {
  switch (switchButton.getButton()) {
    case SWITCHBUTTON::SHORTPRESSED:
      v_shortPressed = true;
      break;
    case SWITCHBUTTON::LONGPRESSED:
      v_longPressed = true;
      break;
  }  
}

// ISR to handle pin change interrupt for D0 to D7 here
ISR (PCINT2_vect) {
  // Update button states from momentary switch
  updateSwitchStates();
}

void setup() {
  Serial.begin(9600);

  // If you have no external pullup resistor, set pin to INPUT_PULLUP
  pinMode(SW_PIN,INPUT_PULLUP);
  
  // Set pin change interrupt for momentary switch
  pciSetup(SW_PIN);
}

void loop() {
  // For local copies of button states
  bool shortPressed = false;
  bool longPressed = false;

  // Example for a "bad", blocking function in loop
  delay(2000); 
  // Blocking longer than LONGPRESSEDMS or LONGPRESSEDDEADTIMEMS will delay the long pressed detection
  
  // Update button states from momentary switch, make an "interrupt save" copy and reset the states
  cli();
  updateSwitchStates();
  shortPressed = v_shortPressed;
  longPressed = v_longPressed;
  v_shortPressed = false;
  v_longPressed = false;
  sei();
  
  // Show button state

  if (shortPressed) { // Short pressed?
    Serial.println("Short pressed");
  }

  if (longPressed) { // Long pressed?
    Serial.println("Long pressed");
  }  
}