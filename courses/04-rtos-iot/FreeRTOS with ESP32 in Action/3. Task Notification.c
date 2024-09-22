#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_random.h"
#include <inttypes.h>  // For PRIu32 macro

// Define LED and Temperature Pin
#define LED_PIN 2
#define TEMP_SENSOR_PIN 15  // Simulating a temp sensor pin (DHT22 or similar)

// Handle for the LED Task
TaskHandle_t ledTaskHandle;

// Simulated temperature reading function
int read_temperature() {
    return 25 + (esp_random() % 10);  // Return random temperature between 25-35
}

// LED Blinking Task (with temperature-based blink rate)
void blink_led_task(void *pvParameter) {
    uint32_t receivedTemp;

    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    while(1) {
        // Wait for a notification from the temperature task
        if (xTaskNotifyWait(0, 0, &receivedTemp, portMAX_DELAY)) {
            printf("LED Task received Temperature: %" PRIu32 "°C\n", receivedTemp);

            // Adjust blink rate based on temperature
            int delay_ms = (40 - receivedTemp) * 100;  // Lower temp = faster blink
            printf("Blinking delay: %d ms\n", delay_ms);

            // Blink LED based on adjusted delay
            gpio_set_level(LED_PIN, 1);  // LED ON
            vTaskDelay(delay_ms / portTICK_PERIOD_MS);  // Delay
            gpio_set_level(LED_PIN, 0);  // LED OFF
            vTaskDelay(delay_ms / portTICK_PERIOD_MS);  // Delay
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

void app_main() {
    // Create the LED blink task
    xTaskCreate(&blink_led_task, "blink_led_task", 2048, NULL, 5, &ledTaskHandle);

    // Create the temperature task
    xTaskCreate(&temperature_task, "temperature_task", 2048, NULL, 5, NULL);
}
