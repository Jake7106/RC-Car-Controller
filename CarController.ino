#include <IRremote.h>

// Pin Declaration
const int IR_RECEIVE_PIN = 12;

const int lineTrackPin = 2;

const int trigPin = 3;
const int echoPin = 4;

const int A_1B = 5;
const int A_1A = 6;

const int rightIR = 7;
const int leftIR = 8;

const int B_1B = 9;
const int B_1A = 10;

int speed = 150;

// Car Commands
enum Command {
  ERROR_CMD, ACCELERATE, DECELERATE, FORWARDLEFT, FORWARD, FORWARDRIGHT,
  LEFT, RIGHT, BACKWARDLEFT, BACKWARD, BACKWARDRIGHT, LINETRACK, SELFDRIVE,
  OBS_AVOID_US, OBS_AVOID_IR, HANDFOLLOW, STOP
};

// Car Modes
enum Mode {
  M_NONE, M_AUTO, M_LINE, M_ULTR, M_IROB, M_FOLLOW
};

Mode mode = M_NONE;

// Main Setup Function
void setup() {
  Serial.begin(9600);

  // Motors
  pinMode(A_1B, OUTPUT);
  pinMode(A_1A, OUTPUT);
  pinMode(B_1B, OUTPUT);
  pinMode(B_1A, OUTPUT);

  // IR Sensors
  pinMode(leftIR, INPUT);
  pinMode(rightIR, INPUT);

  // Ultrasonic
  pinMode(echoPin, INPUT);
  pinMode(trigPin, OUTPUT);

  // Line Track
  pinMode(lineTrackPin, INPUT);

  // Remote
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
  Serial.println("REMOTE CONTROL START");
}

// Main Loop Function
void loop() {
  if (IrReceiver.decode())
  {
    // Checks which button was pressed on remote
    Command cmd = decodeKeyValue(IrReceiver.decodedIRData.command);

    // Finds the associated command with the given button press
    switch (cmd)
    {
      case ACCELERATE:
        speed += 50;
        break;
      
      case DECELERATE:
        speed -= 50;
        break;

      case FORWARDLEFT:
        moveLeft(speed);
        break;

      case FORWARD:
        moveForward(speed);
        delay(1000);
        break;

      case FORWARDRIGHT:
        moveRight(speed);
        break;

      case LEFT:
        turnLeft(speed);
        break;

      case RIGHT:
        turnRight(speed);
        break;

      case BACKWARDLEFT:
        backLeft(speed);
        break;

      case BACKWARD:
        moveBackward(speed);
        delay(1000);
        break;

      case BACKWARDRIGHT:
        backRight(speed);
        break;

      case STOP:
        mode = M_NONE;
        stopMove();
        break;

      case LINETRACK:
        mode = M_LINE;
        break;

      case SELFDRIVE:
        mode = M_AUTO;
        break;

      case OBS_AVOID_US:
        mode = M_ULTR;
        break;

      case OBS_AVOID_IR:
        mode = M_IROB;
        break;

      case HANDFOLLOW:
        mode = M_FOLLOW;
        break;

      case ERROR_CMD:
        default:
          break;
    }

    // Clamp Speed
    if (speed >= 255) speed = 255;
    if (speed <= 0) speed = 0;
    
    delay(500);
    stopMove();

    IrReceiver.resume();
  }

  // Switches the mode in which the car operates depending on if a mode switch call is
  // recieved from the ir remote
  switch(mode)
  {
    case M_AUTO:
      AutoDrive(speed);
      break;

    case M_LINE:
      LineTrack(speed);
      break;

    case M_ULTR:
      UltrasonicDrive(speed);
      break;

    case M_IROB:
      IrObstacleDrive(speed);
      break;

    case M_FOLLOW:
      Following(speed);
      break;

    case M_NONE:
      default:
        break;
  }
}

// Reads data from ultrasonic module
float ReadSensorData()
{
  // Ensure previous pulse has finished
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Sends trigger pulse and waits for return
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Measures distance by how long it took for pulse return
  float distance = pulseIn(echoPin, HIGH) / 58.00;
  return distance;
}

void moveForward(int speed) {
  analogWrite(A_1B, 0);
  analogWrite(A_1A, speed);
  analogWrite(B_1B, speed);
  analogWrite(B_1A, 0);
}

void moveBackward(int speed) {
  analogWrite(A_1B, speed);
  analogWrite(A_1A, 0);
  analogWrite(B_1B, 0);
  analogWrite(B_1A, speed);
}

void turnRight(int speed) {
  analogWrite(A_1B, speed);
  analogWrite(A_1A, 0);
  analogWrite(B_1B, speed);
  analogWrite(B_1A, 0);
}

void turnLeft(int speed) {
  analogWrite(A_1B, 0);
  analogWrite(A_1A, speed);
  analogWrite(B_1B, 0);
  analogWrite(B_1A, speed);
}

void moveLeft(int speed) {
  analogWrite(A_1B, 0);
  analogWrite(A_1A, speed);
  analogWrite(B_1B, 0);
  analogWrite(B_1A, 0);
}

void moveRight(int speed) {
  analogWrite(A_1B, 0);
  analogWrite(A_1A, 0);
  analogWrite(B_1B, speed);
  analogWrite(B_1A, 0);
}

void backLeft(int speed) {
  analogWrite(A_1B, speed);
  analogWrite(A_1A, 0);
  analogWrite(B_1B, 0);
  analogWrite(B_1A, 0);
}

void backRight(int speed) {
  analogWrite(A_1B, 0);
  analogWrite(A_1A, 0);
  analogWrite(B_1B, 0);
  analogWrite(B_1A, speed);
}

void stopMove() {
  analogWrite(A_1B, 0);
  analogWrite(A_1A, 0);
  analogWrite(B_1B, 0);
  analogWrite(B_1A, 0);
}

// Allows the car drive itself by making corrections to avoid objects
void AutoDrive(int speed) {

  // 0: IR returns a hit, 1: IR doesn't return a hit
  int left = digitalRead(leftIR);
  int right = digitalRead(rightIR);

  // Checks if IR sensors have made any detections
  if (!left && right) backLeft(speed);
  else if (left && !right) backRight(speed);
  else if (!left && !right) moveBackward(speed);

  // If not check ulrasonic module
  else {

    // Reads data from us module
    float distance = ReadSensorData();
    Serial.println(distance);

    // No detections, move forward at full speed
    if (distance > 50) moveForward(200);

    // Object is detected, moves backwards and reorientates
    else if (distance < 10 && distance > 2) 
    {
      moveBackward(200);
      delay(1000);
      backLeft(150);
      delay(500);
    } 

    // Distance < 50, speed is reduced
    else moveForward(150);
  }
}

void Following(int speed) {

  // Gets data from ultrasonic and ir modules
  float distance = ReadSensorData();
  int left = digitalRead(leftIR);
  int right = digitalRead(rightIR);

  // If ultrasonic returns within specific range move forward
  if (distance > 5 && distance < 10) moveForward(speed);

  // If IR detects on either side turn that direction
  if (!left && right) turnLeft(speed);
  else if (left && !right) turnRight(speed);

  // If nothing returns dont move car
  else stopMove();
}

// Simple line tracking
void LineTrack(int speed) {

  // 0: Black color detected, 1: White color detected
  int lineColor = digitalRead(lineTrackPin);
  Serial.println(lineColor);

  // Constatly turns left until hitting the edge of the line, then turns right
  // to stay on the line
  if (lineColor) moveLeft(speed);
  else moveRight(speed);
}

// Tests the IR modules
void IrObstacleDrive(int speed) {

  // 0: IR returns a hit, 1: IR doesn't return a hit
  int left = digitalRead(leftIR);  
  int right = digitalRead(rightIR);

  // Object on left
  if (!left && right) backLeft(speed);

  // Object on right
  else if (left && !right) backRight(speed);

  // Object on both sides
  else if (!left && !right) moveBackward(speed);

  // Nothing detected
  else stopMove();
}

// Tests the ultrsonic module
void UltrasonicDrive(int speed) {

  // Get distance reading
  float distance = ReadSensorData();
  Serial.println(distance);

  // Object too far
  if (distance > 25) moveForward(speed);

  // Object too close
  else if (distance < 10 && distance > 2) moveBackward(speed);

  // Failsafe
  else stopMove();
}

// Takes hexidecimal from remote and converts it to assoicated enum value
Command decodeKeyValue(long result)
{
    switch(result)
    {
        case 0x09: return ACCELERATE;
        case 0x15: return DECELERATE;

        case 0x18: return FORWARD;
        case 0x0C: return FORWARDLEFT;
        case 0x5E: return FORWARDRIGHT;

        case 0x08: return LEFT;
        case 0x5A: return RIGHT;

        case 0x42: return BACKWARDLEFT;
        case 0x52: return BACKWARD;
        case 0x4A: return BACKWARDRIGHT;

        case 0x19: return LINETRACK;
        case 0x0D: return SELFDRIVE;
        case 0x43: return OBS_AVOID_US;
        case 0x40: return OBS_AVOID_IR;
        case 0x07: return HANDFOLLOW;
        case 0x16: return STOP;

        default: return ERROR_CMD;
    }
}
