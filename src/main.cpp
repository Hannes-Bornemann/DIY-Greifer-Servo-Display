
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

    // negative Drehung: (öffnen)
    // 10%  Servo 0: 1400
    //      Servo 1: 1400 (geändert, sonst zu schnell!)
    //      Servo 2: 1750
    //      Servo 3: 1380

    // positive Drehung: (schliessen)
    // 10%                                                          Versagt?
    // myservo0.writeMicroseconds(1600); // servo schliessen +100   Nein
    // myservo1.writeMicroseconds(1590);                     +40    Nein
    // myservo2.writeMicroseconds(1290);                     -30    Nein
    // myservo3.writeMicroseconds(1620);                     +70    Nein
    // 20%
    // myservo0.writeMicroseconds(1700); // servo schliessen        Nein
    // myservo1.writeMicroseconds(1630);                            Nein
    // myservo2.writeMicroseconds(1260);                            Nein
    // myservo3.writeMicroseconds(1690);                            Nein
    // 30%
    // myservo0.writeMicroseconds(1800); // servo schliessen        Nein
    // myservo1.writeMicroseconds(1670);                            Versagt nach ca. 100 Zyklen (überhitzt)
    // myservo2.writeMicroseconds(1230);                            Nein
    // myservo3.writeMicroseconds(1760);                            Nein
    // 40%
    // myservo0.writeMicroseconds(1900); // servo schliessen        Versagt nach ca. 65 Zyklen (Zahnrad am Motor hat sich gelöst)
    //
    // myservo2.writeMicroseconds(1200);                            Nein
    // myservo3.writeMicroseconds(1830);                            Versagt nach ca. 140 Zyklen (Servo wird laut, bewegt sich nicht mehr zurück)
    // 50%
    //                                   // servo schliessen
    // myservo2.writeMicroseconds(1170);                            Versagt nach ca. 70 Zyklen (Servo bleibt stehen)
    //
    //

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
void DisplayVergleich(int perc, int closing, int servo0, int servo1, int servo2, int servo3)
{
    // clear the display
    display.clear();
    display.setFont(ArialMT_Plain_16);
    // Text
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 0, "Power (%): " + String(perc));
    display.drawString(0, 16, "closing " + String(closing) + "/500");
    display.drawString(0, 32, "Time[min]:" + String(minutes));
    display.setFont(ArialMT_Plain_10); // Reduce text size
    display.drawString(0, 50, String(servo0) + "|" + String(servo1) + "|" + String(servo2) + "|" + String(servo3));

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
        int percentage = 50;
        microsec = millis();
        minutes = microsec / 60000;

        int servo0value = 1500 + 0.1 * percentage * 100;
        int servo1value = 1550 + 0.1 * percentage * 40;
        int servo2value = 1320 - 0.1 * percentage * 30;
        int servo3value = 1550 + 0.1 * percentage * 70;

        DisplayVergleich(percentage, i, servo0value, servo1value, servo2value, servo3value);

        myservo0.writeMicroseconds(1400); // servo öffnen
        myservo1.writeMicroseconds(1400);
        myservo2.writeMicroseconds(1750);
        myservo3.writeMicroseconds(1380);
        delay(500);

        myservo0.writeMicroseconds(1500); // servo stromlos
        myservo1.writeMicroseconds(1500);
        myservo2.writeMicroseconds(1500);
        myservo3.writeMicroseconds(1500);
        delay(500);

        myservo0.writeMicroseconds(servo0value); // servo schliessen
        myservo1.writeMicroseconds(servo1value);
        myservo2.writeMicroseconds(servo2value);
        myservo3.writeMicroseconds(servo3value);
        delay(10000);
    }
    while (true) // Program stops
    {
        myservo0.writeMicroseconds(1500);
        myservo1.writeMicroseconds(1500);
        myservo2.writeMicroseconds(1500);
        myservo3.writeMicroseconds(1500);
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
