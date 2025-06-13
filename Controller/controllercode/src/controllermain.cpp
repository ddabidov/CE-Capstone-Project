// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// Released under the GPLv3 license to match the rest of the
// Adafruit NeoPixel library
#include <CommLib.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif


// Which pin on the Arduino is connected to the NeoPixels?
#define PIN        11 //led ring pin
const int starpin = 12;  // the number of the pushbutton pin
const int trianglepin = 9;  // the number of the pushbutton pin
const int hexpin = 8;  // the number of the pushbutton pin
const int squarepin = 13;  // the number of the pushbutton pin
const int mosfetPin = 26;
extern ControllerSpeak Controller;
UART Serial1(21, 22); // RX, TX pins for UART communication

#define NUMPIXELS 35 //NeoPixel ring size
int starbutton = digitalRead(starpin);
int trianglebutton = digitalRead(trianglepin);
int hexbutton = digitalRead(hexpin);
int squarebutton = digitalRead(squarepin);
int buzzertimer;
float currenttime;
uint8_t currentscore = 0; // Declare currentscore as a global variable


// When setting up the NeoPixel library, we tell it how many pixels,
// and which pin to use to send signals. Note that for older NeoPixel
// strips you might need to change the third parameter -- see the
// strandtest example for more information on possible values.
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#define DELAYVAL 500 // Time (in milliseconds) to pause between pixels

void buzzer(int i); // Function prototype for buzzer
void theaterChase(uint32_t c, uint8_t wait); // Function prototype for theaterChase
void buttons(); // Function prototype for buttons
void score(uint8_t j); // Function prototype for score

#define UART_BUFFER_SIZE 16
char uartBuffer[UART_BUFFER_SIZE];
uint8_t uartIndex = 0;

void setup() {
  // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
  // Any other board, you can remove this part (but no harm leaving it):
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  // END of Trinket-specific code.
  starbutton = digitalRead(starpin);
  trianglebutton = digitalRead(trianglepin);
  hexbutton = digitalRead(hexpin);
  squarebutton = digitalRead(squarepin);
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
    // Set the GPIO pin as an output
  pinMode(mosfetPin, OUTPUT);
  // Start with the MOSFET off
  digitalWrite(mosfetPin, LOW); // or HIGH for a P-channel MOSFET
  // Set the ID for the controller
  Controller.transmission.id = 1;
  Serial1.begin(9600);
}

void loop() {
      

   while (Serial1.available() > 0) {
    char c = Serial1.read();
    if (c == '\n' || c == '\r') {
      uartBuffer[uartIndex] = '\0'; // Null-terminate
      parseUartMessage(uartBuffer);
      uartIndex = 0; // Reset for next message
    } else if (uartIndex < UART_BUFFER_SIZE - 1) {
      uartBuffer[uartIndex++] = c;
    } else {
      uartIndex = 0; // Buffer overflow, reset
    }
  }
    
  buttons(); // Check for button presses

  if(((millis())-currenttime) > 1000) { // Check if 1 second has passed
    buzzer(0); // Increment the buzzer timer
  }
}
// Parse UART message and act accordingly
void parseUartMessage(const char* msg) {
  pixels.clear(); // Set all pixel colors to 'off'
  int r = random(150, 255);
  int b = 0;
  int g = random(100);
  if (strncmp(msg, "C", 1) == 0 && strlen(msg) == 2) {
    int command = (int)msg[1];
    switch (command) {
    case START_CONNECTION:
      theaterChase(pixels.Color(r, g, b), 50); // Set all pixels to a random color
      score(currentscore); // Update score based on received data
      break;
    case ROUND_START:
      score(currentscore); // Update score based on received data
      break;
    case ROUND_WON:
      theaterChase(pixels.Color(0, 150, 0), 50); // Show a green chase effect
      buzzer(1); // Activate buzzer
      score(currentscore); // Update score based on received data
      break;
    case WRONG_BUTTON:
      pixels.fill(pixels.Color(150, 0, 0)); // Set all pixels to red
      pixels.show(); // Update the strip to show the color
      buzzer(1);
      score(currentscore); // Update score based on received data
      break;
    case ROUND_LOST:
      pixels.fill(pixels.Color(150, 0, 0)); // Set all pixels to red
      pixels.show(); // Update the strip to show the color
      buzzer(1);
      score(currentscore); // Update score based on received data
      break;
    case ROUND_END:
      score(currentscore); // Update score based on received data
      break;
    case SCORE_UPDATE:
      score(currentscore); // Update score based on received data
      break;
    case GAME_START:
      buttons(); // Check for button presses
      score(currentscore); // Update score based on received data
      break;
    case GAME_WON:
      theaterChase(pixels.Color(0, 150, 0), 50); // Show a green chase effect
      buzzer(1); // Activate buzzer
      score(currentscore); // Update score based on received data
      break;
    case GAME_LOST:
      theaterChase(pixels.Color(150, 0, 0), 50); // Show a red chase effect
      buzzer(1); // Activate buzzer
      score(currentscore); // Update score based on received data
      break;
    default:
      score(currentscore); // Update score based on received data
      break;
  }
}
  if (strncmp(msg, "S", 1) == 0 && strlen(msg) == 2) {
    currentscore = (int)msg[1];
    }
    }

void buttons()
{
  starbutton = digitalRead(starpin);
  trianglebutton = digitalRead(trianglepin);
  hexbutton = digitalRead(hexpin);
  squarebutton = digitalRead(squarepin);
if(starbutton == HIGH)
{
  Serial1.println("B1");
    //Controller.SendButtonPress(STAR);
}
else if(trianglebutton == HIGH)
{
  Serial1.println("B3");
    //Controller.SendButtonPress(TRIANGLE);
}
else if(hexbutton == HIGH)
{
  Serial1.println("B2");
    //Controller.SendButtonPress(HEXAGON);
}
else if(squarebutton == HIGH)
{
  Serial1.println("B0");
    //Controller.SendButtonPress(SQUARE);
}
  }



void buzzer(int i)
{
  digitalWrite(mosfetPin, i); // or LOW for a P-channel MOSFET
  currenttime = millis(); // Update the current time
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<3; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < pixels.numPixels(); i=i+3) {
        pixels.setPixelColor(i+q, c);    //turn every third pixel on
      }
      pixels.show();

      delay(wait);

      for (uint16_t i=0; i < pixels.numPixels(); i=i+3) {
        pixels.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

void score(uint8_t j)
{
for(int i=0; i<j; i++) { // For each pixel...

    // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
    // Here we're using a moderately bright green color:
    pixels.setPixelColor(i, pixels.Color(0, 0, 150));
    pixels.show();
  }
}