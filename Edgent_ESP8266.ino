// Fill-in information from your Blynk Template here
#define BLYNK_TEMPLATE_ID "TMPL3L2_1ojPE"
#define BLYNK_TEMPLATE_NAME "GAS SMOKE"
//#define BLYNK_AUTH_TOKEN "yEazvLGpwnjMBuPhfiZiZMHtXe4oQ9r6"
#define BLYNK_DEVICE_NAME "GAS SMOKE"
/*
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "REDMI 9";
char pass[] = "password";*/

#define BLYNK_FIRMWARE_VERSION "0.1.0"

#define BLYNK_PRINT Serial
#define BLYNK_DEBUG

#define APP_DEBUG
// Uncomment your board, or configure a custom board in Settings.h
#define USE_NODE_MCU_BOARD
#include "HX711.h"
#include "BlynkEdgent.h"
#include <MQ2.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#include <Servo.h>
Servo myservo;
const int LOADCELL_DOUT_PIN = 0;//D3
const int LOADCELL_SCK_PIN = 2;//D4
const float EMPTY_WEIGHT = 0.5;  // Weight of empty gas cylinder in kg
const float FULL_WEIGHT = 1.0;  // Weight of full gas cylinder in kg
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#define BUZZ      12 //D6 
#define LED       14 //D5
#define air       13 //D7
HX711 scale;
//change this with the pin that you use
int pin = A0;
float lpg, smoke;
MQ2 mq2(pin);
int button1 = 0;
int button2 = 0;
//SimpleTimer timer;
float level,weight;
void setup(){
  Serial.begin(9600);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(2280.f);  // Calibrate the scale to your specific load cell
  scale.tare();  // Reset the scale to zero
  //Serial.begin(9600);
  delay(100);
  BlynkEdgent.begin();
  pinMode(BUZZ, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(air,OUTPUT);
  myservo.attach(D8);
  digitalWrite(BUZZ, LOW);
  digitalWrite(LED, LOW);
  digitalWrite(air, LOW);  
  // calibrate the device
  mq2.begin();
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  delay(1000);
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0, 0);
  display.println(" IoT Smoke ");
  display.setCursor(0, 20);
  display.println("  Detector  ");
  display.display();
  delay(500);
  timer.setInterval(500L, sendSensorData);
}
void loop() {
  float weight = scale.get_units();
  level = (weight - EMPTY_WEIGHT) / (FULL_WEIGHT - EMPTY_WEIGHT);
  Serial.print("Weight: ");
  Serial.print(weight);
  Serial.print(" kg, Level: ");
  Serial.println(level);
  delay(1000);
  timer.run(); // Initiates SimpleTimer
  BlynkEdgent.run();
}
void sendSensorData()
{
  float* values = mq2.read(true); //set it false if you don't want to print the values to the Serial
  //co = mq2.readCO();
  smoke = mq2.readSmoke();
  lpg = mq2.readLPG();
  if (button1 == 1 && button2==0 )
  {
    // display LPG
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.print("    LPG   ");
    display.setCursor(10, 30);
    display.print(lpg);
    display.setTextSize(1);
    display.print(" PPM");
    delay(5000);
    display.display();
  }
  else if (button2 == 1 && button1==0)
  {
    // display Smoke
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.print("   SMOKE   ");
    display.setCursor(10, 30);
    display.print(smoke);
    display.setTextSize(1);
    display.print(" PPM");
    delay(5000);
    display.display();
  }
  else{
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0, 0);
  display.println(" IoT Smoke ");
  display.setCursor(0, 20);
  display.println("  Detector  ");
  display.display();
  }
  Blynk.virtualWrite(V1, smoke);
  Blynk.virtualWrite(V2, lpg);
  Blynk.virtualWrite(V3, level);
  if (smoke > 50 ) {
    Blynk.logEvent("smoke");
    digitalWrite(BUZZ, HIGH);
    digitalWrite(LED, HIGH);
    digitalWrite(air, HIGH);    
  }
  else if(lpg>20){
    Blynk.logEvent("lpg");
    digitalWrite(BUZZ, HIGH);
    digitalWrite(LED, HIGH);
    myservo.write(180);    
  }
  else if(level<=0.5){
    Blynk.logEvent("gas level");
    digitalWrite(BUZZ, HIGH);
    digitalWrite(LED, HIGH);
  }
  else {
    digitalWrite(BUZZ, LOW);
    digitalWrite(LED, LOW);
    digitalWrite(air, LOW);
    myservo.write(0);
  }
}
// in Blynk app writes values to the Virtual Pin 4
BLYNK_WRITE(V4)
{
  button1 = param.asInt(); // assigning incoming value from pin V4 to a variable
}
// in Blynk app writes values to the Virtual Pin 5
BLYNK_WRITE(V5)
{
  button2 = param.asInt(); // assigning incoming value from pin V5 to a variable
}