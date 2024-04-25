#include <Stepper.h>
#include <Bridge.h>

#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL4dmV73TW7"
#define BLYNK_TEMPLATE_NAME "AutomaticDoor"
#define BLYNK_AUTH_TOKEN "KT67LoCd_HdfaT82M7-uFjzYSkazSO7H"

#include <BlynkSimpleYun.h>

const int SECONDS_BEFORE_CLOSING = 50;
const int STEPS_PER_REVOLUTION = 2038;
const int LED = 2;
const int SENSOR = 2;

bool enabled = true;
bool shouldOpen = true;
bool shouldClose = false;
bool didOpen = false;
bool didClose = false;

int openTicks = 0;

// https://arduino.stackexchange.com/a/70257
// In order for the motor to be able to go in the opposite direction, we have to reverse the port mapping for
// IN2 (typically 9) and IN3 (typically 10).
Stepper stepper(STEPS_PER_REVOLUTION, 8, 10, 9, 11);

// V0 is used for enabling/disabling the lock functionality.
BLYNK_WRITE(V0)
{
  int pinValue = param.asInt();
  enabled = pinValue == 1;
}

void setup() {
  Serial.begin(9600);
  
  pinMode(SENSOR, INPUT);
  
  stepper.setSpeed(5);
  
  // Setup the web server
  Bridge.begin();
  Blynk.begin(BLYNK_AUTH_TOKEN);
}

void loop() {
  Blynk.run();
  doorLoop();
  delay(100);
}

void doorLoop() {
  if (!enabled) {
    return;
  }

  if (shouldOpen) {
    Serial.println("Opening door");
    Blynk.virtualWrite(V1, 1);
    stepper.step(STEPS_PER_REVOLUTION * 0.25);
    shouldOpen = false;
    didOpen = true;
    didClose = false;
  }

  if (shouldClose) {
    Serial.println("Closing door");

    stepper.step(-(STEPS_PER_REVOLUTION * 0.25));
    shouldClose = false;
    didClose = true;
    didOpen = false;
  }
  
  int value = digitalRead(SENSOR);
  if (value == 0) {
    Serial.println("No light, door should open");

    if (!didOpen) {
      shouldOpen = true;
    }
  } else {
      Serial.println("Light, door should close");

      if (!didClose) {
          openTicks++;
          if (openTicks >= SECONDS_BEFORE_CLOSING) {
              shouldClose = true;
              openTicks = 0;
          }
      }
  }
}

