#include <Arduino.h>
#include <string.h>
#include <Adafruit_NeoPixel.h>
#include <CommLib.h>

//constants
#define RING_LED_PIN  3
#define SHAPE_LED_PIN  15
#define RING_LED_COUNT 60
#define SHAPE_LED_COUNT 30
#define MAX_PLAYERS 4

// #define Serial _UART2_;
//button ports 11 12 13

UART SerialA(4, 5);
UART SerialB(8,9);

//Essential objects
Adafruit_NeoPixel RING_LEDS(RING_LED_COUNT, RING_LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel SHAPE_LEDS(SHAPE_LED_COUNT, SHAPE_LED_PIN, NEO_GRB + NEO_KHZ800);
BaseSpeak COMS_CONTROLLER; 


//state machine variables ****************************************************************************
int STATE = 0;

enum Colors {
  WHITE = 0xFFFFFF,
  OFF = 0,
  RED = 0xFF0000,
  GREEN = 0xFF00,
  BLUE = 0x1664b1,
  PINK = 0xFF1111,
  ORANGE = 0xff4400,
  YELLOW = 0xFFFF00
};

typedef struct LEDranges {
  int min;
  int max;
} LEDranges;

typedef struct RoundLedInfo {
  int locationID;
  ButtonType button;
  int colorID;
  uint32_t color;
  bool enabled; 
} RoundLedInfo;

class Player
{
public:
  int playerId; 
  uint32_t playerColor;
  int playerColorID;
  int playerScore; 
  bool isConnected = false; 

  Player(){}
  Player(int id, uint32_t color){
    this->playerId = id; 
    this->playerColor = color;
    this->playerScore = 0; 
  }
  void incScore(){
    this->playerScore = this->playerColor + 1;
  }
  uint32_t getColor() {
    return this->playerColor;
  }
  void connectPlayerFromMessage(Message r){
    playerId = r.id;
    playerColor = r.data[0];
    playerScore = 0; 
  }
};

class LED {
  public:
    int borderLEDs[4] = {0, 15, 30, 45};
    LEDranges ringQuarterRanges[4] = { {1, 14}, {16, 29}, {31, 44}, {46, 60}};
    LEDranges shapeRanges[4] = {{0, 6}, {7, 13}, {14, 20}, {21, 27}};
    uint32_t LEDcolors[4] = {PINK, YELLOW, ORANGE, BLUE};

  public: 
    LED (){
      // turnOnBorders();
    }

    int getColorID(uint32_t color){
      for(int i = 0; i < 4; i++){
        if(color == LEDcolors[i]){
          return i;
        }
      }
      return -1; 
    }
    void turnOnBorders(uint32_t color = 0xFFFFFF){
      for(int ledId: borderLEDs){
        RING_LEDS.setPixelColor(ledId, color);
      }
      RING_LEDS.show();
    }

  void turnOnShape(int shapeID, uint32_t color){
    LEDranges shapeRange = shapeRanges[shapeID]; 
    for (int i = shapeRange.min; i <= shapeRange.max; i++)
    {
      SHAPE_LEDS.setPixelColor(i, color);
    }
    SHAPE_LEDS.show();
  }

   void turnOnShape(int shapeID, int colorID){
      this->turnOnShape(shapeID, LEDcolors[colorID]);
    }

  void turnOffShape(int shapeID){
    LEDranges shapeRange = shapeRanges[shapeID]; 
    for (int i = shapeRange.min; i <= shapeRange.max; i++)
    {
      SHAPE_LEDS.setPixelColor(i, 0x0);
    }
    SHAPE_LEDS.show();
  }
   void turnOnQuarter(int quarterID, uint32_t color){
    LEDranges quarterRange = ringQuarterRanges[quarterID]; 
    for (int i = quarterRange.min; i <= quarterRange.max; i++)
    {
      RING_LEDS.setPixelColor(i, color);
    }
    RING_LEDS.show();
    // turnOnBorders();
   }
   void turnOnQuarter(int quarterID, int colorID){
    this->turnOnQuarter(quarterID, LEDcolors[colorID]);
   }

   void turnOffQuarter(int quarterID){
    LEDranges quarterRange = ringQuarterRanges[quarterID];
    for (int i = quarterRange.min; i <= quarterRange.max; i++)
    {
      RING_LEDS.setPixelColor(i, 0x0);
    }
    RING_LEDS.show();
   }
   
   void startBlink() {
    uint32_t color = 0x00FF00;
    static uint16_t current_pixel = 0;

    bool found = false; 
    RING_LEDS.clear();

    for(int i = 0; i < RING_LED_COUNT; i ++){
      for(int x = 0; x < 4; x++){
        if(i >= ringQuarterRanges[x].min && i <= ringQuarterRanges[x].max){
          RING_LEDS.setPixelColor(i, LEDcolors[x]);
          found = true;
          break;
        }
        if(!found){
          RING_LEDS.setPixelColor(i, 0xFFFFFF);
        }
        found = false; 
      }
      // ring.setPixelColor(i, color);
      RING_LEDS.show();
      delay(100);
    }

    for(int i = 0; i < 3; i++){
      RING_LEDS.clear();
      RING_LEDS.show();
      delay(200);

      for(int c=current_pixel; c < RING_LED_COUNT; c ++) {
        RING_LEDS.setPixelColor(c, color);
      }
      RING_LEDS.show();
      delay(200);
    }


    RING_LEDS.clear();
    RING_LEDS.show();

  }
  
  
};

class GameRound {
  public:
    unsigned long duration; 

    unsigned long startTime; 

    RoundLedInfo leds[4];

    int numOnChanceProportions[4] = {3, 3, 3, 1};

    GameRound () {
      leds[0].button = SQUARE;
      leds[0].locationID = 0;
      leds[1].button = STAR;
      leds[1].locationID = 1;
      leds[2].button = HEXAGON;
      leds[2].locationID = 2;
      leds[3].button = TRIANGLE;
      leds[3].locationID = 3;
    }

    void setUpNewRound() {
      selectIDs();
      startTime = millis();
      duration = 2000;
    }
    RoundLedInfo* getRoundLedInfoByColor(uint32_t color){
      for(int i = 0; i < 4; i ++){
        if (leds[i].color == color){
          return &leds[i];
        }
      }
      return NULL;
    }
    RoundLedInfo* getRoundLedInfoByShape(ButtonType shape){
      for(int i = 0; i < 4; i ++){
        if (leds[i].button == shape){
          return &leds[i];
        }
      }
      return NULL;
    }
    // int selectNumLedsOn(){
    //   srand(millis());
    //   int chanceMax = chanceSum(3);
    //   int chance = random(1, chanceMax);
    //   int numOn; 
    //   if(chance > chanceSum(2)){
    //     numOn = 1;
    //   } else if (chance > chanceSum(1)){
    //     numOn = 2;
    //   } else if (chance > numOnChanceProportions[0]){
    //     numOn = 3;
    //   } else {
    //     numOn = 4; 
    //   }

    //   return numOn;
    // }
    // int chanceSum(int upToID){
    //   int sum = 0;
    //   for(int x = 0; x <= upToID; x++){
    //     sum = sum + x; 
    //   }
    //   return sum;
    // }
    void resetLedInfo(int startIndex = 0){
      for( int x = startIndex; x < 4; x++ ){
        leds[x].color = 0x0; 
        leds[x].colorID = -1; 
        leds[x].enabled = false; 
      }
    }
    void selectIDs(){
      srand(millis());
      uint32_t availableIDs[4] = {PINK,BLUE,ORANGE,YELLOW};
      int numAvailable = 4; 

      for(int i = 0; i < 3; i++){
        int selectedIndex = random(0, numAvailable - 1);
        
        leds[i].color = availableIDs[selectedIndex]; 
        leds[i].enabled = true; 

        int lastIndex = numAvailable - 1;
        if(selectedIndex != numAvailable - 1){
          int temp = availableIDs[lastIndex];
          availableIDs[lastIndex] = availableIDs[selectedIndex];
          availableIDs[selectedIndex] = temp;
        }
      numAvailable--;
    }
    leds[3].color = availableIDs[0];
    leds[3].enabled = true; 
  }
  RoundLedInfo* getRoundLedInfo(int index){
    return &leds[index];
  }
      
  void updateRoundTimer(){
    RING_LEDS.clear();
    int startID = 0;
    int currentID = 1;
    int maxID = 60;
    unsigned long startTime = millis();
    while(startID < maxID){

      if(currentID >= maxID){
        RING_LEDS.clear();
          startID++;
          currentID = startID + 1;
          startTime = millis();
          for(uint16_t i = 0; i < startID; i++){
            RING_LEDS.setPixelColor(i, 0xFFFFFF);
          }
          // ring.show();
        }

        if(millis() >= startTime + (10 * (currentID - startID))){
          RING_LEDS.clear();

          for(uint16_t i = 0; i < currentID; i++){
            if(i < startID){
              RING_LEDS.setPixelColor(i, 0x0000FF);
            } else {
              RING_LEDS.setPixelColor(i, 0xFFFFFF);
            }
          }
          currentID++;
        }

        RING_LEDS.show();

    }

}
};

//prototype variables ***********************************************************************************
bool connected[4] = {false, false, false, false};
int buttonPorts[4] = {11, 12, 13, 14};
int startButtonPort = 10;
long buttonPressDuration[4] = { -1, -1, -1, -1}; 


bool isButtonPressed(int buttonID);
Player* findPlayerIndexById(int playerId);
Player* findPlayerIndexByColor(uint32_t color);
void sendToSingleController(int playerIndex, BaseCommandType command, int data1 = 0, int data2 = 0, int data3 = 0);
void sendToAllConnected(BaseCommandType command, int data1 = 0, int data2 = 0, int data3 = 0);
int readUart(int controllerId);
//*****************************************************************************************************

int roundNum = 0; 

LED LedController;
GameRound currentRound; 

Player player1(1, PINK);
Player player2(2, YELLOW);
Player player3(3, ORANGE);
Player player4(4, BLUE);

Player playersList[4] = {player1, player2, player3, player4};
int numPlayers = 0; 


  struct TxMessage {
    uint8_t id;
    int command;
    uint8_t data[3];
  };

  struct RxMessage {
    uint8_t id;
    ControllerCommandType command;
    ButtonType button;
  };


void setup() {
  // put your setup code here, to run once:
  pinMode(buttonPorts[0], INPUT); // Sets the pin as an output
  pinMode(buttonPorts[1], INPUT); // Sets the pin as an output
  pinMode(buttonPorts[2], INPUT); // Sets the pin as an output
  pinMode(buttonPorts[3], INPUT); // Sets the pin as an output
  pinMode(startButtonPort, INPUT);


  
  digitalWrite(25, HIGH);  
  delay(200); 
  digitalWrite(25, LOW);   


  RING_LEDS.begin();           // INITIALIZE NeoPixel ring object (REQUIRED)
  RING_LEDS.clear();
  RING_LEDS.show();            // Turn OFF all pixels ASAP
  RING_LEDS.setBrightness(100); // Set BRIGHTNESS to about 1/5 (max = 255)

  SHAPE_LEDS.begin();           // INITIALIZE NeoPixel ring object (REQUIRED)
  SHAPE_LEDS.clear();
  SHAPE_LEDS.show();            // Turn OFF all pixels ASAP
  SHAPE_LEDS.setBrightness(100); // Set BRIGHTNESS to about 1/5 (max = 255)
  COMS_CONTROLLER.Init();
  COMS_CONTROLLER.StartListening();
  
  while(!isButtonPressed(startButtonPort)){
    digitalWrite(25, HIGH);  
    delay(10);
  }
  digitalWrite(25, LOW);  

  
  SerialA.begin(9600);
  SerialB.begin(9600);
  Serial.begin(9600);


  TxMessage newMessage = {2, GAME_START, {0,0,0}};
  // SerialA.write((const char*) newMessage, sizeof(TxMessage));
  delay(100);

  int k = 0;
  while(SerialB.available() >= 1){
    char temp; 
    temp = SerialB.read();
    // for(int i = 0; i < temp; i++){
      Serial.print("receivedd : ");
      Serial.println(temp);
      // RING_LEDS.setPixelColor(i, LedController.LEDcolors[k] );
    // }
    // RING_LEDS.show();
    // k = k == 3 ? 3 : k + 1;
    k++;
  }
  delay(10000);
  digitalWrite(25, HIGH);  

}

void loop() {
  switch (STATE)
  {
    //CONECTION STATE
   case (0):
      LedController.turnOnBorders();

      // PROTOTYPE TEST CODE
      for(int i = 0; i < 4; i++){
        if(isButtonPressed(buttonPorts[i])){
          connected[i] = !connected[i];
          if(connected[i]){
            playersList[i].isConnected = true;
            LedController.turnOnQuarter(i, i);
          } else {
            LedController.turnOffQuarter(i);
          }
        }
      }

      // //IMPLEMENTATION
      // if(COMS_CONTROLLER.Available()){
      //   if (COMS_CONTROLLER.ReceiveMessage(COMS_CONTROLLER.reception)) {
      //     if(COMS_CONTROLLER.reception.command == BUTTON_PRESS || COMS_CONTROLLER.reception.id > 0){
      //       int playerId = COMS_CONTROLLER.reception.id;
      //       Player* player = findPlayerIndexById(playerId);
      //       player->isConnected = true; 
      //       // RING_LEDS.setPixelColor(playerId, WHITE);
      //       LedController.turnOnQuarter(playerId - 1, player->playerColor);
      //     }
      //   }
      // }
      int receivedId;
      for(int i = 0; i < MAX_PLAYERS; i++){
        receivedId = readUart(i); 
        Player* player = &playersList[i];
        if(!player->isConnected && receivedId != 1){
          numPlayers++; 
          player->isConnected = true; 
          LedController.turnOnQuarter(i, player->getColor());
          sendBaseCommand(i, START_CONNECTION);
        }
      }

      if(isButtonPressed(startButtonPort)){
        STATE = 1;
      }
      // handleConnectionLights(10);
      delay(50);
    break;
  //START GAME STATE
  case (1): 
    sendToAllConnected(GAME_START);
    LedController.startBlink();
    STATE = 2;
    break; 
  //PREPARE ROUND STATE
  case (2): 
    currentRound.setUpNewRound();
    sendToAllConnected(ROUND_START);
    STATE = 3; 
    break;
  //ONGOING ROUND STATE
  case (3): 
    RoundLedInfo* currentLed; 
    //HANDLE SHAPE LEDs
    for(int i = 0; i < 4; i++){
      currentLed = currentRound.getRoundLedInfo(i);
      if(currentLed->enabled){
        int locID = currentLed->locationID;
        uint32_t colID = currentLed->color;
        LedController.turnOnShape(locID, colID);
      } else {
        LedController.turnOffShape(i);
      }  
    }
    //TEST CODE
    for(int i = 0; i < 4; i++){
      currentLed = currentRound.getRoundLedInfo(i);
      bool isPlayerConnected = playersList[i].isConnected, 
            sameColorAndShape = playersList[i].getColor() == currentLed->color;
      if(isButtonPressed(buttonPorts[i]) && currentLed->enabled && sameColorAndShape && isPlayerConnected){
        currentLed->enabled = false; 
        playersList[i].incScore();
      }
    }

    //IMPLEMENTATION
      if (COMS_CONTROLLER.ReceiveMessage(COMS_CONTROLLER.reception)) {
        if(COMS_CONTROLLER.reception.command == BUTTON_PRESS){
          int playerId = COMS_CONTROLLER.reception.id;
          Player* player = findPlayerIndexById(playerId);
          if(player != NULL){
            currentLed = currentRound.getRoundLedInfoByColor(player->getColor());
            if (COMS_CONTROLLER.reception.button == currentLed->button){
              currentLed->enabled = false; 
              player->incScore();
              sendToSingleController(player->playerId, SCORE_UPDATE, player->playerScore);
            }
          }
        }
      }
    if(millis() - currentRound.startTime >= currentRound.duration){
      STATE = 4; 
    }
      
//how many -> which colors -> which buttons [0,1,2,3]

    break; 

  case (4): 
    currentRound.resetLedInfo();
    RING_LEDS.clear();
    SHAPE_LEDS.clear();
    SHAPE_LEDS.show();
    STATE = 2; 
    delay(100);
    break; 
  }
}

bool isButtonPressed(int buttonID){
  bool buttonPressed = (digitalRead(buttonID) == HIGH),
       buttonBeingHeld = buttonPressDuration[buttonID] != -1;

  if(buttonPressed && !buttonBeingHeld){
    buttonPressDuration[buttonID] = millis();
    return true;
  } else if (!buttonPressed){
    buttonPressDuration[buttonID] = -1;
  }

  return false; 
}

Player* findPlayerIndexByColor(uint32_t color){
  for(int i = 0; i < 4; i++){
    if (playersList[i].getColor() == color)
    {
      return &playersList[i]; 
    } 
  }
  return NULL; 
}
Player* findPlayerIndexById(int playerId){
  for(int i = 0; i < 4; i++){
    if (playersList[i].playerId == playerId)
    {
      return &playersList[i]; 
    } 
  }
  return NULL; 
}
void sendToSingleController(int playerIndex, BaseCommandType command, int data1, int data2, int data3){
  // COMS_CONTROLLER.transmission.command = command;
  // COMS_CONTROLLER.transmission.id = playersList[playerIndex].playerId;
  // COMS_CONTROLLER.transmission.data[0] = data1;
  // COMS_CONTROLLER.transmission.data[1] = data2;
  // COMS_CONTROLLER.transmission.data[2] = data3;
  // COMS_CONTROLLER.SendMessage(COMS_CONTROLLER.transmission);
}

void sendToAllConnected(BaseCommandType command, int data1, int data2, int data3){
  // for(int i = 0; i < 4; i++){
  //   if(playersList[i].isConnected){
  //     COMS_CONTROLLER.transmission.command = command;
  //     COMS_CONTROLLER.transmission.id = playersList[i].playerId;
  //     COMS_CONTROLLER.transmission.data[0] = data1;
  //     COMS_CONTROLLER.transmission.data[1] = data2;
  //     COMS_CONTROLLER.transmission.data[2] = data3;
  //     COMS_CONTROLLER.SendMessage(COMS_CONTROLLER.transmission);
  //   }
  // }
}

void sendBaseCommand(int controllerId, BaseCommandType command){
  UART sender = controllerId == 0 ? SerialA : SerialB;
  
  sender.println("C" + (char) command);

}

void sendScoreUpdate(int controllerId, int score){
  UART sender = controllerId == 0 ? SerialA : SerialB;
  
  sender.println("S" + (char) score);

}

int readUart(int controllerId){
  UART receiver = controllerId == 0 ? SerialA : SerialB;
  int buttonId = -1; 
   if (receiver.available() > 0) {
    
    char c = receiver.read();
    if (c == 'B' ) {
      int temp = receiver.read() - '0';
      if(temp > 0 && temp < 5){
        buttonId = temp; 
      }

      while(receiver.available() > 0){
        receiver.read();
      }
    } 
  }  
  return buttonId;
}