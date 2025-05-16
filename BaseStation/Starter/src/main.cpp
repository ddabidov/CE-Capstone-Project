#include <Arduino.h>
#include <Adafruit_NeoPixel.h>



#define LED_PIN  3

#define LED_COUNT 60

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);


void handleConnectionLights(uint8_t wait);

//state machine variables ****************************************************************************
int state = 0;

//Player variables ***********************************************************************************
bool connected[4] = {false, false, false, false};
int buttonPorts[4] = {26, 22, 20, 18};
int startButtonPort = 16;

uint16_t LEDrangeMin[4] = {1, 16, 31, 46};
uint16_t LEDrangeMax[4] = {14, 29, 44, 60}; 
uint32_t LEDcolors[4] = {0xFFFFFF, 0xFF0000, 0x00FF00, 0x0000FF};

long buttonPressDuration[4] = { -1, -1, -1, -1};
bool roundInProgress = false; 
int roundColorIDs[4] = {-1, -1, -1, -1};
int roundLocationIDs[4] = {-1, -1, -1, -1};
int roundNum = 0; 
long roundStartTime; 
int tempLEDID = 0; 

#define LEDring mainRing ;


bool isButtonPressed(int buttonID);
void startBlink();
void selectIDs(int selectedIDs[], int numToSelect);

void setup() {
  // put your setup code here, to run once:
  pinMode(buttonPorts[0], INPUT); // Sets the pin as an output
  pinMode(buttonPorts[1], INPUT); // Sets the pin as an output
  pinMode(buttonPorts[2], INPUT); // Sets the pin as an output
  pinMode(buttonPorts[3], INPUT); // Sets the pin as an output
  pinMode(startButtonPort, INPUT);
  digitalWrite(25, HIGH);
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)

  // mainRing = new LEDring();
}

void loop() {

  switch (state)
  {
  case (0):
      for(int i = 0; i < 4; i++){
        if(isButtonPressed(buttonPorts[i])){
          connected[i] = !connected[i];
          // if(connected[i]){
          //   mainRing.turnOnQuarter(i);
          // } else {
          //   mainRing.turnOffQuarter(i);
          // }
        }
      }

      if(isButtonPressed(startButtonPort)){
        state = 1;
      }
      handleConnectionLights(10);
      delay(50);
    break;
  case (1): 
    startBlink();

  //**************************************************LED Ring Round Timer thingy************************************* */
    // strip.clear();
    // int startID = 0;
    // int currentID = 1;
    // int maxID = LED_COUNT;
    // unsigned long startTime = millis();
    // while(startID < maxID){

    //   if(currentID >= maxID){
    //     strip.clear();
    //       startID++;
    //       currentID = startID + 1;
    //       startTime = millis();
    //       for(uint16_t i = 0; i < startID; i++){
    //         strip.setPixelColor(i, 0xFFFFFF);
    //       }
    //       // strip.show();
    //     }

    //     if(millis() >= startTime + (10 * (currentID - startID))){
    //       strip.clear();

    //       for(uint16_t i = 0; i < currentID; i++){
    //         if(i < startID){
    //           strip.setPixelColor(i, 0x0000FF);
    //         } else {
    //           strip.setPixelColor(i, 0xFFFFFF);
    //         }
    //       }
    //       currentID++;
    //     }

    //     strip.show();

    // }
//******************************************************************************************************************* */
    // delay(5000);
    state = 2;
    break; 
  case (2): 
  if(!roundInProgress){
      int vals[4] = {0, 25, 50, 75};
      int chance = random(1,100);
      int numOn; 
      if(chance > 90){
        numOn = 1;
      } else if (chance > 60){
        numOn = 2;
      } else if (chance > 30){
        numOn = 3;
      } else {
        numOn = 4; 
      }
  
      selectIDs(roundColorIDs, numOn);
      selectIDs(roundLocationIDs, numOn);

      strip.clear();
    for(int i = 0; i < numOn;i++){
      int locID = roundLocationIDs[i];
      int colID = roundColorIDs[i];
      for(int x = LEDrangeMin[locID]; x <= LEDrangeMax[locID]; x++){
        strip.setPixelColor(x, LEDcolors[colID]);
      }
    }
    strip.show();
    roundInProgress = true;
    roundStartTime = millis();
    // delay(200);
  
    }
    for(int i = 0; i < 4; i++){
      if(isButtonPressed(buttonPorts[i])){
        
      }
    }
    if(millis() - roundStartTime >= 2000){
      roundInProgress = false; 
    }
      
//how many -> which colors -> which buttons [0,1,2,3]

    break; 
  }
  // digitalWrite(25, 0);
  // unsigned long currentMillis = millis();                     //  Update current time

}

// // put function definitions here:
// int myFunction(int x, int y) {
//   return x + y;
// }
void selectIDs(int selectedIDs[4], int numToSelect){
  int availableIDs[4] = {0,1,2,3};
  int numAvailable = 4; 

  for(int i = 0; i < numToSelect; i++){
    int selected = random(0, numAvailable-1);
    selectedIDs[i] = availableIDs[selected]; 

    int lastIndex = numAvailable - 1;
    if(selected != numAvailable - 1){
      int temp = availableIDs[lastIndex];
      availableIDs[lastIndex] = availableIDs[selected];
      availableIDs[selected] = temp;
    }
    numAvailable--;
  }

  while(numAvailable-- > 0){
    selectedIDs[numToSelect + numAvailable] = -1;
  }
}
void handleConnectionLights(uint8_t wait) {

  for(int x = 0; x < 4; x++){
    if (connected[x])
    {
      for(uint16_t i=LEDrangeMin[x]; i <= LEDrangeMax[x]; i++){
        strip.setPixelColor(i, LEDcolors[x]);
      }
    } else if (!connected[x]){
      for(uint16_t i=LEDrangeMin[x]; i <= LEDrangeMax[x]; i++){
        strip.setPixelColor(i, 0x0);
      }
    }
    
  }

  for(uint16_t i=0; i < 60; i++){
    if(i % 15 == 0){
      strip.setPixelColor(i, 0xFFFFFF);
    }
  }
  
  strip.show();                             //  Update strip to match
  digitalWrite(25, HIGH);
}

void startBlink() {
  int wait = 10;
  uint32_t color = 0x00FF00;
  static uint32_t loop_count = 0;
  static uint16_t current_pixel = 0;

  bool found = false; 
  strip.clear();

  for(int i = 0; i < LED_COUNT; i ++){
    for(int x = 0; x < 4; x++){
      if(i >= LEDrangeMin[x] && i <= LEDrangeMax[x]){
        strip.setPixelColor(i, LEDcolors[x]);
        found = true;
        break;
      }
      if(!found){
        strip.setPixelColor(i, 0xFFFFFF);
      }
      found = false; 
    }
    // strip.setPixelColor(i, color);
    strip.show();
    delay(100);
  }

  for(int i = 0; i < 3; i++){
    strip.clear();
    strip.show();
    delay(200);

    for(int c=current_pixel; c < LED_COUNT; c ++) {
      strip.setPixelColor(c, color);
    }
    strip.show();
    delay(200);
  }


  strip.clear();
  strip.show();

}
  
//   for(int i = 0; i < 4; i++){
//     for(int x = LEDrangeMax[i]; x <= LEDrangeMin[i]; x ++){
//       strip.setPixelColor(i, LEDcolors[i]);
//       strip.show();
//       delay(50);
//     }
//   }

//   for(int k = 0; k < 3; k++){
//     for(int x = 0; x < LED_COUNT; x ++){
//       strip.setPixelColor(x, 0x0);
//     }
//     strip.show();
//     delay(250);
//     for(int i = 0; i < 4; i++){
//       for(int x = LEDrangeMax[i]; x <= LEDrangeMin[i]; x ++){
//         strip.setPixelColor(i, LEDcolors[i]);
//       }
//     }
//     strip.show();
//     delay(250);
//   }

// }



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


 typedef struct LEDranges {
    int min;
    int max;
  } LEDranges;


// class LEDring {
//   private:
//     const int PIN = 3;
//     const int NUM_LEDs = 60; 
//     int borderLEDs[4] = {0, 15, 30, 45};
//     LEDranges quarterRanges[4] = { {1, 14}, {16, 29}, {31, 44}, {46, 60}};

//   public: 
//     LEDring (){

//     }

//    void turnOnQuarter(int quarterID, uint32_t color){
//     LEDranges quarterRange = quarterRanges[quarterID];
//     for (int i = quarterRange.min; i < quarterRange.max; i++)
//     {
//       strip.setPixelColor(i, color);
//     }
//     strip.show();
//    }

//    void turnOffQuarter(int quarterID){
//     LEDranges quarterRange = quarterRanges[quarterID];
//     for (int i = quarterRange.min; i < quarterRange.max; i++)
//     {
//       strip.setPixelColor(i, 0x0);
//     }
//     strip.show();
//    }
   
// };