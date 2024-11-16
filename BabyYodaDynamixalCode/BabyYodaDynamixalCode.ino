/*******************************************************************************
 * Copyright 2016 ROBOTIS CO., LTD.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *******************************************************************************/

#include <DynamixelShield.h>
#include <SoftwareSerial.h>
// Create software serial object to communicate with HC-05 (Bluetooth)
SoftwareSerial mySerial(3, 2); // HC-05 Tx & Rx is connected to Arduino #3 & #2

// ID for the six motor in total. Left/Right mean looking from the front, face to face with baby Yoda.
const uint8_t HEAD_ID = 101;
const uint8_t NECK_ID = 102;
const uint8_t LEFT_ARM_ID = 103;
const uint8_t LEFT_SHOULDER_ID = 104;
const uint8_t RIGHT_SHOULDER_ID = 105;
const uint8_t RIGHT_ARM_ID = 106;

// Stores the Degree information for each part.
// Note that this degree is not the same as the degree from dxl.getPresentPosition(), it is a custom degree system for convience.
// Don't use dxl.setGoalPosition to set goal position becuase the degree for the motor is not the same as the degree we use. And it doesn't check degree invariants, which might break the machine.
// Use RotateTo() to set goal positions.
// The inital position is where all the degrees are 0.
// The degrees already accounts for gear ratios. Suppose you want the right arm (Gear ratio 1:2) to rotate 90 degrees, you don't write need to write 180 degree. Just say 90 degree.
// Degree[0], head degree, -45~20 degree, with degree 0 points the head directly upward. The negative degree is nodding forward. The positive degree is raising the head backward.
// Degree[1], neck degree,-60~60 degree, with degree 0 points the head directly to the front. The negative degree looking to the right. The positive degree is looking to the left.
// Degree[2], left arm degree, -90~10 degree, with degree 0 points the left arm directly downward making a right angle between the shoulder and the arm. The negative degree bends the left arm outward. The positive degree bends the left arm inward.
// Degree[3], left shoulder degree, 0~120 degree, with degree 0 points the left arm directly downward. The negative degree bends the left arm backward. The positive degree bends the left arm forward.
// Degree[4], right shoulder degree, -120~0 degree, with degree 0 points the right arm directly downward. The negative degree bends the right arm forward. The positive degree bends the right arm backward.
// Degree[5], right arm degree, -90~10 degree, with degree 0 points the right arm directly downward making a right angle between the shoulder and the arm. The negative degree bends the right arm outward. The positive degree bends the right arm inward.
float Degree[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

// GearRatio for each motor
// GearRatio[0], GearRatio for head
// GearRatio[1], GearRatio for neck
// GearRatio[2], GearRatio for left arm
// GearRatio[3], GearRatio for left shoulder
// GearRatio[4], GearRatio for right shoulder
// GearRatio[5], GearRatio for right arm
float GearRatio[6] = {2.0, 1.0, 2.0, 1.0, 1.0, 2.0};

//record the starting time for each motor, unit in ms
unsigned long StartTime[6] = {0,0,0,0,0,0};

//the motor should stop after this amount of time, unit in ms, 0 means no limit on stoping.
unsigned long StopTime[6] = {0,0,0,0,0,0};

//record the rotational speed of each motor unit in rpm
float MotorSpeed[6]={0,0,0,0,0,0};

const float DXL_PROTOCOL_VERSION = 2.0;

DynamixelShield dxl;

// initialize very motor, turing them to initial degree
void initID(uint8_t ID, float initDegree = 0.0)
{
  // Get DYNAMIXEL information
  dxl.ping(ID);
  // Turn off torque when configuring items in EEPROM area
  // set it to zero degree position.
  dxl.torqueOff(ID);
  dxl.setOperatingMode(ID, OP_VELOCITY);
  dxl.torqueOn(ID);

  // make sure the motor is able to rotate from the closest side
  float initSpeed = 40; // how fast the motor turn to inital position, unit in rpm
  float currentDegree = dxl.getPresentPosition(ID, UNIT_DEGREE);
  float degreeDifference = initDegree - currentDegree;
  if (degreeDifference > 180)
  {
    degreeDifference = degreeDifference - 360;
  }
  else if (degreeDifference < -180)
  {
    degreeDifference = degreeDifference + 360;
  }
  // Now degreeDifference ranges from -180 to 180
  if (degreeDifference < 0)
  {
    dxl.setGoalVelocity(ID, -initSpeed, UNIT_RPM);
  }
  else
  {
    dxl.setGoalVelocity(ID, initSpeed, UNIT_RPM);
  }
  delay(((fabs(degreeDifference) / 360.0) * 60.0) / initSpeed * 1000.0);
  dxl.setGoalVelocity(ID, 0, UNIT_RPM);

  Degree[ID - 101] = 0.0;
}

// assert whether the degree is within the allowed range.
bool assertDegree(uint8_t ID, float deg)
{
  if (ID == HEAD_ID)
  {
    return -45 <= deg && 20 >= deg;
  }
  if (ID == NECK_ID)
  {
    return -60 <= deg && 60 >= deg;
  }
  if (ID == LEFT_ARM_ID)
  {
    return -90 <= deg && 10 >= deg;
  }
  if (ID == LEFT_SHOULDER_ID)
  {
    return 0 <= deg && 120 >= deg;
  }
  if (ID == RIGHT_SHOULDER_ID)
  {
    return -120 <= deg && 0 >= deg;
  }
  if (ID == RIGHT_ARM_ID)
  {
    return -90 <= deg && 10 >= deg;
  }
  // If the ID is not known
  return false;
}

// start the "ID" motor with 'speed' and stop it after 'stopTime' miliseconds
void start(uint8_t ID, float speed = 15.0, int stopTime = 0)
{
  dxl.setGoalVelocity(ID, speed * GearRatio[ID - 101], UNIT_RPM);
  StartTime[ID-101] = millis();
  MotorSpeed[ID-101] = speed;
  if(stopTime){
    StopTime[ID-101] = stopTime;
  }
}

// stop the "ID" motor 
void stop(uint8_t ID){
  dxl.setGoalVelocity(ID, 0, UNIT_RPM);
  Degree[ID-101] += float(millis()-StartTime[ID-101])/1000.0/60.0*MotorSpeed[ID-101]*360.0;
  MotorSpeed[ID-101] = 0;
  StopTime[ID-101] = 0;
}

// check if the motors should be stopped
void checkStopTime(){
  for(int ID = 101; ID <= 106;ID++){
    if(StopTime[ID-101]&&millis()-StartTime[ID-101]>=StopTime[ID-101]){
      stop(ID);
    }
  }
}

//Assert 
void AssertMotor(){
  for(int ID = 101; ID <= 106;ID++){
    float deg = float(millis()-StartTime[ID-101])/1000.0/60.0*MotorSpeed[ID-101]*360.0+Degree[ID-101];
    if(!assertDegree(ID, deg)){
      stop(ID);
    }
  }
}

// rotate rotates the motor with ID: 'ID' to the degree 'degree', with 'speed' rpm
void rotateTo(uint8_t ID, float deg, float speed = 15.0)
{
  if (!assertDegree(ID, deg))
  {
    return;
  }
  //'speed' should be positive
  if (speed <= 0)
  {
    speed = -speed;
  }
  //'speed' should not be bigger than 50
  if (speed > 40)
  {
    speed = 40;
  }
  float currentDegree = Degree[ID - 101];
  // rotate the motor
  if (deg < currentDegree)
  {
    dxl.setGoalVelocity(ID, -speed * GearRatio[ID - 101], UNIT_RPM);
  }
  else
  {
    dxl.setGoalVelocity(ID, speed * GearRatio[ID - 101], UNIT_RPM);
  }
  // rotate for the calculated amount of time.
  delay(((fabs(deg - currentDegree) / 360.0) * 60.0) / speed * 1000.0);
  dxl.setGoalVelocity(ID, 0, UNIT_RPM);
  Degree[ID - 101] = deg;
}

void testMotion()
{
  rotateTo(HEAD_ID, -45.0, 10.0);
  delay(500);
  rotateTo(HEAD_ID, 20.0);
  delay(500);
  rotateTo(HEAD_ID, 0.0, 5.0);
  delay(500);
  rotateTo(NECK_ID, -60, 20.0);
  delay(500); 
  rotateTo(NECK_ID, 60);
  delay(500);
  rotateTo(NECK_ID, 0.0);
  delay(500);
  rotateTo(RIGHT_SHOULDER_ID, -120.0, 70.0);
  delay(500);
  rotateTo(RIGHT_SHOULDER_ID, 0.0, 70.0);
  delay(500);
  rotateTo(LEFT_SHOULDER_ID, 120.0);
  delay(500);
  rotateTo(LEFT_SHOULDER_ID, 0.0);
  delay(500);
  rotateTo(LEFT_ARM_ID, -90.0);
  delay(500);
  rotateTo(LEFT_ARM_ID, 10.0);
  delay(500);
  rotateTo(LEFT_ARM_ID, 0.0);
  delay(500);
  rotateTo(RIGHT_ARM_ID, -90.0);
  delay(500);
  rotateTo(RIGHT_ARM_ID, 10.0);
  delay(500);
  rotateTo(RIGHT_ARM_ID, 0.0);
  delay(500);
}

void control(){
  if (mySerial.available())
  {
    char data = mySerial.read();
    Serial.write(data);
    switch (data){
      case 'T': //test motion
        testMotion();
        break;
      case 'I': //initialize position
        initID(HEAD_ID, 110.0);
        initID(NECK_ID, 0.0);
        initID(LEFT_ARM_ID, 70.0);
        initID(LEFT_SHOULDER_ID, 290.0);
        initID(RIGHT_SHOULDER_ID, 180.0);
        initID(RIGHT_ARM_ID, 240.0);
        break;
      case 'F': //move head forward
        start(101, 15.0);
        break;
      case 'f': //stop moving head forward
        stop(101);
        break;
      case 'B': //move head backward
        start(101, -15.0);
        break;
      case 'b': //stop moving head backward
        stop(101);
        break;
      case 'L': //move head leftward
        start(102, -15.0);
        break;
      case 'l': //stop moving head leftward
        stop(102);
        break;
      case 'R': //move head rightward
        start(102, 15.0);
        break;
      case 'r': //stop moving head rightward
        stop(102);
        break;
      case 'A': //move left arm outward
        start(103, -15.0);
        break;
      case 'a': //stop moving left arm outward
        stop(103);
        break;
      case 'S': //move left arm inward
        start(103, 15.0);
        break;
      case 's': //stop moving left arm inward
        stop(103);
        break;
      case 'Q': //move left arm forward
        start(104, 15.0);
        break;
      case 'q': //stop moving left arm forward
        stop(104);
        break;
      case 'Z': //move left arm backward
        start(104, -15.0);
        break;
      case 'z': //stop moving left arm backward
        stop(104);
        break;
      case 'P': //move right arm forward
        start(105, -15.0);
        break;
      case 'p': //stop moving right arm forward
        stop(105);
        break;
      case 'M': //move right arm backward
        start(105, 15.0);
        break;
      case 'm': //stop moving right arm backward
        stop(105);
        break;
      case 'K': //move right arm outward
        start(106, -15.0);
        break;
      case 'k': //stop moving right arm outward
        stop(106);
        break;
      case 'J': //move right arm inward
        start(106, 15.0);
        break;
      case 'j': //stop moving right arm inward
        stop(106);
        break;
    }
  }
}

void setup()
{
  // Begin serial communication with Arduino and Arduino IDE (Serial Monitor)
  Serial.begin(9600);
  // Begin serial communication with Arduino and HC-05
  mySerial.begin(9600);

  // Set Port baudrate to 57600bps. This has to match with DYNAMIXEL baudrate.
  dxl.begin(57600);
  // Set Port Protocol Version. This has to match with DYNAMIXEL protocol version.
  dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);

  // The initial position. If any mechanical structures has been changed than the inital values has to be modified
  initID(HEAD_ID, 110.0);
  initID(NECK_ID, 0.0);
  initID(LEFT_ARM_ID, 70.0);
  initID(LEFT_SHOULDER_ID, 290.0);
  initID(RIGHT_SHOULDER_ID, 180.0);
  initID(RIGHT_ARM_ID, 240.0);
  delay(500);
}
void loop()
{
  control();
  checkStopTime();
  //AssertMotor();
}
