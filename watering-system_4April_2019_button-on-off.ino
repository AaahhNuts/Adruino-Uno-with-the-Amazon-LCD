/*
 * TimeRTCSet.pde
 * example code illustrating Time library with Real Time Clock.
 *
 * RTC clock is set in response to serial port time message 
 * A Processing example sketch to set the time is included in the download
 * On Linux, you can use "date +T%s > /dev/ttyACM0" (UTC time zone)
 * 
 * Added the config area below.
 * Added a water putton on/off.
 * 
 * Jon Stephenson 
 * 
 * jon.a.stephenson at gmail dot com
 * 
 */

#include <SPI.h>        // RC522 Module uses SPI protocol
#include <Wire.h>  // Comes with Arduino IDE
#include "RTClib.h"


//  Edit below.......

// How many days to water ?  Change to match the watering days.
int weekDayW = 7;

//  Remove the days you do NOT want to water. Remove the "xxxxxx",  for each day
char wateringDays[7][10] = {"Sunday", "Monday", "Tuesday", "Wednewday", "Thursday", "Friday", "Saterday"};


// How many times a day to water ?  Change to match the watering hours.
int dayHourW = 2;

// Add/Remove/Change the hours of the day you do NOT want to water.  Remove or Add the "(day time)",  for each hour.
char wateringHours[2][3] = {"6","20"};  // example water at 06:00 and 20:00 hours

// How long to water for each time
int wateringTime = 5;  // how long in minutes, change as required  Tested 20 min - all ok

// STOP editing here .......


#define  relay        6     // Set Relay Pin
#define  button       5     // Water button
#define  wateringLED  4     // Watering LED

RTC_DS1307 RTC;

const char dayHour = 24;
char hoursOfTheDay[24][3] = {"0","1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16","17","18","19","20","21","22","23"};

const char weekDay = 7;
char daysOfTheWeek[7][10] = {"Sunday", "Monday", "Tuesday", "Wednewday", "Thursday", "Friday", "Saterday"};

unsigned long wateringCountdown = 0;  // Count down the watering time
int lasthourWatered = -1;  // Have we watered this hour
int dayNum;
int hourNum;
bool watering = false;
bool firstTime = true;
//bool debug = true;  // USE PuTTY as monitor COM5/6/12 whatever the nano is on, 9600 underline Cursor
bool debug = false;  // USE PuTTY as monitor COM5/6/12 whatever the nano is on, 9600 underline Cursor

unsigned long realwateringTime = (wateringTime * 60000);

// Variables will change:
int ledState = HIGH;         // the current state of the output pin
int buttonState = HIGH;      // the current reading from the input pin
int lastButtonState = HIGH;  // the previous reading from the input pin

// the following variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 50;    // the debounce time; increase if the output flickers


void setup()  {
  Serial.begin(9600);
  pinMode(relay, OUTPUT); 
  pinMode(wateringLED, OUTPUT);       
  pinMode(button, INPUT);        
  digitalWrite(relay, HIGH);   // Make sure the water valve is closed 
  digitalWrite(wateringLED, LOW);  // turn it off
  RTC.begin();
  Wire.begin();

//  if (!RTC.isrunning()) {
    RTC.adjust(DateTime(__DATE__, __TIME__));  // set time via usb
//  }
  if (debug) {
    DateTime now = RTC.now();
    Serial.print("Setup begin: ");
    Serial.print(now.hour(), DEC);
    Serial.print(":");
    Serial.print(now.minute(), DEC);
    Serial.print(":");
    Serial.println(now.second(), DEC);
  }
}


void loop() {
  checkWatering();
  if (debug) {
    Serial.print("buttonState: ");
    Serial.print(buttonState);  // Active LOW
    Serial.print("  watering: ");
    Serial.print(watering);
    Serial.print("  lastbuttonstate: ");
    Serial.print(lastButtonState);
  }

  checkButton();

    if ((buttonState == LOW) && (watering == true) && (lastButtonState == true)) {  // Turn watering OFF with the button (after it has been pressed)
      digitalWrite(relay, HIGH);    // Close the watering relay
      digitalWrite(wateringLED, LOW);  // turn off led
      if (debug) {
        DateTime now = RTC.now();
        Serial.print("\n\r");
        Serial.print("Button Watering Cancelled at:  ");
        Serial.print(now.hour(), DEC);
        Serial.print(":");
        Serial.print(now.minute(), DEC);
        Serial.print(":");
        Serial.print(now.second(), DEC);
        Serial.println("        ");
      }
      watering = false;
      wateringCountdown = 0;
      delay(200);
  } else
  
  if ((buttonState == LOW) && (watering == false) && (lastButtonState == true)) {  // Turn watering ON when the button (if not watering)
      digitalWrite(wateringLED, HIGH);
      digitalWrite(relay, LOW);     // Open the watering relay
      if (debug) {
        DateTime now = RTC.now();
        Serial.print("\n\r");
        Serial.print("Button Watering Started at:  ");
        Serial.print(now.hour(), DEC);
        Serial.print(":");
        Serial.print(now.minute(), DEC);
        Serial.print(":");
        Serial.print(now.second(), DEC);
        Serial.println("        ");
      }
      watering = true;
      wateringCountdown = wateringCountdown + 200;
      delay(200);
  } 

  
  delay(200);
  if (watering) {
    wateringCountdown = wateringCountdown + 200;
  }
  if (wateringCountdown >= realwateringTime) {  // Button watering time is done
      digitalWrite(relay, HIGH);    // Close the watering relay
      digitalWrite(wateringLED, LOW);  // turn OFF the led
      wateringCountdown = 0;  // reset the counter
      if (debug) {
        DateTime now = RTC.now();
        Serial.print("\n\r");
        Serial.print("Button Watering Stopped at:  ");
        Serial.print(now.hour(), DEC);
        Serial.print(":");
        Serial.print(now.minute(), DEC);
        Serial.print(":");
        Serial.print(now.second(), DEC);
        Serial.println("        ");
      }
      watering = false;  // Flag Not watering
  }
}

void checkWatering() {
  DateTime now = RTC.now();
  if (debug) {
    Serial.print("  Time: ");
    Serial.print(now.hour(), DEC);
    Serial.print(":");
    Serial.print(now.minute(), DEC);
    Serial.print(":");
    Serial.print(now.second(), DEC);
    Serial.print("        ");
    Serial.print("\r");
  }

  if (now.hour() == 0) {  // Just incase someone wants to water at midnight !!
    lasthourWatered = -1;
  }
  if (now.hour() == 1) {  // Reset the firstTime flag at 1 am
    firstTime = true;
  }
    if ((lasthourWatered < now.hour()) && (firstTime)) {  // New hour and it's not zero hours again
      for (dayNum = 0; dayNum < (weekDayW); dayNum++ ) {  
        if (strcmp(wateringDays[dayNum],daysOfTheWeek[now.dayOfTheWeek()])==0){  //  Do we water this day ?
           for (hourNum = 0; hourNum < (dayHourW); hourNum++ ) {
              if (strcmp(wateringHours[hourNum],hoursOfTheDay[now.hour()])==0){  //  Do we water this hour ?
                  if (debug) {
                    Serial.print("\n\r");
                    Serial.print("Timed Watering Started at:  ");
                    Serial.print(now.hour(), DEC);
                    Serial.print(":");
                    Serial.print(now.minute(), DEC);
                    Serial.print(":");
                    Serial.print(now.second(), DEC);
                    Serial.println("        ");
                    Serial.println("Watering........        ");
                  }
                  startWatering();    // Turn on the watering relay for the watering time
                  watering = false;
                  lasthourWatered = now.hour();
               }
           }
        }
     }
  }

  if ((now.hour() == 0) && (firstTime)){  // Let it water once at zero hours then not again till 1 hour
    firstTime = false;
  }
}


void checkButton() {
  // read the state of the switch into a local variable:
  int reading = digitalRead(button);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH),  and you've waited
  // long enough since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;
    }
  }

  // save the reading.  Next time through the loop,
  // it'll be the lastButtonState:
  lastButtonState = reading;
}


void startWatering(){
      digitalWrite(relay, LOW);    // Close the watering relay
      digitalWrite(wateringLED, HIGH);
      delay(realwateringTime);
      if (debug) {
          DateTime now = RTC.now();
          Serial.print("Timed Watering Ended at:  ");
          Serial.print(now.hour(), DEC);
          Serial.print(":");
          Serial.print(now.minute(), DEC);
          Serial.print(":");
          Serial.print(now.second(), DEC);
          Serial.println("        ");
      }
      digitalWrite(relay, HIGH);    // Close the watering relay
      digitalWrite(wateringLED, LOW);  // turn off led
 }


/*  code to process time sync messages from the serial port   */
#define TIME_HEADER  "T"   // Header tag for serial time sync message

unsigned long processSyncMessage() {
  unsigned long pctime = 0L;
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013 

  if(Serial.find(TIME_HEADER)) {
     pctime = Serial.parseInt();
     return pctime;
     if( pctime < DEFAULT_TIME) { // check the value is a valid time (greater than Jan 1 2013)
       pctime = 0L; // return 0 to indicate that the time is not valid
     }
  }
  return pctime;
}





