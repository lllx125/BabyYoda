#include <Bluepad32.h>

// Callback when the controller is connected

void onConnectedGamepad(GamepadPtr gp);

// Global gamepad object

GamepadPtr myGamepad;

// A small struct to hold a 2D state

struct StickState {

int x;  // -1, 0, +1

int y;  // -1, 0, +1

};

// Overall “robot command state”

struct RobotControllerState {

// Head DPAD

int headLR;  // -1 = left, 0 = none, 1 = right

int headUD;  // -1 = down, 0 = none, 1 = up

// Face buttons

int specialX;  // X

int specialY;  // Y

int specialB;  // B

int specialA;  // A

// Joystick-based “PosToState” for arms

StickState lArm;

StickState rArm;

};

// Global variable to remember the previous frame’s state.

RobotControllerState previousState = {0, 0, 0, 0, 0, 0, {0, 0}, {0, 0}};

// Callback implementation

void onConnectedController(GamepadPtr gp) {

myGamepad = gp;

}

void onDisconnectedController(ControllerPtr ctl) {}

// converts a joy stick to discreet values

StickState posToState(float x, float y) {

// If magnitude is too small, treat as “no movement”

float mag = sqrtf(x*x + y*y);

if (mag < 0.5f) {

return {0, 0};

}

float deg = atan2(y, x) * 180.0f / float(M_PI);

// Replicate your Python if/elif’s

if      (deg >  -30 && deg <=  30)  return { 1,  0};

else if (deg >   30 && deg <=  60)  return { 1,  1};

else if (deg >   60 && deg <= 120)  return { 0,  1};

else if (deg >  120 && deg <= 150)  return {-1,  1};

else if (deg >  150 || deg <= -150) return {-1,  0};

else if (deg >  -60 && deg <= -30)  return { 1, -1};

else if (deg > -120 && deg <= -60)  return { 0, -1};

else if (deg > -150 && deg <=-120)  return {-1, -1};

// Fallback

return {0, 0};

}

// HEAD (DPAD)

String processHead(const RobotControllerState &oldState, const RobotControllerState &newState) {

String code;

// Compare left-right

if (oldState.headLR != newState.headLR) {

if      (newState.headLR ==  0) code += "l";  // “stop turning head”

else if (newState.headLR ==  1) code += "R";  // turn head right

else if (newState.headLR == -1) code += "L";  // turn head left

}

// Compare up-down

if (oldState.headUD != newState.headUD) {

if      (newState.headUD ==  0) code += "f";  // “stop tilt”

else if (newState.headUD ==  1) code += "B";  // tilt up

else if (newState.headUD == -1) code += "F";  // tilt down

}

return code;

}

// SPECIAL BUTTONS (X, Y, B, A)

String processSpecialFunctions(const RobotControllerState &oldState, const RobotControllerState &newState) {

String code;

// X pressed => "T" (Test motion)

if (!oldState.specialX && newState.specialX) {

code += "T";

}

// Y pressed => "I" (Initialize)

if (!oldState.specialY && newState.specialY) {

code += "I";

}

// B pressed => "W" (Wave)

if (!oldState.specialB && newState.specialB) {

code += "W";

}

// A is present in Python but not used to emit a command (only read).

// If you do need an “A pressed => do something,” replicate it here:

// if (!oldState.specialA && newState.specialA) { code += ??? }

return code;

}

// LEFT ARM

String processLArm(const RobotControllerState &oldState, const RobotControllerState &newState) {

String code;

// X direction

if (oldState.lArm.x != newState.lArm.x) {

if      (newState.lArm.x ==  0) code += "s";

else if (newState.lArm.x ==  1) code += "S";

else if (newState.lArm.x == -1) code += "A";

}

// Y direction

if (oldState.lArm.y != newState.lArm.y) {

if      (newState.lArm.y ==  0) code += "q";

else if (newState.lArm.y ==  1) code += "Z";

else if (newState.lArm.y == -1) code += "Q";

}

return code;

}

// RIGHT ARM

String processRArm(const RobotControllerState &oldState, const RobotControllerState &newState) {

String code;

// X direction

if (oldState.rArm.x != newState.rArm.x) {

if      (newState.rArm.x ==  0) code += "j";

else if (newState.rArm.x ==  1) code += "K";

else if (newState.rArm.x == -1) code += "J";

}

// Y direction

if (oldState.rArm.y != newState.rArm.y) {

if      (newState.rArm.y ==  0) code += "p";

else if (newState.rArm.y ==  1) code += "M";

else if (newState.rArm.y == -1) code += "P";

}

return code;

}

String buildCode(const RobotControllerState &oldState, const RobotControllerState &newState) {

String out;

out += processHead(oldState, newState);

out += processSpecialFunctions(oldState, newState);

out += processLArm(oldState, newState);

out += processRArm(oldState, newState);

return out;

}

void setup() {

Serial.begin(115200);

// Initialize Bluepad32

Serial1.begin(9600, SERIAL_8N1, 3,1);

BP32.setup(&onConnectedController, &onDisconnectedController);

}

void loop() {

// Call the Bluepad32 update function

BP32.update();

if (myGamepad && myGamepad->isConnected()) {

RobotControllerState current;

int dpad = myGamepad->dpad();  // Get the DPAD bitmask

// Left / Right detection

int lr = 0;  // Default: no movement

if (dpad & DPAD_LEFT)  lr = -1;

if (dpad & DPAD_RIGHT) lr =  1;

// Up / Down detection

int ud = 0;

if (dpad & DPAD_UP)    ud =  1;

if (dpad & DPAD_DOWN)  ud = -1;

current.headLR = lr;

current.headUD = ud;

// Face buttons

current.specialX = myGamepad->x();  // Boolean

current.specialY = myGamepad->y();  // Boolean

current.specialB = myGamepad->b();  // Boolean

current.specialA = myGamepad->a();  // Boolean

// Left analog stick, normalized ~-1..1

float lX = (float)myGamepad->axisX() / 512.0f;

float lY = (float)myGamepad->axisY() / 512.0f;

current.lArm = posToState(lX, lY);

// Right analog stick

float rX = (float)myGamepad->axisRX() / 512.0f;

float rY = (float)myGamepad->axisRY() / 512.0f;

current.rArm = posToState(rX, rY);

// Build the command string by comparing with previous frame.

String code = buildCode(previousState, current);

// Send data to Arduino via Serial1

if (code.length() > 0) {

Serial1.println(code[0]);

Serial.println(code[0]);

}

// Update previous for next iteration

previousState = current;

}

}