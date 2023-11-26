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
; // default: SCL : GPIO22; SDA: GPIO21

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
    // myservo0.write(0); //  range 0 - 180
    // myservo1.write(0);

    myservo0.writeMicroseconds(1000); //  range:  500-1500 (negative turning direction, 500 = maximum torque, 1500 = torque 0)
    myservo1.writeMicroseconds(1000); //          1500-2500 (positive turning direction, 1500 = torque 0, 2500 = maximum torque)
    delay(500);

    myservo0.writeMicroseconds(1500); // turn off servo
    myservo1.writeMicroseconds(1500);
    // delay(500);
    Serial.println("open");
}

void gripperClose()
{
    // myservo0.write(180);
    // myservo1.write(180);

    myservo0.writeMicroseconds(2000);
    myservo1.writeMicroseconds(2000);
}

void DrawStateDisplay()
{
    // clear the display
    display.clear();
    microsec = millis();
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
    minutes = microsec / 60000;
    /*
    DrawStateDisplay();
    gripperClose();
    closings++;
    delay(1000);
    gripperOpen();
    delay(1000);
    */
    /*
    // buttonPinTests
    buttonState = digitalRead(buttonPin);
    if (buttonState == 1)
    {
        gripperClose();
    }
    else
    {
        gripperOpen();
    }
    */

    // Verschleißtest
    Serial.print("closings: ");
    Serial.print(closings);
    Serial.print("  failures: ");
    Serial.println(failures);

    delay(500);
    buttonState = digitalRead(buttonPin);
    Serial.print("buttonState = ");
    Serial.println(buttonState);

    DrawStateDisplay();

    if (buttonState != gripperState)
    {
        failures++;
        Serial.print("New failure");
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
        Serial.print("in buttonState == 1");
        gripperOpen(); // Greifer öffnen
        gripperState = 0;
    }

    if (buttonState == 0) // Knopf nicht gedrückt, Greifer offen
    {
        Serial.print("in buttonState == 0!!!");
        gripperClose(); // Greifer schließen
        gripperState = 1;
        closings++;

        delay(15000); // closing time
        // new features number 2
        // new features number 3
    }
    // new feature for git check
}
