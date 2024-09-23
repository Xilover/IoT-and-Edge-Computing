#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"  // Include for software timers
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "esp_random.h"
#include <inttypes.h>  // For PRIu32

// Define LED and Button Pins
#define LED_PIN 2
#define BUTTON_PIN 15  // GPIO 15 connected to the button

// Mutex for LED access
SemaphoreHandle_t xMutex;

// Handle for the LED Task
TaskHandle_t ledTaskHandle;

// Timer handle
TimerHandle_t xTimer;

// Simulated temperature reading function
int read_temperature() {
    return 25 + (esp_random() % 10);  // Return random temperature between 25-35
}

// LED Blinking Task (with temperature-based blink rate and mutex protection)
void blink_led_task(void *pvParameter) {
    uint32_t receivedTemp;

    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    while(1) {
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

// Timer callback function to trigger temperature readings
void vTimerCallback(TimerHandle_t xTimer) {
    // Simulate reading temperature
    int temp = read_temperature();
    printf("Timer triggered: Reading temperature: %d°C\n", temp);

    // Send a notification to the LED task with the temperature value
    xTaskNotify(ledTaskHandle, temp, eSetValueWithOverwrite);
}

void app_main() {
    // Create mutex for LED control
    xMutex = xSemaphoreCreateMutex();

    if (xMutex != NULL) {
        // Create the LED blink task
        xTaskCreate(&blink_led_task, "blink_led_task", 2048, NULL, 5, &ledTaskHandle);

        // Create the software timer
        xTimer = xTimerCreate("TempTimer", pdMS_TO_TICKS(5000), pdTRUE, (void *)0, vTimerCallback);

        // Start the timer
        if (xTimer != NULL) {
            xTimerStart(xTimer, 0);
        }
    } else {
        printf("Failed to create mutex\n");
    }
}
