/*
 * Copyright (c) 2022, CATIE
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "bme280.h"

using namespace sixtron;
Mutex stdio_mutex;

// Instantiate I2C and sensor
I2C i2c1(I2C1_SDA, I2C1_SCL);          // Use the correct pins for your board
BME280 sensor(&i2c1);                  // Pass I2C instance to the BME280 sensor
DigitalOut led1(LED1, 0);    
InterruptIn button1(BUTTON1);

// Declare EventQueue
EventQueue queue;

// Loop for printing temperature
void Temperature() {
    while (true) {
        stdio_mutex.lock();
        // Get temperature reading
        float temp = sensor.temperature();  
        printf("Current temperature: %.2f°C\n", temp);
        stdio_mutex.unlock();  
        ThisThread::sleep_for(2000ms); // Wait for 2 seconds
    }
}

void Pressure(){
    stdio_mutex.lock();
    float pressure = sensor.pressure();  
    printf("Current pressure: %.2f hPa\n", pressure);
    stdio_mutex.unlock();
}

// Loop for blinking the LED
void Blinker() {
    while (true) {
        led1 = !led1; // Toggle the LED state
        ThisThread::sleep_for(5000ms); // Blink every 0.5 seconds
    }
}

int main() {
    Thread temperatureThread;
    Thread ledThread;
    Thread queueThread;

    // Initialize sensor
    if (!sensor.initialize()) {
        printf("Failed to initialize BME280 sensor.\n");
        return -1; // Exit if initialization fails
    }
    // Set power mode to NORMAL
    sensor.set_sampling();


    button1.fall(queue.event(Pressure));

    // Start threads
    temperatureThread.start(Temperature);
    ledThread.start(Blinker);

    // Start the EventQueue in its own thread
    queueThread.start(callback(&queue, &EventQueue::dispatch_forever));

    // Main thread does nothing, threads handle the tasks
    while (true) {
        ThisThread::sleep_for(1s); // Prevent main thread from exiting
    }
}
