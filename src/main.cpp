
#include <ESP32Servo.h>
#include <Arduino.h>
#include <Wire.h>
#include "SSD1306Wire.h"

Servo myservo0; // create servo object to control a servo
Servo myservo1;

// int pos = 0; // servo position (0=open , 1=closed)

int servo0Pin = 18;
int servo1Pin = 19;
int buttonPin = 12;
int buttonState = 0;  //  0=unpressed
int gripperState = 0; //  0=open
int closings = 0;     //  number of closings since start
int failures = 0;     //  number of failures since start
long microsec = millis();
int minutes;

// Initialisieren des Displays
SSD1306Wire display(0x3c, SDA, SCL);
// default: SCL : GPIO22; SDA: GPIO21

void setup()
{
    Serial.begin(115200);

    // Initalisieren des Display
    display.init();
    display.flipScreenVertically();

    // Allow allocation of all timers
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);
    myservo0.setPeriodHertz(50); // standard 50 hz servo
    myservo1.setPeriodHertz(50);
    myservo0.attach(servo0Pin, 500, 2500); // using min/max of 500us and 2500us
                                           // different servos may require different min/max settings
                                           // for an accurate 0 to 180 sweep
    myservo1.attach(servo1Pin, 500, 2500);
    pinMode(buttonPin, INPUT_PULLDOWN);
}

void gripperOpen()
{
    myservo0.writeMicroseconds(1800); //  range:  500-1500 (negative turning direction, 500 = maximum torque, 1500 = torque 0)
    myservo1.writeMicroseconds(1800); //          1500-2500 (positive turning direction, 1500 = torque 0, 2500 = maximum torque)
    delay(500);
    myservo0.writeMicroseconds(1500); // turn off servo
    myservo1.writeMicroseconds(1500);
    // delay(500);
    Serial.println("open");
}

void gripperClose()
{
    myservo0.writeMicroseconds(1245);
    myservo1.writeMicroseconds(1245);
}

void DrawStateDisplay()
{
    // clear the display
    display.clear();
    // Text
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 0, "Schliessungen:");
    display.drawString(0, 16, String(closings));
    display.drawString(0, 32, "Fehler:");
    display.drawString(60, 32, String(failures));
    display.drawString(0, 48, "Time[min]:");
    display.drawString(75, 48, String(minutes));
    // write the buffer to the display
    display.display();
}

void loop()
{
    /*
    DrawStateDisplay();
    gripperClose();
    closings++;
    delay(1000);
    gripperOpen();
    delay(1000);
    */
    /*
    // buttonPinTest
    DrawStateDisplay();
    buttonState = digitalRead(buttonPin);
    while (buttonState == 0) // Taster nicht gedrückt
    {
        myservo0.writeMicroseconds(1500); // servo stromlos
        myservo1.writeMicroseconds(1500);
        Serial.println("open");
        buttonState = digitalRead(buttonPin);
    }
    delay(1);                // sonst wird folgende while schleife iwie einmal übersprungen
    while (buttonState != 0) // Taster gedrückt
    {
        myservo0.writeMicroseconds(1020); // servo öffnet 1720 bis 2020=höchste Kraft (300 range) 50% bei 1170, dann 1A Stromverbrauch
        myservo1.writeMicroseconds(1020);   // servo schließt 1020 bis 1320=niedrigste (300 range)
        Serial.println("close");
        buttonState = digitalRead(buttonPin);
    }
    myservo0.writeMicroseconds(1800);
    myservo1.writeMicroseconds(1800);
    delay(500);
    */

    // Verschleißtest
    microsec = millis();
    minutes = microsec / 60000;
    buttonState = digitalRead(buttonPin);
    DrawStateDisplay();
    if (buttonState != gripperState)
    {
        failures++;
        if (failures > 2)
        {
            Serial.print("End Program ");
            gripperOpen();
            while (true) // Program stops at 3 failures
            {
                myservo0.writeMicroseconds(1500);
                myservo1.writeMicroseconds(1500);
            }
        }
    }
    delay(500);

    if (buttonState == 1) // Knopf gedrückt, Greifer geschlossen
    {
        gripperOpen();
        gripperState = 0;
    }

    if (buttonState == 0) // Knopf nicht gedrückt, Greifer offen
    {
        gripperClose();
        gripperState = 1;
        closings++;
        delay(10000); // closing time
    }
}
