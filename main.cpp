/*
 * Copyright (c) 2020, CATIE
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mbed.h"
#include "bme280.h"
#include "AccessCode.h"
#include <cstdio>
#include <nsapi_dns.h>
#include <MQTTClientMbedOs.h>

using namespace sixtron;

namespace {
#define USERNAME            "Bodhix20"
#define GROUP_NAME          "Station_Meteo"
#define MQTT_TOPIC_PUBLISH      USERNAME "/groups/" GROUP_NAME "/json"
#define MQTT_TOPIC_SUBSCRIBE    USERNAME "/feeds/station-meteo.alert"
#define SYNC_INTERVAL           1
#define MQTT_CLIENT_ID          "6LoWPAN_Node_" GROUP_NAME
}

// Peripherals
static DigitalOut led(LED1,1);
static InterruptIn button(BUTTON1);

// Instantiate I2C and sensor
I2C i2c1(I2C1_SDA, I2C1_SCL);          // Use the correct pins for your board
BME280 sensor(&i2c1);                  // Pass I2C instance to the BME280 sensor

Mutex stdio_mutex;

// Network
NetworkInterface *network;
MQTTClient *client;

// MQTT
const char* hostname = "io.adafruit.com";
int port = 1883;
bool streaming_enabled = true;
// Error code
nsapi_size_or_error_t rc = 0;

// Event queue
static int id_yield;
static EventQueue main_queue(32 * EVENTS_EVENT_SIZE);

/*!
 *  \brief Called when a message is received
 *
 *  Print messages received on mqtt topic
 */
void messageArrived(MQTT::MessageData& md)
{
    MQTT::Message &message = md.message;
    printf("Message arrived: qos %d, retained %d, dup %d, packetid %d\r\n", message.qos, message.retained, message.dup, message.id);
    printf("Payload %.*s\r\n", message.payloadlen, (char*)message.payload);

    // Get the payload string
    char* char_payload = (char*)malloc((message.payloadlen+1)*sizeof(char)); // allocate the necessary size for our buffer
    char_payload = (char *) message.payload; // get the arrived payload in our buffer
    char_payload[message.payloadlen] = '\0'; // String must be null terminated

    // Compare our payload with known command strings
    if (strcmp(char_payload, ":(") == 0) {
        led = 1;
        printf("Que calor\n");
    }
    else {
        led = 0;
        printf("Il fait bon :)\n");
    }
}

/*!
 *  \brief Yield to the MQTT client
 *
 *  On error, stop publishing and yielding
 */
static void yield(){
    // printf("Yield\n");
    
    rc = client->yield(100);

    if (rc != 0){
        printf("Yield error: %d\n", rc);
        main_queue.cancel(id_yield);
        main_queue.break_dispatch();
        system_reset();
    }
}


/*!
 *  \brief Publish JSON data over the corresponding Adafruit MQTT topic
 *
 *  \param jsonPayload JSON-formatted string to publish
 *  \return 0 on success, or a non-zero error code
 */
static int8_t publishJson(const char* jsonPayload) {
    // Create an MQTT message
    MQTT::Message message;
    message.qos = MQTT::QOS1;
    message.retained = false;
    message.dup = false;
    message.payload = (void*)jsonPayload;
    message.payloadlen = strlen(jsonPayload);

    printf("Send JSON: %s to MQTT Broker: %s at thread %s\n", jsonPayload, hostname,MQTT_TOPIC_PUBLISH);
    int rc = client->publish(MQTT_TOPIC_PUBLISH, message);
    if (rc != 0) {
        printf("Failed to publish: %d\n", rc);
        return rc;
    }
    return 0;
}

// Function to toggle the streaming flag when the button is pressed
void toggleStreaming() {
    streaming_enabled = !streaming_enabled;  // Toggle the flag
    if (streaming_enabled) {
        printf("Data streaming enabled.\n");
    } else {
        printf("Data streaming disabled.\n");
    }
}

//Function that updates the sends the current temperature to the MQTT
void UpdateData() {
    if(streaming_enabled){
        char jsonMessage[256];
        float temperature = sensor.temperature();  
        float humidity = sensor.humidity();      
        float pressure = sensor.pressure()/100; 

        // Format the JSON payload
        snprintf(jsonMessage, sizeof(jsonMessage),"{ \"feeds\": { \"Temperature\": \"%.2f\", \"Humidity\": \"%.2f\", \"Pressure\": \"%.2f\" } }",temperature, humidity, pressure);


        // Publish the JSON payload
        if (publishJson(jsonMessage) != 0) {
            printf("Failed to publish message.\n");
        }
    }
}


// main() runs in its own thread in the OS
// (note the calls to ThisThread::sleep_for below for delays)

int main()
{
    // Initialize sensor
    if (!sensor.initialize()) {
        printf("Failed to initialize BME280 sensor.\n");
        return -1; // Exit if initialization fails
    }
    // Set power mode to NORMAL
    sensor.set_sampling();


    printf("Connecting to border router...\n");

    /* Get Network configuration */
    network = NetworkInterface::get_default_instance();

    if (!network) {
        printf("Error! No network interface found.\n");
        return 0;
    }

    /* Add DNS */
    nsapi_addr_t new_dns = {
        NSAPI_IPv6,
        { 0xfd, 0x9f, 0x59, 0x0a, 0xb1, 0x58, 0, 0, 0, 0, 0, 0, 0, 0, 0x00, 0x01 }
    };
    nsapi_dns_add_server(new_dns, "LOWPAN");

    /* Border Router connection */
    rc = network->connect();
    if (rc != 0) {
        printf("Error! net->connect() returned: %d\n", rc);
        return rc;
    }

    /* Print IP address */
    SocketAddress a;
    network->get_ip_address(&a);
    printf("IP address: %s\n", a.get_ip_address() ? a.get_ip_address() : "None");

    /* Open TCP Socket */
    TCPSocket socket;
    SocketAddress address;
    network->gethostbyname(hostname, &address);
    address.set_port(port);

    /* MQTT Connection */
    client = new MQTTClient(&socket);
    socket.open(network);
    rc = socket.connect(address);
    if(rc != 0){
        printf("Connection to MQTT broker Failed\n");
        return rc;
    }

    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 4;
    data.keepAliveInterval = 25;
    //data.clientID.cstring = MQTT_CLIENT_ID;
    data.username.cstring = USERNAME ;
    data.password.cstring = ADAFRUIT_KEY ;

    if (client->connect(data) != 0){
        printf("Connection to MQTT Broker Failed\n");
    }else{
        printf("Connected to MQTT broker\n");
    }


    /* MQTT Subscribe 
    */
    if ((rc = client->subscribe(MQTT_TOPIC_SUBSCRIBE, MQTT::QOS0, messageArrived)) != 0){
        printf("rc from MQTT subscribe is %d\r\n", rc);
    }
    printf("Subscribed to Topic: %s\n", MQTT_TOPIC_SUBSCRIBE);

    yield();

    // Yield every 1 second
    id_yield = main_queue.call_every(SYNC_INTERVAL * 1000, yield);

    Ticker dataTicker;
    dataTicker.attach(main_queue.event(UpdateData),5s);

    button.fall(main_queue.event(toggleStreaming)); // When button is pressed, toggle streaming

    

    main_queue.dispatch_forever();
}
 