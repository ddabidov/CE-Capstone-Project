#include <Arduino.h>
#include <RF24.h>
#include <nrf24L01.h>
// put function declarations here:
int myFunction(int, int);
RF24 radio = RF24(9, 10); // CE pin 9, CSN pin 10
int retries;

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
    radio.write(&msg, sizeof(msg)); // Write the message to the radio.
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
  }

  // Get the current number of retries for failed transmissions.
  int GetRetries() {
    return retries;
  }

  // Set the communication channel for the radio.
  void SetChannel(int channel) {
    radio.setChannel(channel);
  }

  // Get the current communication channel of the radio.
  int GetChannel() {
    return radio.getChannel();
  }

  // Set the data rate for communication.
  void SetDataRate(rf24_datarate_e rate) {
    radio.setDataRate(rate);
  }

  // Get the current data rate of the radio.
  uint8_t GetDataRate() {
    return radio.getDataRate();
  }

  // Set the power amplifier level for the radio.
  void SetPALevel(uint8_t level) {
    radio.setPALevel(level);
  }

  // Get the current power amplifier level of the radio.
  uint8_t GetPALevel() {
    return radio.getPALevel();
  }

  // Configure the writing pipe address for sending messages.
  void SetWritingPipe(uint64_t pipe) {
    radio.openWritingPipe(pipe);
  }

  // Configure the reading pipe address for receiving messages.
  void SetReadingPipe(uint64_t pipe) {
    radio.openReadingPipe(1, pipe);
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

  // Print detailed information about the radio's configuration.
  void PrintDetails() {
    radio.printDetails();
  }

  // Print the current communication channel to the Serial Monitor.
  void PrintChannel() {
    Serial.print("Channel: ");
    Serial.println(radio.getChannel());
  }

  // Print the current data rate to the Serial Monitor.
  void PrintDataRate() {
    Serial.print("Data Rate: ");
    Serial.println(radio.getDataRate());
  }
};

class BaseSpeak : public DeviceSpeak {
public:
  void Init() {
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
private:
  
};

class ControllerSpeak {
public:
  void Init() {
    if (!radio.begin()) {
      Serial.println("Radio failed to initialize!");
      return;
    }
    radio.setChannel(1);
    radio.setDataRate(RF24_250KBPS);
    radio.setPALevel(RF24_PA_HIGH);
    radio.openWritingPipe(0xF0F0F0F0D2LL);
    radio.openReadingPipe(1, 0xF0F0F0F0E1LL);
    radio.startListening();
  }
  void SendMessage(Message msg) {
    radio.stopListening();
    radio.write(&msg, sizeof(msg));
    radio.startListening();
  }
  bool ReceiveMessage(Message &msg) {
    if (radio.available()) {
      radio.read(&msg, sizeof(msg));
      return true;
    }
    return false;
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