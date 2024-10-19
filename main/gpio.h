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


// CONFIGURABLE OPTIONS

// GPIO port number definitions
#define STATUS_LED GPIO_NUM_2 // Onboard LED GPIO pin
#define SUCCESS_LED GPIO_NUM_4 // Door open LED GPIO pin

#define ESP_INTR_FLAG_DEFAULT 0


// CONVENIENCE DEFINITIONS

#define GPIO_LOW 0
#define GPIO_HIGH 1

#define E_KEYPAD_NO_KEY_FOUND ((uint8_t) -1)


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
 * @brief Blink a GPIO with a success pattern
 * @param gpio_num GPIO pin number
*/
void gpio_blink_success_nonblocking(const uint32_t gpio_num);

/*
 * @brief Configure GPIO pins
*/
void gpio_configure();

/*
 * @brief Lookup a key from a GPIO number
 * @param io_num GPIO number
 * @return Key value or E_KEYPAD_NO_KEY_FOUND if no key was found
*/
uint8_t gpio_keypad_key_lookup();

// Queue for GPIO events
extern QueueHandle_t gpio_evt_queue;


#endif // IMP_TERM_GPIO_H
