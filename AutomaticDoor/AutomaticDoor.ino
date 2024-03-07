#include <Stepper.h>
#include <Bridge.h>
#include <BridgeServer.h>
#include <BridgeClient.h>

const int SECONDS_BEFORE_CLOSING = 50;
const int STEPS_PER_REVOLUTION = 2038;
const int LED = 2;
const int SENSOR = 2;

bool shouldOpen = true;
bool shouldClose = false;
bool didOpen = false;
bool didClose = false;

int openTicks = 0;

BridgeServer server;

// https://arduino.stackexchange.com/a/70257
// In order for the motor to be able to go in the opposite direction, we have to reverse the port mapping for
// IN2 (typically 9) and IN3 (typically 10).
Stepper myStepper(STEPS_PER_REVOLUTION, 8, 10, 9, 11);

void setup() {
  Serial.begin(9600);
  
  pinMode(SENSOR, INPUT);
  
  myStepper.setSpeed(5);
  
  // Setup the web server
  Bridge.begin();
  server.listenOnLocalhost();
  server.begin();
}

void loop() {
  // doorLoop();
  serverLoop();
  //delay(100);
}

void serverLoop() {
  BridgeClient client = server.accept();
  if (!client) {
    Serial.println("No client attempting connection.");
    return;
  }

  Serial.println("accepting connection");
  client.print("<h1>test</h1>");
  client.stop();
  delay(50);
}

void doorLoop() {
  if (shouldOpen) {
    Serial.println("Opening door");
    myStepper.step(STEPS_PER_REVOLUTION * 0.25);
    shouldOpen = false;
    didOpen = true;
    didClose = false;
  }

  if (shouldClose) {
    Serial.println("Closing door");

    myStepper.step(-(STEPS_PER_REVOLUTION * 0.25));
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

