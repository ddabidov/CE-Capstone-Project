#include <Arduino.h>
#include "RF24.h"
// put function declarations here:
int myFunction(int, int);
#define CE_PIN 7
#define CSN_PIN 8
// instantiate an object for the nRF24L01 transceiver
RF24 radio(CE_PIN, CSN_PIN);

// Define a structure to represent a message that will be sent or received via the NRF24 module.
struct Message {
  int id;          // An integer identifier for the message.
  char command;    // A single character representing the command or action.
};

// Define a class to encapsulate communication functionality for a device using the NRF24 module.
class DeviceSpeak {
public:
  // Initialize the NRF24 module with default settings.
  void Init() {
    // Attempt to initialize the radio. If it fails, print an error message.
    if (!radio.begin()) {
      Serial.println("Radio failed to initialize!");
      return;
    }
    // Set the communication channel for the radio (1 in this case).
    radio.setChannel(1);
    // Set the data rate for communication (250 kbps in this case).
    radio.setDataRate(RF24_250KBPS);
    // Set the power amplifier level to high for better range.
    radio.setPALevel(RF24_PA_HIGH);
    // Configure the writing pipe address (used for sending messages).
    radio.openWritingPipe(0xF0F0F0F0E1LL);
    // Configure the reading pipe address (used for receiving messages).
    radio.openReadingPipe(1, 0xF0F0F0F0D2LL);
    // Start listening for incoming messages.
    radio.startListening();
  }

  // Send a message to the configured writing pipe.
  void SendMessage(Message msg) {
    radio.stopListening(); // Stop listening to prepare for sending.

    bool success = radio.write(&msg, sizeof(msg));
    if (success) {
      Serial.println("Message sent successfully!");
    } else {
      Serial.println("Message failed to send!");
    } // Write the message to the radio.

    radio.startListening(); // Resume listening after sending.
  }

  // Check if a message is available and read it into the provided Message object.
  bool ReceiveMessage(Message &msg) {
    if (radio.available()) { // Check if data is available to read.
      radio.read(&msg, sizeof(msg)); // Read the data into the provided message object.
      return true; // Indicate that a message was successfully received.
    }
    return false; // No message was available.
  }

  // Set the number of retries for failed transmissions.
  void SetRetries(int r) {
    retries = r;
    radio.setRetries(retries, 15); // Set the number of retries and delay between retries.
  }

  // Get the current number of retries for failed transmissions.
  int GetRetries() {
    return retries;
  }

  // Start listening for incoming messages.
  void StartListening() {
    radio.startListening();
  }

  // Stop listening for incoming messages.
  void StopListening() {
    radio.stopListening();
  }

  // Check if there is data available to read.
  bool Available() {
    return radio.available();
  }

  // Read data from the radio into the provided buffer.
  void Read(void *buf, size_t len) {
    radio.read(buf, len);
  }

  // Write data to the radio from the provided buffer.
  void Write(const void *buf, size_t len) {
    radio.write(buf, len);
  }

  private:
    int retries; // Number of retries for failed transmissions.
    uint8_t deviceID; // Communication channel for the radio.
    Message msg;

    
};

class BaseSpeak : public DeviceSpeak {
public:
  void Init() {
    // Attempt to initialize the radio. If it fails, print an error message.
    if (!radio.begin()) {
      Serial.println("Radio failed to initialize!");
      return;
    }
    // Set the communication channel for the radio (1 in this case).
    radio.setChannel(1);
    // Set the data rate for communication (250 kbps in this case).
    radio.setDataRate(RF24_250KBPS);
    // Set the power amplifier level to high for better range.
    radio.setPALevel(RF24_PA_HIGH);
    // Configure the writing pipe address (used for sending messages).
    radio.openReadingPipe(1, 0xF0F0F0F0C1LL);
    radio.openReadingPipe(2, 0xF0F0F0F0C2LL);
    radio.openReadingPipe(3, 0xF0F0F0F0C3LL);
    radio.openReadingPipe(4, 0xF0F0F0F0C4LL);
    // Configure the reading pipe address (used for receiving messages).
    radio.openWritingPipe(0xF0F0F0F0D2LL);
    // Start listening for incoming messages.
    radio.startListening();
  }
private:
  
};

class ControllerSpeak {
public:
 void Init() {
    // Attempt to initialize the radio. If it fails, print an error message.
    if (!radio.begin()) {
      Serial.println("Radio failed to initialize!");
      return;
    }
    // Set the communication channel for the radio (1 in this case).
    radio.setChannel(1);
    // Set the data rate for communication (250 kbps in this case).
    radio.setDataRate(RF24_250KBPS);
    // Set the power amplifier level to high for better range.
    radio.setPALevel(RF24_PA_HIGH);
    // Configure the writing pipe address (used for sending messages).
    switch (ControllerSpeak.deviceID) {
    case 1:
      radio.openWritingPipe(0xF0F0F0F0C1LL);
      break;
    case 2:
      radio.openWritingPipe(0xF0F0F0F0C2LL);
      break;
    case 3:
      radio.openWritingPipe(0xF0F0F0F0C3LL);
      break;
    case 4:
      radio.openWritingPipe(0xF0F0F0F0C4LL);
      break;
    default:
      radio.openWritingPipe(0xF0F0F0F0EELL); // Default to device EE for debugging if device loads uninitialized.
      break;
    }
    // Configure the reading pipe address (used for receiving messages).
    radio.openReadingPipe(1, 0xF0F0F0F0BBLL);
    // Start listening for incoming messages.
    radio.startListening();
  }
   
  void SendButtonPress(int buttonID) {
    ControllerSpeak.msg.id = buttonID; // Set the message ID to the button ID.
    ControllerSpeak.msg.command = 'P'; // Set the command to 'P' for button press.
    radio.stopListening(); // Stop listening to prepare for sending.
    radio.write(&ControllerSpeak.msg, sizeof(ControllerSpeak.msg)); // Write the message to the radio.
    radio.startListening(); // Resume listening after sending.
  }
};

void setup() {
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Hello, Serial Monitor!");
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}