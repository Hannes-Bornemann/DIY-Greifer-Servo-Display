
#include <ESP32Servo.h>
#include <Arduino.h>
#include <Wire.h>
#include "SSD1306Wire.h"

Servo myservo0; // create servo object to control a servo
Servo myservo1;
Servo myservo2;
Servo myservo3;

// int pos = 0; // servo position (0=open , 1=closed)

int servo0Pin = 2;
int servo1Pin = 4;
int servo2Pin = 18;
int servo3Pin = 19;

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
    myservo0.attach(servo0Pin, 500, 2500);
    myservo1.attach(servo1Pin, 500, 2500);
    myservo2.attach(servo2Pin, 500, 2500);
    myservo3.attach(servo3Pin, 500, 2500);
    // "different servos may require different min/max settings for an accurate 0 to 180 sweep"
    // min/max für alle Servos gleich gelassen, nur aufgerufenen Wert verändert:
    // myservo.writeMicroseconds(X);
    // Servo 0: positive Drehung: 1500 <= X <= 2500 (1500 langsamste) Range: 1000
    //          negative Drehung: 500  <= X <= 1500 (1500 langsamste)
    // Servo 1: positive Drehung: 1550 <= X <= 1950 (1550 langsamste) Range: 400
    //          negative Drehung: 1000 <= X <= 1400 (1400 langsamste)
    // Servo 2: positive Drehung: 1020 <= X <= 1320 (1320 langsamste) Range: 300
    //          negative Drehung: 1720 <= X <= 2020 (1720 langsamste)
    // Servo 3: positive Drehung: 1550 <= X <= 2250 (1540 langsamste) Range: 700
    //          negative Drehung: 750  <= X <= 1450 (1450 langsamste)
    // positive Drehung:
    // 10%   Servo 0: 1600
    //      Servo 1: 1590
    //      Servo 2: 1290
    //      Servo 3: 1620
    // 20%   Servo 0:
    //      Servo 1:
    //      Servo 2:
    //      Servo 3:
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
void DisplayVergleich(int perc, int closing)
{
    // clear the display
    display.clear();
    display.setFont(ArialMT_Plain_16);
    // Text
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 0, "Power (%): ");
    // display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.print(perc);

    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 16, "closings of 500: ");
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.drawString(0, 0, String(closing));

    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 48, "Time[min]:");
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.drawString(0, 48, String(minutes));

    // write the buffer to the display
    display.display();
}

// int getMicrosec(int servotyp, float percentage)
// {
//     int minimum;
//     int maximum;
//     if (servotyp == 0)
//     {
//         minimum = min0;
//         maximum = max0;
//     }
//     if (servotyp == 1)
//     {
//         minimum = min1;
//         maximum = max1;
//     }
//     if (servotyp == 2)
//     {
//         minimum = min2;
//         maximum = max2;
//     }
//     int range = (maximum - minimum) / 2;
//     int microsec = minimum + range + percentage / 100.0f * range;
//     return microsec;
// }

// void testRange()
// {
//     for (int i = 0; i < max0; i++)
//     {
//         myservo0.writeMicroseconds(i * 100);
//         Serial.println(i * 100);
//     }
//     delay(1000);
// }

void verschleisstest()
{
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

void Vergleichstest()
{
    for (int i = 0; i <= 500; i++)
    {
        microsec = millis();
        minutes = microsec / 60000;
        DisplayVergleich(10, i);

        myservo0.writeMicroseconds(1500); // servo stromlos
        myservo1.writeMicroseconds(1500);
        myservo2.writeMicroseconds(1500);
        myservo3.writeMicroseconds(1500);
        delay(1000);
        myservo0.writeMicroseconds(1600);
        myservo1.writeMicroseconds(1590);
        myservo2.writeMicroseconds(1290);
        myservo3.writeMicroseconds(1620);
        delay(1000);
    }
}
void loop()
{
    Vergleichstest();
    /*
    DrawStateDisplay();
    gripperClose();
    closings++;
    delay(1000);
    gripperOpen();
    delay(1000);
    */

    // buttonPinTest
    // DrawStateDisplay();
    // buttonState = digitalRead(buttonPin);
    // while (buttonState == 0) // Taster nicht gedrückt
    // {
    //     myservo0.writeMicroseconds(1500); // servo stromlos
    //     myservo1.writeMicroseconds(1500);
    //     myservo2.writeMicroseconds(1500);
    //     myservo3.writeMicroseconds(1500);
    //     Serial.println("open");
    //     buttonState = digitalRead(buttonPin);
    // }
    // delay(1);                // sonst wird folgende while schleife iwie einmal übersprungen
    // while (buttonState != 0) // Taster gedrückt
    // {
    //     myservo0.writeMicroseconds(1600);
    //     myservo1.writeMicroseconds(1590);
    //     myservo2.writeMicroseconds(1290);
    //     myservo3.writeMicroseconds(1620);

    //     Serial.println("close");
    //     // Serial.println(setMicrosec);
    //     buttonState = digitalRead(buttonPin);
    // }
    // myservo0.writeMicroseconds(1800);
    // myservo1.writeMicroseconds(1800);
    // delay(500);
}
