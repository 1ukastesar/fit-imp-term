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