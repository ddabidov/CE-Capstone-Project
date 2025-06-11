#pragma once
#include <Arduino.h>
#include "CommLib.h"

// Simple API tester for CommLib
class CommLibTester {
public:
    // Call this in setup()
    static void Begin() {
        Serial.begin(115200);
        while (!Serial) { delay(10); }
        Serial.println("CommLib API Tester Starting...");
        Station.transmission.id = 1;
        Station.Init();
        CommSetup();
    }

    // Call this in loop()
    static void Run() {
        static uint8_t base_id = 1; // For incrementing ID 1-4
        static uint8_t ctrl_id = 1;
        // Test sending and receiving for both base and controller
        if (isBase) {
            // Test BaseSpeak send
            Station.transmission.id = base_id;
            Station.transmission.command = ROUND_START;
            Station.transmission.data[0] = 1;
            Station.transmission.data[1] = 2;
            Station.transmission.data[2] = 3;
            Station.SendMessage(Station.transmission);
            Serial.println("Base: Sent test message.");

            // Test BaseSpeak receive (simulate controller message)
            if (Station.PollController(Station.reception)) {
                // Only print if id is valid
                if (Station.reception.id >= 1 && Station.reception.id <= 4) {
                    Serial.print("Base: Received Controller message, id: ");
                    Serial.println(Station.reception.id);
                    Serial.print("Command: ");
                    Serial.println(Station.reception.command);
                    Serial.print("Button: ");
                    Serial.println(Station.reception.button);
                } else {
                    Serial.println("Base: Received invalid or spurious message.");
                }
                // Clear reception after processing
                memset(&Station.reception, 0, sizeof(Station.reception));
            } else {
                Serial.println("Base: No controller message received.");
            }
            // Increment ID between 1 and 4
            base_id = (base_id % 4) + 1;
            delay(1000);
        } else {
            // Test ControllerSpeak send
            Controller.transmission.id = ctrl_id;
            Controller.transmission.command = BUTTON_PRESS;
            Controller.transmission.button = STAR;
            Controller.SendButtonPress(Controller.transmission.button);
            Serial.println("Controller: Sent button press.");

            // Test ControllerSpeak receive (simulate base message)
            if (Controller.ReceiveMessage(Controller.reception)) {
                Serial.print("Controller: Received Base message, id: ");
                Serial.println(Controller.reception.id);
                Serial.print("Command: ");
                Serial.println(Controller.reception.command);
                Serial.print("Data: ");
                Serial.print(Controller.reception.data[0]);
                Serial.print(", ");
                Serial.print(Controller.reception.data[1]);
                Serial.print(", ");
                Serial.println(Controller.reception.data[2]);
                // Clear reception after processing
                memset(&Controller.reception, 0, sizeof(Controller.reception));
            } else {
                Serial.println("Controller: No base message received.");
            }
            // Increment ID between 1 and 4
            ctrl_id = (ctrl_id % 4) + 1;
            delay(1000);
        }
    }
};

// PlatformIO/Arduino entry points
void setup() {
    CommLibTester::Begin();
}

void loop() {
    CommLibTester::Run();
}