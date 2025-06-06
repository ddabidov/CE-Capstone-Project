#define CE_PIN 20
#define CSN_PIN PICO_DEFAULT_SPI_CSN_PIN

#include <Arduino.h>
#include "RF24.h"
#include "CommLib.h"


// Instantiate an object for the nRF24L01 transceiver
RF24 radio(CE_PIN, CSN_PIN);

// DeviceSpeak method implementations
void DeviceSpeak::Init() {
    if (!radio.begin()) {
        Serial.println("Radio failed to initialize!");
        return;
    }
    radio.setChannel(1);
    radio.setDataRate(RF24_250KBPS);
    radio.setPALevel(RF24_PA_HIGH);
    radio.openWritingPipe(0xF0F0F0F0E1LL);
    radio.openReadingPipe(1, 0xF0F0F0F0D2LL);
    radio.startListening();
}

void DeviceSpeak::SendMessage(Message msg) {
    radio.stopListening();
    bool success = radio.write(&msg, sizeof(msg));
    if (success) {
        Serial.println("Message sent successfully!");
    } else {
        Serial.println("Message failed to send!");
    }
    radio.startListening();
}

bool DeviceSpeak::ReceiveMessage(Message &msg) {
    if (radio.available()) {
        radio.read(&msg, sizeof(msg));
        return true;
    }
    return false;
}

void DeviceSpeak::SetRetries(int r) {
    retries = r;
    radio.setRetries(retries, 15);
}

int DeviceSpeak::GetRetries() {
    return retries;
}

void DeviceSpeak::StartListening() {
    radio.startListening();
}

void DeviceSpeak::StopListening() {
    radio.stopListening();
}

bool DeviceSpeak::Available() {
    return radio.available();
}

void DeviceSpeak::Read(void *buf, size_t len) {
    radio.read(buf, len);
}

void DeviceSpeak::Write(const void *buf, size_t len) {
    radio.write(buf, len);
}

// BaseSpeak method implementations
void BaseSpeak::Init() {
    while (!Serial) {
    // some boards need to wait to ensure access to serial over USB
   }
    if (!radio.begin()) {
        Serial.println("Radio failed to initialize!");
        return;
    }
    radio.powerDown();
    delay(10);
    radio.powerUp();
    radio.setChannel(1);
    radio.setDataRate(RF24_250KBPS);
    radio.setPALevel(RF24_PA_HIGH);
    // Use a single pipe for all controllers
    radio.openReadingPipe(1, 0xF0F0F0F0C1LL);
    radio.openWritingPipe(0xF0F0F0F0BBLL);
    radio.flush_rx(); // Flush RX buffer at init
    radio.startListening();
    radio.flush_rx(); // Flush again after listening
}

void BaseSpeak::SendMessage(TxMessage msg) {
    radio.stopListening();
    if (radio.write(&msg, sizeof(msg))) {
        Serial.println("Message sent successfully!");
    } else {
        Serial.println("Message failed to send!");
    }
    radio.startListening();
}

bool BaseSpeak::ReceiveMessage(RxMessage &msg) {
    if (radio.available()) {
        radio.read(&msg, sizeof(msg));
        radio.flush_rx(); // Clear RX buffer after reading
        // Only treat as valid if id is in 1-4 range
        if (msg.id >= 1 && msg.id <= 4) {
            return true;
        }
    }
    // Optionally, check RX FIFO status
    if (radio.available()) {
        Serial.println("Warning: RX FIFO still not empty after read+flush!");
    }
    return false;
}

bool BaseSpeak::PollController(RxMessage &msg) {
    if (Available()) {
        bool got = ReceiveMessage(msg);
        return got;
    }
    return false;
}

// ControllerSpeak method implementations
void ControllerSpeak::Init() {
    if (!radio.begin()) {
        Serial.println("Radio failed to initialize!");
        return;
    }
    radio.powerDown();
    delay(10);
    radio.powerUp();
    radio.setChannel(1);
    radio.setDataRate(RF24_250KBPS);
    radio.setPALevel(RF24_PA_HIGH);
    // Always use the same pipe regardless of ID
    radio.openWritingPipe(0xF0F0F0F0C1LL);
    radio.openReadingPipe(1, 0xF0F0F0F0BBLL);
    radio.flush_rx(); // Flush RX buffer at init
    radio.startListening();
    radio.flush_rx(); // Flush again after listening
}

void ControllerSpeak::SendButtonPress(int buttonID) {
    transmission.id = buttonID;
    transmission.command = BUTTON_PRESS;
    transmission.button = static_cast<ButtonType>(buttonID);
    radio.stopListening();
    radio.write(&transmission, sizeof(transmission));
    radio.startListening();
}

bool ControllerSpeak::ReceiveMessage(RxMessage &msg) {
    if (radio.available()) {
        radio.read(&msg, sizeof(msg));
        radio.flush_rx();
        if (msg.id >= 1 && msg.id <= 4) {
            return true;
        }
    }
    if (radio.available()) {
        Serial.println("Warning: RX FIFO still not empty after read+flush!");
    }
    return false;
}

// Global variables
uint8_t isBase = 0; // 0 for Controller, 1 for Base
BaseSpeak Station;
ControllerSpeak Controller;

// Communication setup and loop
void CommSetup() {
    pinMode(25, OUTPUT);
    digitalWrite(25, LOW);
    switch (isBase) {
        case 0:
            Controller.transmission.id = 1;
            Controller.Init();
            break;
        case 1:
            Station.Init();
            break;
        default:
            break;
    }
}

void CommLoop() {
    digitalWrite(25, LOW);
    switch (isBase) {
        case 0:
            if (Controller.Available()) {
                if (Controller.ReceiveMessage(Controller.reception)) {
                    Serial.print("Received message ID: ");
                    Serial.println(Controller.reception.id);
                    digitalWrite(25, HIGH);
                    delay(100);
                    digitalWrite(25, LOW);
                }
            } else {
                Serial.println("No message available.");
            }
            break;
        case 1:
            Station.SendMessage(Station.transmission);
            Serial.println("Message sent!");
            digitalWrite(25, HIGH);
            delay(100);
            digitalWrite(25, LOW);
            delay(1000);
            break;
        default:
            break;
    }
}

