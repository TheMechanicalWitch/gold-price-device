#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeSerif12pt7b.h>
#include <Fonts/FreeSerifBold12pt7b.h>
#include <ESP8266WiFi.h>

#define testVal 12345.67
#define PORT 8080

Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);

// OLED FeatherWing buttons map to different pins depending on board:
#if defined(ESP8266)
  #define BUTTON_A  0
  #define BUTTON_B 16
  #define BUTTON_C  2
#elif defined(ESP32) && !defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S2)
  #define BUTTON_A 15
  #define BUTTON_B 32
  #define BUTTON_C 14
#elif defined(ARDUINO_STM32_FEATHER)
  #define BUTTON_A PA15
  #define BUTTON_B PC7
  #define BUTTON_C PC5
#elif defined(TEENSYDUINO)
  #define BUTTON_A  4
  #define BUTTON_B  3
  #define BUTTON_C  8
#elif defined(ARDUINO_NRF52832_FEATHER)
  #define BUTTON_A 31
  #define BUTTON_B 30
  #define BUTTON_C 27
#else // 32u4, M0, M4, nrf52840, esp32-s2 and 328p
  #define BUTTON_A  9
  #define BUTTON_B  6
  #define BUTTON_C  5
#endif

const char* ssid = "Telia-B7F3CC";
const char* password = "011235813";

WiFiClient client;

struct parameters {
float deposit;
float weight;
float stackvalue;
float depositPotential;
};

struct parameters data;

void getData() {
  if (client.connect("192.168.1.64", PORT)) {
    Serial.println("Connected to server");

    if (client.readBytes((char*)&data, sizeof(data)) == sizeof(data)) {
        Serial.print("Received a: ");
        Serial.println(data.deposit);
        Serial.print("Received b: ");
        Serial.println(data.weight);
        Serial.print("Received c: ");
        Serial.println(data.stackvalue);
        Serial.print("Received d: ");
        Serial.println(data.depositPotential);
    } else {
        Serial.println("Failed to receive struct");
    }

    client.stop(); // Close the connection after receiving data
  } else {
    Serial.println("Connection to server failed");
  }
}

void setup() {
  Serial.begin(115200);

  Serial.println("128x64 OLED FeatherWing test");
  delay(250); // wait for the OLED to power up
  display.begin(0x3C, true); // Address 0x3C default

  Serial.println("OLED begun");

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(1000);

  // Clear the buffer.
  display.clearDisplay();
  display.display();

  display.setRotation(1);
  Serial.println("Button test");

  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);

  // text display tests
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setFont(&FreeSerif12pt7b);
  display.setCursor(29, 39);
 
   WiFi.begin(ssid, password);
  
   while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    display.print("C");
   }
  display.clearDisplay();
  display.display();
  display.drawCircle(64, 32, 30, 1);
  display.setCursor(29, 39);
  display.print("GOLD");
}

void loop() {
  if(!digitalRead(BUTTON_A)) {
    display.clearDisplay();
    getData();
    display.setFont(nullptr);
  
    display.setCursor(0, 5);
    display.print("Stack Value");
    display.setCursor(0, 24);
    display.setFont(&FreeSerifBold12pt7b);
    
    display.print(data.stackvalue);
    
    display.setFont(&FreeMono9pt7b);
    display.setCursor(0, 42);
    display.print(data.weight);
    display.print(" grams");
    display.setFont(nullptr);
    display.setCursor(0, 45);
    display.print("Stack Weight");
    
  }
  if(!digitalRead(BUTTON_B)){
    display.clearDisplay();
    display.setFont(nullptr);
    display.setCursor(0, 5);
    display.print(data.deposit);
    display.setFont(&FreeSerifBold12pt7b);
    display.setCursor(0, 32);
    display.print(data.depositPotential);
    display.setCursor(0, 42);
    display.setFont(nullptr);
    display.print("Gold the deposit may buy");
  }
  if(!digitalRead(BUTTON_C)) {
    display.clearDisplay();
    display.setFont(&FreeMono9pt7b);
    display.setCursor(0, 10);
    display.print("Page C");
  }
  delay(10);
  yield();
  display.display();
}
