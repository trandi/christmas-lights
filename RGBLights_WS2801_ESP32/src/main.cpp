#include <Arduino.h>
#include <WiFi.h>

#include <Streaming.h> //use Arduino Serial with "<<" and ">>" operators
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <ESP32Encoder.h>


#include "ledpatterns.h"
#include "control.h"


constexpr gpio_num_t ENCODER_PIN_A{GPIO_NUM_25};
constexpr gpio_num_t ENCODER_PIN_B{GPIO_NUM_26};
ESP32Encoder encoder;

constexpr gpio_num_t PUSHBUTTON_PIN{GPIO_NUM_15};
volatile ControlMode controlMode{ControlMode::whitelight};
volatile unsigned long pushButtonPressed_Timestamp{millis()};



// Banggood - Wemos Lolin32 board
// integrated 128x64 I2C SSD1306 OLED display
constexpr uint8_t SCREEN_WIDTH{128};
constexpr uint8_t SCREEN_HEIGHT{64};
constexpr int8_t OLED_RESET{-1}; // Reset pin # (or -1 if sharing Arduino reset pin)
constexpr uint8_t I2C_SDA{5};
constexpr uint8_t I2C_SCL{4};
constexpr uint8_t I2C_ADDRESS{0x3c};

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Strip of 31 WS2801 pixels composed of 4 LEDs each (that are in sync and can't be controlled separately)
// https://learn.adafruit.com/36mm-led-pixels/
constexpr uint8_t NUM_LEDS{31};
constexpr uint8_t DATA_PIN{0};     // 13 MOSI for 2nd SPI bus (HSPI)
constexpr uint8_t CLOCK_PIN{0};    // 14 closk for 2nd SPI bus (HSPI)
// Define the array of leds
CRGB leds[NUM_LEDS];



/*
* The interrupt handling routine should have the IRAM_ATTR attribute, in order for the compiler to place the code in IRAM. 
* Also, interrupt handling routines should only call functions also placed in IRAM, as can be seen in the IDF documentation
*/
void IRAM_ATTR pushButtonCallback() {
  // DEBOUNCE, require a minimum duration between presses
  unsigned long time = millis();
  if (time - pushButtonPressed_Timestamp > 200) {
    controlMode = next((ControlMode &)controlMode);
    pushButtonPressed_Timestamp = time;
  }
}


void setup() {
  // not needed, save energy ?
  WiFi.mode(WIFI_OFF);
  btStop();


  // Serial for debugging 
  Serial.begin(115200);
  Serial << "Starting" << endl;


  // Rotary Encoder & Switch
  pinMode(PUSHBUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(PUSHBUTTON_PIN, pushButtonCallback, RISING);

	ESP32Encoder::useInternalWeakPullResistors = UP;
	encoder.attachSingleEdge(ENCODER_PIN_A, ENCODER_PIN_B);
	// clear the encoder's raw count and set the tracked count to 0
	encoder.setCount(1);  


  // OLED display
  Wire.begin(I2C_SDA, I2C_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, I2C_ADDRESS, false, false)) {
    Serial << "SSD1306 allocation failed" << endl;
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Ready to display...");
  display.display();


  // RGB LED Strip
  FastLED
    .addLeds<WS2801, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);
  FastLED.setBrightness(0);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
}









  
ControlMode lastControlMode{undefined};
int64_t encoderCount{-999};  
void loop()
{
  // Switch
  if(controlMode != lastControlMode) {
    display.fillRect(0, 40, 100, 10, BLACK);
    display.setCursor(0, 40);
    display.print(str((ControlMode &)controlMode));
    display.display();

    lastControlMode = controlMode;

    if(controlMode == whitelight) {
      fill_solid(leds, NUM_LEDS, CRGB::White);
      FastLED.show();FastLED.show();
    }    
  }

  // Encoder
  int64_t newEncoderCount = encoder.getCount();
  if(newEncoderCount != encoderCount) {
    display.fillRect(0, 30, 30, 10, BLACK);
    display.setCursor(0, 30);
    display.print((int)newEncoderCount);
    display.display();

    encoderCount = newEncoderCount;

    // max brightness is 100 (less than the actual 255 max !)
    FastLED.setBrightness(constrain(encoderCount, 0, 10) * 10);
    FastLED.show();FastLED.show();
  }
  


  if(controlMode == animations) {
      // Call the current pattern function once, updating the 'leds' array
      gPatterns[gCurrentPatternNumber](leds);

      

      // send the 'leds' array out to the actual LED strip
      FastLED.show();FastLED.show();
      // insert a delay to keep the framerate modest
      FastLED.delay(1000/FRAMES_PER_SECOND); 

      // do some periodic updates
      EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
      EVERY_N_SECONDS( 10 ) { nextPattern(); } // change patterns periodically
  }
}