//https://wokwi.com/projects/409708988359415809

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "esp_random.h"
#include <inttypes.h>  // For PRIu32

// Define LED and Button Pins
#define LED_PIN 2
#define BUTTON_PIN 15  // Simulating a button pin

// Mutex for LED access
SemaphoreHandle_t xMutex;

// Binary semaphore for signaling button press
SemaphoreHandle_t xButtonSemaphore;

// Handle for the LED Task
TaskHandle_t ledTaskHandle;

// Simulated temperature reading function
int read_temperature() {
    return 25 + (esp_random() % 10);  // Return random temperature between 25-35
}

// LED Blinking Task (with temperature-based blink rate and mutex protection)
void blink_led_task(void *pvParameter) {
    uint32_t receivedTemp;

    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    while(1) {
        // Wait for the semaphore signal (e.g., button press)
        if (xSemaphoreTake(xButtonSemaphore, portMAX_DELAY) == pdTRUE) {
            // Wait for a notification from the temperature task
            if (xTaskNotifyWait(0, 0, &receivedTemp, portMAX_DELAY)) {
                printf("LED Task received Temperature: %" PRIu32 "°C\n", receivedTemp);

                // Take the mutex before controlling the LED
                if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE) {
                    // Adjust blink rate based on temperature
                    int delay_ms = (40 - receivedTemp) * 100;  // Lower temp = faster blink
                    printf("Blinking delay: %d ms\n", delay_ms);

                    // Blink LED based on adjusted delay
                    gpio_set_level(LED_PIN, 1);  // LED ON
                    vTaskDelay(delay_ms / portTICK_PERIOD_MS);  // Delay
                    gpio_set_level(LED_PIN, 0);  // LED OFF
                    vTaskDelay(delay_ms / portTICK_PERIOD_MS);  // Delay

                    // Give the mutex back after using the LED
                    xSemaphoreGive(xMutex);
                }
            }
        }
    }
}

// Temperature Reading Task
void temperature_task(void *pvParameter) {
    while(1) {
        int temp = read_temperature();  // Simulate reading temperature
        printf("Temperature Task sending: %d°C\n", temp);

        // Send a notification to the LED task with the temperature value
        xTaskNotify(ledTaskHandle, temp, eSetValueWithOverwrite);

        vTaskDelay(2000 / portTICK_PERIOD_MS);  // Delay before next temperature read
    }
}

// Button Task to monitor button presses
void button_task(void *pvParameter) {
    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_PIN, GPIO_PULLUP_ONLY);  // Enable internal pull-up

    while(1) {
        // Check if the button is pressed (GPIO goes LOW when pressed)
        if (gpio_get_level(BUTTON_PIN) == 0) {
            printf("Button pressed! Signaling LED task...\n");

            // Signal the LED task to start blinking
            xSemaphoreGive(xButtonSemaphore);

            // Debounce delay (to avoid multiple triggers)
            vTaskDelay(500 / portTICK_PERIOD_MS);
        }

        // Small delay to prevent button bounce issues
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}


void app_main() {
    // Create mutex for LED control
    xMutex = xSemaphoreCreateMutex();

    // Create binary semaphore for button press simulation
    xButtonSemaphore = xSemaphoreCreateBinary();

    if (xMutex != NULL && xButtonSemaphore != NULL) {
        // Create the LED blink task
        xTaskCreate(&blink_led_task, "blink_led_task", 2048, NULL, 5, &ledTaskHandle);

        // Create the temperature task
        xTaskCreate(&temperature_task, "temperature_task", 2048, NULL, 5, NULL);

        // Create the button task to simulate button press
        xTaskCreate(&button_task, "button_task", 2048, NULL, 5, NULL);
    } else {
        printf("Failed to create mutex or semaphore\n");
    }
}
