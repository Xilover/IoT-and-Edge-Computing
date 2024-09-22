#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_random.h"  // Required for esp_random()

// Define LED and Temperature Pin
#define LED_PIN 2
#define TEMP_SENSOR_PIN 15  // Simulating a temp sensor pin (DHT22 or similar)

// Queue Handle for temperature data
QueueHandle_t tempQueue;

// Simulated temperature reading function
int read_temperature() {
    return 25 + (esp_random() % 10);  // Return random temperature between 25-35
}

// LED Blinking Task (with temperature-based blink rate)
void blink_led_task(void *pvParameter) {
    int receivedTemp;
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    while(1) {
        // Check if there's new temperature data in the queue
        if (xQueueReceive(tempQueue, &receivedTemp, portMAX_DELAY)) {
            printf("LED Task received Temperature: %d°C\n", receivedTemp);

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

        // Send the temperature data to the queue
        xQueueSend(tempQueue, &temp, portMAX_DELAY);

        vTaskDelay(2000 / portTICK_PERIOD_MS);  // Send data every 2 seconds
    }
}

void app_main() {
    // Create a queue capable of holding 5 integers (temperatures)
    tempQueue = xQueueCreate(5, sizeof(int));

    if (tempQueue != NULL) {
        // Create the LED blink task
        xTaskCreate(&blink_led_task, "blink_led_task", 2048, NULL, 5, NULL);

        // Create the temperature task
        xTaskCreate(&temperature_task, "temperature_task", 2048, NULL, 5, NULL);
    } else {
        printf("Failed to create queue\n");
    }
}
