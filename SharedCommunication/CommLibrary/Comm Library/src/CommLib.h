#pragma once
#include <Arduino.h>
#include "RF24.h"

// Pin definitions
#define CE_PIN 7
#define CSN_PIN 8

// Enum definitions
enum BaseCommandType {
  START_CONNECTION,
  ROUND_START,
  ROUND_WON,
  WRONG_BUTTON,
  ROUND_LOST,
  ROUND_END,
  SCORE_UPDATE,
  GAME_START,
  GAME_WON,
  GAME_LOST
};

enum ControllerCommandType {
  BUTTON_PRESS,
  LOW_BATTERY
};

enum ButtonType {
  STAR,
  SQUARE,
  TRIANGLE,
  HEXAGON
};

// Message structure for generic device
struct Message {
  uint8_t id;
  char command;
  uint8_t data[3];
};

// DeviceSpeak class
class DeviceSpeak {
public:
  int retries;
  uint8_t deviceID;
  Message transmission;

  void Init();
  void SendMessage(Message msg);
  bool ReceiveMessage(Message &msg);
  void SetRetries(int r);
  int GetRetries();
  void StartListening();
  void StopListening();
  bool Available();
  void Read(void *buf, size_t len);
  void Write(const void *buf, size_t len);
};

// BaseSpeak class
class BaseSpeak : public DeviceSpeak {
public:
  struct TxMessage {
    uint8_t id;
    BaseCommandType command;
    uint8_t data[3];
  };

  struct RxMessage {
    uint8_t id;
    ControllerCommandType command;
    ButtonType button;
  };

  TxMessage transmission;
  RxMessage reception;

  void Init();
  void SendMessage(TxMessage msg);
  bool ReceiveMessage(RxMessage &msg);
  bool PollController(RxMessage &msg);
};

// ControllerSpeak class
class ControllerSpeak : public DeviceSpeak {
public:
  struct RxMessage {
    uint8_t id;
    BaseCommandType command;
    uint8_t data[3];
  };

  struct TxMessage {
    uint8_t id;
    ControllerCommandType command;
    ButtonType button;
  };

  TxMessage transmission;
  RxMessage reception;

  void Init();
  void SendButtonPress(int buttonID);
};

// Global variables
extern uint8_t isBase;
extern BaseSpeak Station;
extern ControllerSpeak Controller;

// Function prototypes
void CommSetup();
void CommLoop();