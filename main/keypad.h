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

#include <nvs.h>
#include <nvs_flash.h>

// CONFIGURABLE OPTIONS

#define KEYPAD_STORAGE_NS "keypad"
#define KEYPAD_PIN_MAX_LEN 10
#define KEYPAD_PIN_SUBMIT_KEY '#'
#define KEYPAD_PIN_CHANGE_KEY '*'


// EXPORTED SYMBOLS

void nvs_configure();
noreturn void keypad_handler_task();


#endif // IMP_TERM_KEYPAD_H
