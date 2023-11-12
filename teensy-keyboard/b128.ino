/**
 * B128 Keyboard Interpreter for Teensy++ 2.0
 * 
 * 2020, Nick Melnick - http://www.nmelnick.com
 * 
 * Based on code from:
 *  - Toshiba T1000 Keyboard interpreter, 2017, Chad Boughton
 *    https://www.instructables.com/Make-Any-Vintage-Keyboard-Work-With-a-Modern-PC/
 *  - Teensyduino Core Library, 2013, PJRC.COM, LLC.
 *    http://www.pjrc.com/teensy/
 *    https://codebender.cc/library/Keypad#Keypad.cpp
 * 
 * This could probably be improved a lot, I don't have enough C skills to even
 * be rusty at it.
 *
 * When connecting the hardware, the idea was to match the B128 keyboard's pin
 * number 1:1 with the Teensy's pin numbers. Unfortunately, pin 6 is shared with
 * the LED, which causes some difficulty. As such, pin 6 is mapped to pin 26,
 * but can be modified with the PIN_6 constant.
 * 
 * The keymap can largely be configured in your OS, however, some customization
 * can be made easily here. The keys[ROWS][COLS] mapping visually maps with the
 * key matrix found in the schematic(1), and you can make alterations there. To
 * make life easier while I try to find what is comfortable for me, constants
 * were created for the B128-specific keys, and they can be edited directly
 * instead of altering the matrix.
 * 
 * If a serial console is attached, this will output the USB keycodes, the
 * calculated key value, and whether the key is pressed/held/released. It will
 * also output modifier changes.
 * 
 * 1. http://www.zimmers.net/anonftp/pub/cbm/schematics/computers/b/
 */

#include <Keypad.h>

#ifdef M
#undef M
#endif
#define M(n) ((n) & 0xFF)

typedef uint KeyCode;

const byte ROWS = 6;
const byte COLS = 16;

// Pin 6 is routed to pin 26
const byte PIN_6 = 26;

// Commodore B128 specific key mapping
const KeyCode KEY_RVS = KEY_END;
const KeyCode KEY_GRAPH = KEY_PAGE_UP;
const KeyCode KEY_RUNSTOP = KEY_PAGE_DOWN;
const KeyCode KEY_CE = KEY_DELETE;
const KeyCode KEY_CMD = KEY_LEFT_ALT;
const KeyCode KEY_POUND = KEY_BACKSLASH;
const KeyCode KEY_PI = KEY_BACKSLASH;
const KeyCode KEY_QUESTION = KEY_NUM_LOCK;
const KeyCode KEY_00 = KEYPAD_0;

char keys[ROWS][COLS] = {
  {M(KEY_F1),        M(KEY_F2),M(KEY_F3),M(KEY_F4),M(KEY_F5),M(KEY_F6),M(KEY_F7),   M(KEY_F8),    M(KEY_F9),       M(KEY_F10),       M(KEY_DOWN),       M(KEY_UP),       M(KEY_HOME),     M(KEY_RVS),      M(KEY_GRAPH),     M(KEY_RUNSTOP)},
  {M(KEY_ESC),       M(KEY_1), M(KEY_2), M(KEY_3), M(KEY_4), M(KEY_5), M(KEY_7),    M(KEY_8),     M(KEY_9),        M(KEY_0),         M(KEY_EQUAL),      M(KEY_LEFT),     M(KEY_QUESTION), M(KEY_CE),       M(KEYPAD_ASTERIX),M(KEYPAD_SLASH)},
  {M(KEY_TAB),       M(KEY_Q), M(KEY_W), M(KEY_E), M(KEY_R), M(KEY_6), M(KEY_U),    M(KEY_I),     M(KEY_O),        M(KEY_MINUS),     M(KEY_POUND),      M(KEY_RIGHT),    M(KEYPAD_7),     M(KEYPAD_8),     M(KEYPAD_9),      M(KEYPAD_MINUS)},
  {0,                M(KEY_A), M(KEY_S), M(KEY_D), M(KEY_T), M(KEY_Y), M(KEY_J),    M(KEY_K),     M(KEY_L),        M(KEY_P),         M(KEY_RIGHT_BRACE),M(KEY_BACKSPACE),M(KEYPAD_4),     M(KEYPAD_5),     M(KEYPAD_6),      M(KEYPAD_PLUS)},
  {M(KEY_LEFT_SHIFT),M(KEY_Z), M(KEY_X), M(KEY_F), M(KEY_G), M(KEY_H), M(KEY_M),    M(KEY_COMMA), M(KEY_SEMICOLON),M(KEY_LEFT_BRACE),M(KEY_ENTER),      M(KEY_CMD),      M(KEYPAD_1),     M(KEYPAD_2),     M(KEYPAD_3),      M(KEYPAD_ENTER)},
  {M(KEY_LEFT_CTRL), 0,        M(KEY_C), M(KEY_V), M(KEY_B), M(KEY_N), M(KEY_SPACE),M(KEY_PERIOD),M(KEY_SLASH),    M(KEY_QUOTE),     M(KEY_PI),         0,               M(KEYPAD_0),     M(KEYPAD_PERIOD),M(KEY_00),        0}
};
byte rowPins[ROWS] = {1,2,3,4,5,PIN_6};
byte colPins[COLS] = {14,13,12,11,10,9,8,7,23,21,20,19,18,17,16,15};

Keypad kpd = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
String lastModifier = "";
bool keyShift = false;
bool keyCtrl = false;
bool keyAlt = false;

void setup() {
  Serial.begin(9600);
  delay(2000);
  Serial.println("*** commodore keyboard 128, v4.0 ***");
  Serial.println("ready.");
}

void loop() {
  // Fills kpd.key[] array with up-to 10 active keys.
  // Returns true if there are ANY active keys.
  if (kpd.getKeys()) {
    // Scan the whole key list
    for (int i = 0; i < LIST_MAX; i++) {
      // Only find keys that have changed state
      if (kpd.key[i].stateChanged) {
        reportKey(kpd.key[i].kchar, kpd.key[i].kcode, kpd.key[i].kstate);
        switch (kpd.key[i].kstate) {
          case PRESSED:
          case HOLD:
            keyDown(kpd.key[i].kchar, kpd.key[i].kcode);
            break;

          case RELEASED:
            keyUp(kpd.key[i].kchar, kpd.key[i].kcode);
            break;

          case IDLE:
            break;
        }
      }
    }
  }
}

/**
 * Provide name of KeyState
 */
String stateName(KeyState state) {
  switch (state) {
    case PRESSED:
      return "PRESSED";
    case HOLD:
      return "HOLD";
    case RELEASED:
      return "RELEASED";
    case IDLE:
      return "IDLE";
  }
  return "UNKNOWN";
}

/**
 * Output keypress to serial
 */
void reportKey(int keyCode, int kcode, KeyState state) {
  if (state == IDLE) {
    return;
  }
  Serial.print("USB Keycode ");
  Serial.print(keyCode);
  Serial.print(" kcode ");
  Serial.print(kcode);
  Serial.print(" is ");
  Serial.println(stateName(state));
}

/**
 * Takes the current state of the modifier booleans and returns a flagged char
 * to provide to set_modifier().
 */
char modifiers() {
  char modifier = 0;
  String shift = "----- ";
  String ctrl = "---- ";
  String alt = "--- ";
  if (keyShift) {
    modifier |= MODIFIERKEY_SHIFT;
    shift = "shift ";
  }
  if (keyCtrl) {
    modifier |= MODIFIERKEY_CTRL;
    ctrl = "ctrl ";
  }
  if (keyAlt) {
    modifier |= MODIFIERKEY_ALT;
    alt = "alt ";
  }
  String thisModifier = String(shift + ctrl + alt);
  if (thisModifier.compareTo(lastModifier) != 0) {
    Serial.print("Modifiers: ");
    Serial.println(thisModifier);
  }
  lastModifier = thisModifier;
  return modifier;
}

/**
 * Send down to keyEvent()
 */
void keyDown(char kchar, int kcode) {
  keyEvent(true, kchar, kcode);
}

/**
 * Send up to keyEvent()
 */
void keyUp(char kchar, int kcode) {
  keyEvent(false, kchar, kcode);
}

/**
 * Perform a key event: output a key, alter modifiers based on special keys.
 */
void keyEvent(bool down, char kchar, int kcode) {
  // release all the keys at the same instant
  Keyboard.set_key1(down ? kchar : 0);

  // This if statement is a hack due to the bitmask operation forcing
  // KEY_A and KEY_LEFT_ALT to have the same value
  if (kcode != 49) {
    switch (kchar) {
      case char(KEY_LEFT_SHIFT):
        keyShift = down;
        break;
      case char(KEY_LEFT_CTRL):
        keyCtrl = down;
        break;
      case char(KEY_LEFT_ALT):
        keyAlt = down;
        break;
    }
  }
  Keyboard.set_modifier(modifiers());
  Keyboard.send_now();
}

