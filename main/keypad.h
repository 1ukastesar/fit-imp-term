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


// EXPORTED SYMBOLS

void nvs_configure();
void keypad_keypress_handler(char key_pressed);


#endif // IMP_TERM_KEYPAD_H
