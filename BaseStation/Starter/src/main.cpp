#include <Arduino.h>
#include <string.h>
#include <Adafruit_NeoPixel.h>
#include <CommLib.h>

//constants
#define RING_LED_PIN  15
#define SHAPE_LED_PIN  3
#define RING_LED_COUNT 60
#define SHAPE_LED_COUNT 30
#define MAX_PLAYERS 2

// 2 UART channels
UART SerialA(4, 5);
UART SerialB(8, 9);

//LED Object Init
Adafruit_NeoPixel RING_LEDS(RING_LED_COUNT, RING_LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel SHAPE_LEDS(SHAPE_LED_COUNT, SHAPE_LED_PIN, NEO_GRB + NEO_KHZ800);


//state machine state
int STATE = 0;

//quick color hexcodes
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

//struct for LED ranges to partition ring quarters and shapes
typedef struct LEDranges {
  int min;
  int max;
} LEDranges;

//struct to hold info for shape leds during each round
typedef struct RoundLedInfo {
  int locationID;     //index for LEDs
  ButtonType button;  //shape of led / corresponding button
  int colorID;        //deprecated color index
  uint32_t color;     //color for this round
  bool enabled;       //turned on for this round
} RoundLedInfo;


//class for each player
class Player
{
public:
  int playerId;               //id / index in player list
  uint32_t playerColor;       //controller color
  int playerColorID;          //deprecated color index
  int playerScore;            //player score
  bool isConnected = false;   // controller is connected for this player
  

  Player(int id, uint32_t color){
    this->playerId = id; 
    this->playerColor = color;
    this->playerScore = 0; 
    this->isConnected = false; 
  }

  //quick function to increase score
  void incScore(){
    // Serial.print("Increasing Score from ");
    // Serial.print(playerScore);
    this->playerScore++;
    // Serial.print(" to ");
    // Serial.println(playerScore);
  }
  uint32_t getColor() {
    return playerColor;
  }
  int getScore(){
    return playerScore;
  }
};

//Class to control all LEDs
class LED {
  public:
    //white border pixel locations
    int borderLEDs[4] = {0, 15, 30, 45};

    //LED/pixel ranges for ring quarters and shapes
    LEDranges ringQuarterRanges[4] = { {1, 14}, {16, 29}, {31, 44}, {46, 60}};
    LEDranges shapeRanges[4] = {{0, 6}, {7, 13}, {14, 20}, {21, 27}};

    //colors for players
    uint32_t LEDcolors[4] = {BLUE, PINK, ORANGE, YELLOW};

    //deprecated, returns id/index of given player color
    int getColorID(uint32_t color){
      for(int i = 0; i < 4; i++){
        if(color == LEDcolors[i]){
          return i;
        }
      }
      return -1; 
    }

    //turn on border pixels, default white
    void turnOnBorders(uint32_t color = 0xFFFFFF){
      for(int ledId: borderLEDs){
        RING_LEDS.setPixelColor(ledId, color);
      }
      RING_LEDS.show();
    }

    //turn on shape with given color
    void turnOnShape(int shapeID, uint32_t color){
      LEDranges shapeRange = shapeRanges[shapeID]; 
      for (int i = shapeRange.min; i <= shapeRange.max; i++)
      {
        SHAPE_LEDS.setPixelColor(i, color);
      }
      SHAPE_LEDS.show();
    }

    //alternate turn on shape that uses color ID
    void turnOnShape(int shapeID, int colorID){
      this->turnOnShape(shapeID, LEDcolors[colorID]);
    }

    //turn off given shape
    void turnOffShape(int shapeID){
      LEDranges shapeRange = shapeRanges[shapeID]; 
      for (int i = shapeRange.min; i <= shapeRange.max; i++)
      {
        SHAPE_LEDS.setPixelColor(i, 0x0);
      }
      SHAPE_LEDS.show();
    }

    //turn on ring quarter
    void turnOnQuarter(int quarterID, uint32_t color){
      LEDranges quarterRange = ringQuarterRanges[quarterID]; 
      for (int i = quarterRange.min; i <= quarterRange.max; i++)
      {
        RING_LEDS.setPixelColor(i, color);
      }
      RING_LEDS.show();
    }

    //alternate to turn on led with color id
    void turnOnQuarter(int quarterID, int colorID){
      this->turnOnQuarter(quarterID, LEDcolors[colorID]);
    }

    //turn off given quarter
    void turnOffQuarter(int quarterID){
      LEDranges quarterRange = ringQuarterRanges[quarterID];
      for (int i = quarterRange.min; i <= quarterRange.max; i++)
      {
        RING_LEDS.setPixelColor(i, 0x0);
      }
      RING_LEDS.show();
    }
     
    //LED pattern before game start
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
    unsigned long duration; //duration in ms of round

    unsigned long startTime; // round start time, from millis()

    RoundLedInfo leds[4];   //led info for current round

    //deprecated - chance to turn on 1/2/3/4 shape leds for current round
    int numOnChanceProportions[4] = {3, 3, 3, 1};

    bool hasBeenWon = false;   //whether round has been won yet

    int WinnerIndex = -1; //player index/id who won this round

    bool hasPlayerTried[4]; //has player made their attempt this round

    //configure led info on init
    GameRound () {
      leds[0].button = TRIANGLE;
      leds[0].locationID = 0;
      leds[1].button = HEXAGON;
      leds[1].locationID = 1;
      leds[2].button = STAR;
      leds[2].locationID = 2;
      leds[3].button = SQUARE;
      leds[3].locationID = 3;
    }

    //reset variables for new round
    void setUpNewRound() {
      selectIDs();
      startTime = millis();
      duration = 2000;
      hasBeenWon = false; 
      WinnerIndex = -1; 
      hasPlayerTried[0] = false; 
      hasPlayerTried[1] = false; 
      hasPlayerTried[2] = false; 
      hasPlayerTried[3] = false; 
    }
    //return LED info of shape led that has the given color
    RoundLedInfo* getRoundLedInfoByColor(uint32_t color){
      for(int i = 0; i < 4; i ++){
        if (leds[i].color == color){
          return &leds[i];
        }
      }
      return NULL;
    }
    //return LED info of given shape LED
    RoundLedInfo* getRoundLedInfoByShape(ButtonType shape){
      for(int i = 0; i < 4; i ++){
        if (leds[i].button == shape){
          return &leds[i];
        }
      }
      return NULL;
    }
    //return whether given player has made their attempt this round
    bool getHasPlayerTried(int playerIndex){
      return hasPlayerTried[playerIndex];
    }
    //record that given player made their attempt this round
    void playerTried(int playerIndex){
      hasPlayerTried[playerIndex] = true; 
    }
    //deprecated - select the number of shapes to be turned on this round
    int selectNumLedsOn(){
      srand(millis());
      int chanceMax = chanceSum(3);
      int chance = random(1, chanceMax);
      int numOn; 
      if(chance > chanceSum(2)){
        numOn = 1;
      } else if (chance > chanceSum(1)){
        numOn = 2;
      } else if (chance > numOnChanceProportions[0]){
        numOn = 3;
      } else {
        numOn = 4; 
      }

      return numOn;
    }
    //deprecated - return sum of chances for 1/2/3/4 LEDs to be turned on
    int chanceSum(int upToID){
      int sum = 0;
      for(int x = 0; x <= upToID; x++){
        sum = sum + x; 
      }
      return sum;
    }
    //reset LED info for new round
    void resetLedInfo(int startIndex = 0){
      for( int x = startIndex; x < 4; x++ ){
        leds[x].color = 0x0; 
        leds[x].colorID = -1; 
        leds[x].enabled = false; 
      }
    }

    //choose which shape LEDs get which colors 
    void selectIDs(){
      srand(millis());
      uint32_t availableIDs[4] = {PINK,BLUE,ORANGE,YELLOW};
      int numAvailable = 4; 

      
      for(int i = 0; i < 3; i++){
        //select random index for random color
        int selectedIndex = random(0, numAvailable - 1);
        
        //assign randomly selected color in shape LED's info
        leds[i].color = availableIDs[selectedIndex]; 
        leds[i].enabled = true; 

        //swap the randomly selected color with last index
        int lastIndex = numAvailable - 1;
        if(selectedIndex != numAvailable - 1){
          int temp = availableIDs[lastIndex];
          availableIDs[lastIndex] = availableIDs[selectedIndex];
          availableIDs[selectedIndex] = temp;
        }
        
        //reduce number available to ignore colors at end of array
        numAvailable--;
    }

    //choose last, unselected color for final shape led
    leds[3].color = availableIDs[0];
    leds[3].enabled = true; 
  }

  //get shape LED info by shape index
  RoundLedInfo* getRoundLedInfo(int index){
    return &leds[index];
  }
      
  //deprecated - makes ring LED update timer pattern
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

// VARIABLES FOR  PROTOTYPE
//bool connected[4] = {false, false, false, false}; //testing connection for 4 players
//int buttonPorts[4] = {11, 12, 13, 14}; //testing button ports



// Function Declarations
bool isButtonPressed(int buttonID);
Player* findPlayerIndexById(int playerId);
Player* findPlayerIndexByColor(uint32_t color);
void sendToSingleController(int playerIndex, BaseCommandType command, int data1 = 0, int data2 = 0, int data3 = 0);
void sendToAllConnected(BaseCommandType command);
int readUart(int controllerId);
void sendBaseCommand(int controllerId, BaseCommandType command);
void sendScoreUpdate(int controllerId, int score);
void DEV_TURN_ON_ALL();
//*****************************************************************************************************

//deprecated - round counter to determine round duration
int roundNum = 0; 

// primary LED controller
LED LedController;

// primary Round object
GameRound currentRound; 

//instantiate player objects
Player player1(1, BLUE);
Player player2(2, PINK);
Player player3(3, ORANGE);
Player player4(4, YELLOW);
// player array
Player playersList[4] = {player1, player2, player3, player4};
//number of players connected
int numPlayers = 0; 
// index of winning player in player lists - tracks winning player index between states
int winnerIndex = -1; 

//base station button ports
int buttonPorts[3] = {11, 12, 13};
// base station button length of button press - used for debouncing
long buttonPressDuration[3] = { -1, -1, -1}; 


void setup() {
  //set button pins to input mode
  pinMode(buttonPorts[0], INPUT); 
  pinMode(buttonPorts[1], INPUT); 
  pinMode(buttonPorts[2], INPUT); 



  //blink to indicate program starting
  // digitalWrite(25, HIGH);  
  // delay(200); 
  // digitalWrite(25, LOW);   

  //Init LED objects
  RING_LEDS.begin(); 
  RING_LEDS.clear();
  RING_LEDS.show();            
  RING_LEDS.setBrightness(100); 

  SHAPE_LEDS.begin();          
  SHAPE_LEDS.clear();
  SHAPE_LEDS.show();           
  SHAPE_LEDS.setBrightness(100); 

  //init radio module
  // COMS_CONTROLLER.Init();
  // COMS_CONTROLLER.StartListening();
  
  //init UART modules
  SerialA.begin(9600);
  SerialB.begin(9600);
  Serial.begin(9600);

}

void loop() {
  bool endGame = false; 
  switch (STATE)
  {
    //CONECTION STATE
   case (0):
      LedController.turnOnBorders();

      // UART IMPLEMENTATION

      int receivedId;
      //for each player/UART channel
      for(int i = 0; i < MAX_PLAYERS; i++){
        //Check for button press
        receivedId = readUart(i); 
        Player* player = &playersList[i];
        
        //check if button was received, player is connected yet
        if(receivedId != -1 && !player->isConnected ){
          //count and connect player
          numPlayers++; 
          player->isConnected = true; 

          //turn on player's color
          LedController.turnOnQuarter(i, player->getColor());

          //communicated connection to controller 
          sendBaseCommand(i, START_CONNECTION);
          delay(20);
          sendScoreUpdate(i, 0);
        }
      }

      //start game if yellow button is pressed
      if(isButtonPressed(buttonPorts[1])){
        STATE = 1;
      }
      delay(50);
    break;

  //START GAME STATE
  case (1): 
    //alert connected players/controllers that gane is starting
    sendToAllConnected(GAME_START);
    LedController.startBlink();
    STATE = 2;
    break; 
  //PREPARE ROUND STATE
  case (2): 
    //setup round object, alert controllers
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

    //UART IMPLEMENTATION

    //go through UART channels
    for(int i = 0; i < MAX_PLAYERS; i++){
        
        //check for received button press
        receivedId = readUart(i); 
        Player* player = &playersList[i];

                                  //check if....
        if(receivedId != -1 &&                      // button press was received
            player->isConnected &&                  // player is connected
            !currentRound.hasBeenWon &&             // current round hasnt been won
            !currentRound.getHasPlayerTried(i) )    // player hasnt already made an attempt
        {  
          //get LED info for LED with player's color
          currentLed = currentRound.getRoundLedInfoByColor(player->getColor());

          //check if player pressed same shape as led with their color
          if(receivedId == currentLed->locationID){
            //turn off led
            currentLed-> enabled = false; 
            //increase player score
            player->incScore();
            //set winner
            currentRound.hasBeenWon = true; 
            currentRound.WinnerIndex = i; 
          }

          //mark player as having made an attempt
          currentRound.playerTried(i);
        }       

    }

    //move to next state when duration expires or when player wins
    if(millis() - currentRound.startTime >= currentRound.duration || currentRound.hasBeenWon){
      STATE = 4; 
    }
    break; 

  // AFTER ROUND STATE
  case (4): 

    //reset led info
    currentRound.resetLedInfo();

    //send round lost message to losers
    for(int i = 0; i < MAX_PLAYERS; i++ ){
      if(playersList[i].isConnected && i != currentRound.WinnerIndex ){
        sendBaseCommand(i, ROUND_LOST);
      }
    }

    //sent round won + score update to winner
    if(currentRound.hasBeenWon){
      Player* winner = &playersList[currentRound.WinnerIndex];
      sendBaseCommand(currentRound.WinnerIndex, ROUND_WON);
      delay(50);
      sendScoreUpdate(currentRound.WinnerIndex, winner->getScore());

      //check if round winner won game
      if(winner->getScore() >= 30){
        endGame = true; 
        winnerIndex = currentRound.WinnerIndex;
      }
    }


    //clear LEDs
    RING_LEDS.clear();
    SHAPE_LEDS.clear();
    SHAPE_LEDS.show();

    //new round if no winner, final state if someone won game
    if(endGame){
      STATE = 5; 
    } else {
      STATE = 2; 
    }

    delay(100);
    break; 
    
  case (5):
    //alert winner of their success
    sendBaseCommand(winnerIndex, GAME_WON);

    //alert losers of their failure
    for(int i = 0; i<MAX_PLAYERS; i++){
      if(playersList[i].isConnected && i != winnerIndex){
        sendBaseCommand(i, GAME_LOST);
      }
    }

    //end game
    STATE = 6; 

    break; 
  }
}

//confirms button press when button is first pressed, but not when held
bool isButtonPressed(int buttonID){
  //read buttons pressed/held status
  bool buttonPressed = (digitalRead(buttonID) == HIGH),
       buttonBeingHeld = buttonPressDuration[buttonID] != -1;

  //return true when first pressed
  if(buttonPressed && !buttonBeingHeld){
    buttonPressDuration[buttonID] = millis();
    return true;
  } else if (!buttonPressed){
    //updated 'being held' status if not pressed
    buttonPressDuration[buttonID] = -1;
  }

  return false; 
}

//deprecated - find player from list based on color
Player* findPlayerIndexByColor(uint32_t color){
  for(int i = 0; i < 4; i++){
    if (playersList[i].getColor() == color)
    {
      return &playersList[i]; 
    } 
  }
  return NULL; 
}
//deprecated - get's player's index by their ID
Player* findPlayerIndexById(int playerId){
  for(int i = 0; i < 4; i++){
    if (playersList[i].playerId == playerId)
    {
      return &playersList[i]; 
    } 
  }
  return NULL; 
}

//send command to all connected controllers
void sendToAllConnected(BaseCommandType command){
  for(int i = 0; i < 4; i++){
    if(playersList[i].isConnected){
      sendBaseCommand(i, command);
    }
  }
}

//send command to one given controller
void sendBaseCommand(int controllerId, BaseCommandType command){
  UART* sender = controllerId == 0 ? &SerialA : &SerialB;

  char sendCommandFormat = command + '0';
  sender->print('C');
  sender->println(sendCommandFormat);
  // Serial.println("Sent command to ");
  // Serial.print(controllerId);
  // Serial.print(" command type ");
  // Serial.print((int) command);
  // Serial.print(" from ");
  // Serial.println(controllerId == 0 ? "SerialA" : "SerialB");
}

//send score update to given controller
void sendScoreUpdate(int controllerId, int score){
  UART* sender = controllerId == 0 ? &SerialA : &SerialB;
  

  sender->print('S');
  char scoreAsByte = score; 
  sender->print(scoreAsByte);

  // Serial.print("Sent score to ");
  // Serial.print(controllerId);
  // Serial.print(" score of ");
  // Serial.print(score);
  // Serial.print(" as byte ");
  // Serial.println(scoreAsByte);

  delay(20);
  sendBaseCommand(controllerId, SCORE_UPDATE);

}

//read button press from given UART channel 
int readUart(int controllerId){
  UART* receiver = controllerId == 0 ? &SerialA : &SerialB;
  int buttonId = -1; 
   if (receiver->available() > 0) {
    
    char c = receiver->read();
    Serial.print("recevied char: ");
    Serial.println(c);
    if (c == 'B' ) {
      char temp1 = receiver->read(); 
      int temp = temp1 - '0';
      if(temp >= 0 && temp <= 3){
        buttonId = temp; 
        Serial.print("recevied Button #");
        Serial.print(temp1);
        Serial.print(" or ");
        Serial.print(temp);
        Serial.print(" from ");
        Serial.println(controllerId == 0 ? "SerialA" : "SerialB");
        
        
      }

      while(receiver->available() > 0){
        receiver->read();
      }
    } 
  }  
  return buttonId;
}

// dev function to turn on all lights with all colors
void DEV_TURN_ON_ALL(){
  LedController.turnOnQuarter(0, player1.getColor());
  LedController.turnOnQuarter(1, player2.getColor());
  LedController.turnOnQuarter(2, player3.getColor());
  LedController.turnOnQuarter(3, player4.getColor());
  LedController.turnOnShape(0, player1.getColor());
  LedController.turnOnShape(1, player2.getColor());
  LedController.turnOnShape(2, player3.getColor());
  LedController.turnOnShape(3, player4.getColor());
  LedController.turnOnBorders();
  RING_LEDS.show();
  SHAPE_LEDS.show();

}