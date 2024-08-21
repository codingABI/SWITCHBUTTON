/**
 * Class: SWITCHBUTTON
 *
 * Arduino Library for a momentary switch button.
 *
 * This library
 * - detects and differentiates between short and long button presses
 * - supports debouncing
 * - works nonblocking and without delay()
 * - supports sleep modes (see example "sleepmode")
 *
 * Home: https://github.com/codingABI/SWITCHBUTTON
 *
 * @author codingABI https://github.com/codingABI/
 * @copyright CC0
 * @file SWITCHBUTTON.h
 * @version 1.0.1
 */
#pragma once

/** Library version */
#define SWITCHBUTTON_VERSION "1.0.1"

#include <arduino.h>

/** Time duration in milliseconds to detect a button long press */
#define LONGPRESSEDMS 1000
/** Default debounce time in milliseconds after a rising or falling signal edge */
#define DEBOUNCEMS 100
/** Splits a sustained long press in individuals long presses after these milliseconds */
#define LONGPRESSEDDEADTIMEMS 500

/** Class for a momentary switch button. */
class SWITCHBUTTON {
  public:
    /** Button logical states */
    enum buttonLogicalStates
    {
      IDLE, /**< Button is not pressed and is idle */
      SHORTPRESSED, /**< Button was short pressed */
      LONGPRESSED, /**< Button was long pressed */
      LONGPRESSEDRELEASED, /**< Button was released after a long press */
      INPROGRESS, /**< Button press is in progress */
      MISSED, /**< Incomplete long press was detected */
      DEBOUNCING /**< Button is blocked for debouncing */
    };

    /** Button physical states */
    enum buttonPhysicalStates
    {
      RELEASED, /**< Button not pressed */
      PRESSED,  /**< Button pressed */
      UNKNOWN /**< Button never checked or set */
    };

    /**@brief
     * Constructor of a the momentary switch object
     *
     * @param[in] sw_pin Digital input pin connected to the momentary switch
     * @param[in] inputPulledUp Set to true (=default), if momentary switch is pulled up by a resistor or pinMode(,INPUT_PULLUP) otherwise set parameter to false
     */
    SWITCHBUTTON(byte sw_pin, bool inputPulledUp=true)
    {
      m_sw_pin = sw_pin; // Digital input pin for the button
      m_inputPulledUp = inputPulledUp; // Has button a pullup resistor (LOW=pressed,HIGH=released)?
      m_debounceTimeMS = DEBOUNCEMS;
      mv_state = UNKNOWN;
      mv_lastState = UNKNOWN;
      mv_lastLongPressedMS = 0;
      mv_lastButtonStartMS = 0;
      mv_lastButtonChangeMS=0;
      mv_waitingRelease = false;
      mv_pendingLongPressed = false;
    }

    /**@brief
     * Returns the current logical state from stored button state
     *
     * If you do not use interrupts, you have to start setState() and checkButton() or a function using
     * these (for example getButton()) very frequently in your loop to prevent missing button presses
     *
     * @retval SWITCHBUTTON::IDLE                Button is not pressed and is idle
     * @retval SWITCHBUTTON::SHORTPRESSED        Button was short pressed
     * @retval SWITCHBUTTON::LONGPRESSED         Button was long pressed
     * @retval SWITCHBUTTON::LONGPRESSEDRELEASED Button was released after a long pressed
     * @retval SWITCHBUTTON::INPROGRESS          Button press is in progress
     * @retval SWITCHBUTTON::MISSED              Incomplete long press was detected
     * @retval SWITCHBUTTON::DEBOUNCING          Button is blocked for debouncing
     */
    byte checkButton()
    {
      unsigned long currentMillis = millis();
      // When first check or after debounce time
      if ((mv_lastState == UNKNOWN)
        || (currentMillis-mv_lastButtonChangeMS > m_debounceTimeMS)) {
        mv_lastButtonChangeMS = currentMillis - m_debounceTimeMS - 1; // Prevent overrun
        if ((mv_state==RELEASED) && (mv_lastState == UNKNOWN)) { // Init
          mv_lastState = mv_state;
          return IDLE;
        }
        if (mv_state != mv_lastState) { // Button state has changed
          if (mv_state == PRESSED) { // Rising edge
            mv_waitingRelease = true;
            mv_lastButtonStartMS = currentMillis;
          }
          mv_lastButtonChangeMS = currentMillis;
          mv_lastState = mv_state;
        }
        if (mv_state == PRESSED) { // Button is pressed
          if (mv_waitingRelease) {
            if ((currentMillis - mv_lastButtonStartMS > LONGPRESSEDMS)
              && (currentMillis-mv_lastLongPressedMS > LONGPRESSEDDEADTIMEMS)) {
              mv_lastButtonStartMS = currentMillis - LONGPRESSEDMS - 1; // Prevent overrun
              mv_lastLongPressedMS = currentMillis;
              mv_pendingLongPressed = true;
              return LONGPRESSED;
            } else return INPROGRESS;
          } else return INPROGRESS;
        } else { // Button is released
          if (mv_waitingRelease) {
            mv_waitingRelease = false;
            if (currentMillis - mv_lastButtonStartMS <= LONGPRESSEDMS){
              return SHORTPRESSED;
            } else {
              if (mv_pendingLongPressed) {
                mv_pendingLongPressed = false;
                return LONGPRESSEDRELEASED;
              }
              return MISSED; // Too long gap between rising edge and button release
            }
          } else return IDLE;
        }
      } else { // In debounce time
        return DEBOUNCING;
      }
    }

    /**@brief
     * Get current logical state for momentary switch
     *
     * Reads physical button state with DigitalRead() and checks current logical state by calling checkButton()
     *
     * @retval SWITCHBUTTON::IDLE                Button is not pressed and is idle
     * @retval SWITCHBUTTON::SHORTPRESSED        Button was short pressed
     * @retval SWITCHBUTTON::LONGPRESSED         Button was long pressed
     * @retval SWITCHBUTTON::LONGPRESSEDRELEASED Button was released after a long pressed
     * @retval SWITCHBUTTON::INPROGRESS          Button press is in progress
     * @retval SWITCHBUTTON::MISSED              Incomplete long press was detected
     * @retval SWITCHBUTTON::DEBOUNCING          Button is blocked for debouncing
     */
    byte getButton()
    {
      if (m_inputPulledUp) {
        setState((digitalRead(m_sw_pin)==LOW) ? PRESSED:RELEASED);
      } else {
        setState((digitalRead(m_sw_pin)==HIGH) ? PRESSED:RELEASED);
      }
      return checkButton();
    }

    /**@brief
     * Get last stored physical button state for the momentary switch
     *
     * @retval SWITCHBUTTON::IDLE    Button was not pressed and is idle
     * @retval SWITCHBUTTON::PRESSED Button was pressed
     * @retval SWITCHBUTTON::UNKNOWN Button was never checked or set
     */
    byte getState()
    {
      return mv_state;
    }

    /**@brief
     * Checks, if it save to go to sleep deeper than SLEEP_MODE_IDLE
     *
     * Returns true, if device has no pending button press.
     * Sleep mode SLEEP_MODE_IDLE is always possible but deeper sleep
     * modes, for example SLEEP_MODE_PWR_SAVE, are only save after
     * readyForSleep() returns true
     *
     * @retval true Yes, it is save to go to sleep deeper then SLEEP_MODE_IDLE
     * @retval false No, only SLEEP_MODE_IDLE is possible
     */
    bool readyForSleep()
    {
      return (checkButton() == IDLE);
    }

    /**@brief
     * Change debounce time for a rising or falling signal edge
     *
     * @param[in] debounceTimeMS Debounce time in milliseconds
     */
    void setDebounceTimeMS(unsigned int debounceTimeMS)
    {
      m_debounceTimeMS = debounceTimeMS;
    }

    /**@brief
     * Store physical button state for the momentary switch
     *
     * @param[in] state Button state: SWITCHBUTTON::PRESSED(=pressed) or SWITCHBUTTON::RELEASED(=released).
     */
    void setState(byte state)
    {
      if (state == PRESSED ) mv_state = PRESSED;
      if (state == RELEASED ) mv_state = RELEASED;
    }

  private:
    byte m_sw_pin;
    bool m_inputPulledUp;
    unsigned int m_debounceTimeMS;
    volatile unsigned long mv_lastLongPressedMS;
    volatile unsigned long mv_lastButtonStartMS;
    volatile unsigned long mv_lastButtonChangeMS;
    volatile byte mv_state;
    volatile byte mv_lastState;
    volatile bool mv_waitingRelease;
    volatile bool mv_pendingLongPressed;
};