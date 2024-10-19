/*
 * @file main/gpio.h
 *
 * @proj imp-term
 * @brief GPIO related functions
 * @author Lukas Tesar <xtesar43@stud.fit.vut.cz>
 * @year 2024
*/

#ifndef IMP_TERM_GPIO_H
#define IMP_TERM_GPIO_H

#include <stdint.h>
#include <stdnoreturn.h>

#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>


// CONVENIENCE DEFINITIONS

#define GPIO_LOW 0
#define GPIO_HIGH 1

#define E_KEYPAD_NO_KEY_FOUND ((uint8_t) -1)

#define map_keypad_col_to_gpio_pin(col) gpio_keypad_pin_cols[col]
#define map_keypad_row_to_gpio_pin(row) gpio_keypad_pin_rows[row]


// EXPORTED SYMBOLS

/*
 * @brief Blink a GPIO pin (turn it on for a given duration)
 * @param gpio_num GPIO pin number
 * @param duration Duration in milliseconds
*/
void gpio_blink_blocking(const uint8_t gpio_num, const uint16_t duration);

/*
 * @brief Blink a GPIO pin (turn it on for a given duration) in a non-blocking manner
 * @param gpio_num GPIO pin number
 * @param duration Duration in milliseconds
*/
void gpio_blink_nonblocking(const uint8_t gpio_num, const uint16_t duration);

/*
 * @brief Blink a GPIO with two short blinks (blocking)
 * @param gpio_num GPIO pin number
*/
void gpio_blink_twice_blocking(const uint32_t gpio_num);

/*
 * @brief Blink a GPIO with two short blinks (non-blocking)
 * @param gpio_num GPIO pin number
*/
void gpio_blink_twice_nonblocking(const uint32_t gpio_num);

/*
 * @brief Configure GPIO pins
*/
void gpio_configure();

/*
 * @brief Lookup a key from a GPIO number
 * @param io_num GPIO number
 * @return Key value or E_KEYPAD_NO_KEY_FOUND if no key was found
*/
uint8_t gpio_keypad_key_lookup(uint32_t io_num);

// Queue for GPIO events
extern QueueHandle_t gpio_evt_queue;


#endif // IMP_TERM_GPIO_H
