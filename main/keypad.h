/*
 * @file main/keypad.h
 *
 * @proj imp-term
 * @brief PIN checking and memory access related functions
 * @author Lukas Tesar <xtesar43@stud.fit.vut.cz>
 * @year 2024
*/

#ifndef IMP_TERM_KEYPAD_H
#define IMP_TERM_KEYPAD_H

// CONFIGURABLE OPTIONS

#define KEYPAD_DEFAULT_ACCESS_PIN "1234"
#define KEYPAD_DEFAULT_ADMIN_PIN "00000000"

#define KEYPAD_STORAGE_NS "keypad"
#define KEYPAD_PIN_MIN_LEN 4
#define KEYPAD_PIN_MAX_LEN 10
#define KEYPAD_PIN_SUBMIT_KEY '#'
#define KEYPAD_PIN_CHANGE_KEY '*'

#define DOOR_OPEN_FOR_SEC 10 // Time in seconds the door stay open


// EXPORTED SYMBOLS

/*
 * @brief Initialize NVS storage and if empty, set default values
*/
void nvs_configure();

/*
 * @brief Handle a keypress on the keypad
*/
noreturn void keypad_handler_task();

/*
 * @brief Handle a door open/close event
*/
noreturn void door_handler_task();


#endif // IMP_TERM_KEYPAD_H
