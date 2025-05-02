#include <Arduino.h>
#include <RF24.h>
#include <nrf24L01.h>
// put function declarations here:
int myFunction(int, int);
RF24 radio = RF24(9, 10); // CE pin 9, CSN pin 10
int retries;

struct Message {
  int id;
  char command;
};

class DeviceSpeak {
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
  void SetRetries(int r) {
    retries = r;
  }
  int GetRetries() {
    return retries;
  }
  void SetChannel(int channel) {
    radio.setChannel(channel);
  }
  int GetChannel() {
    return radio.getChannel();
  }
  void SetDataRate(rf24_datarate_e rate) {
    radio.setDataRate(rate);
  }
  uint8_t GetDataRate() {
    return radio.getDataRate();
  } 
  void SetPALevel(uint8_t level) {
    radio.setPALevel(level);
  }
  uint8_t GetPALevel() {
    return radio.getPALevel();
  }
  void SetWritingPipe(uint64_t pipe) {
    radio.openWritingPipe(pipe);
  }
  void SetReadingPipe(uint64_t pipe) {
    radio.openReadingPipe(1, pipe);
  }
  void StartListening() {
    radio.startListening();
  }
  void StopListening() {
    radio.stopListening();
  }
  bool Available() {
    return radio.available();
  }
  void Read(void *buf, size_t len) {
    radio.read(buf, len);
  }
  void Write(const void *buf, size_t len) {
    radio.write(buf, len);
  }
  void PrintDetails() {
    radio.printDetails();
  }
  void PrintChannel() {
    Serial.print("Channel: ");
    Serial.println(radio.getChannel());
  }
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