#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_random.h"  // Include this for esp_random()

// Define LED and Temperature Pin
#define LED_PIN 2
#define TEMP_SENSOR_PIN 15  // Simulating a temp sensor pin (DHT22 or similar)

// Simulated temperature reading function
int read_temperature() {
    // Simulate reading temperature
    return 25 + (esp_random() % 10);  // Return random temperature between 25-35
}

// LED Blinking Task
void blink_led_task(void *pvParameter) {
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    while(1) {
        gpio_set_level(LED_PIN, 1);  // LED ON
        vTaskDelay(1000 / portTICK_PERIOD_MS);  // 1 second delay
        gpio_set_level(LED_PIN, 0);  // LED OFF
        vTaskDelay(1000 / portTICK_PERIOD_MS);  // 1 second delay
    }
}

// Temperature Reading Task
void temperature_task(void *pvParameter) {
    while(1) {
        int temp = read_temperature();  // Simulate reading temperature
        printf("Current Temperature: %d°C\n", temp);
        vTaskDelay(2000 / portTICK_PERIOD_MS);  // Delay for 2 seconds
    }
}

void app_main() {
    // Create the LED blink task
    xTaskCreate(&blink_led_task, "blink_led_task", 2048, NULL, 5, NULL);

    // Create the temperature task
    xTaskCreate(&temperature_task, "temperature_task", 2048, NULL, 5, NULL);
}
