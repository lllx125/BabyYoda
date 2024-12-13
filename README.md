# BabyYoda

the Code for Baby Yoda using Dynamixel in Fall 2024 by Lixing Li. This Intruction aims to help you learn to control Baby Yoda, built your own Baby Yoda, understand and expand on the current version. GooD Luck!

### Control Version: Xbox to Laptop to HC-05 to Arduino to Dynamixel - 12/07/2024

## Control Intructions

### Control

This is designed to control Baby Yoda face-to-face. Therefore, when you push the left joy stick, it moves its arm on your left (its right arm).

-   Left Joy Stick: Left Arm
-   Right Joy Stick: Right Arm
-   Dpad: Head
-   X: Shows a series of predefined motions
-   Y: Go to initial position

### Power

There are two seperate power systems: Arduino (9V) and Dynamixel Shield (12V). They are powered differently. When you turn Baby Yoda on, make sure they are both powered.

The switch controls the power for the Dynamixel Shield. The Right most charger charges the internal battery for the Dynamixel Shield.

The Arduino is powered by either plugingin the charger from a 9V battery or 9V power source. Or you can plugin the USB-B port and power it by your computer.

### Connections

Turn on Bluetooth and pair your computer with HC-05. The passcode is `1234`.

Press and hold the pairing button on the Xbox One Controller and pair it with your laptop. The pairing button is the one on the top of the Xbox Controller, next to the USB-C port.

#### Pull this repository from Github and run `SendMessage.py` you might need to install some [dependencies](#dependecies).

### Port Setups (Optional)

Change `port` in `SendMessage.py` to the port name of the HC-05. In windows, it is usually `COM4` or `COM5`/ In mac, it is usually `/dev/cu.HC-05`

## For Developers

## Setup

### Wiring

-   Connect the Dynamixels in series and plugin the wire onto the dynamixel shield (any pin is ok Dynamixel shield is able to recognize their motors by the ID).

-   The Dynamixel shield is placed directly on top of the Aruidno Uno board.

-   The Bluetooth Module HC-05 is connected to the Arduino (via the pins on the Dynamixel Shield) in the following ways: GND to GND, 5V to 5V, TX to 3, RX to 4.

-   The shield is independently powered by a 12 V battery and it has its own switch to control the power.

### Dynamixel Details

-   Each Dynamixel motor's ID is changed to the following
    (Left/Right mean looking from the front, face to face with baby Yoda.)

    -   Head motor (up-down): 101
    -   Neck motor (left-right): 102
    -   Left Arm motor (outward-inward): 103
    -   Left Shoulder motor (forward-backward): 104
    -   Right Shoulder motor (forward-backward): 105
    -   Right Arm motor (outward-inward): 106

-   To upload the code, make sure that the switch on the Dynamixel shield is turned to `upload` mode. When you want the motor to run, make sure the switch is turned to `Dynamixel` mode.

-   If the motor is stuck, the LED will flash and the motor might lose its torque.

### Code Dependecies

The Arduino code is dependent on `DynamixelShield`. Download this library from Arduino's library manager.

The python code is dependent on `inputs` and `pyserial`. Run

```bash
pip install inputs
pip install pyserial
```

in the terminal to install the packages.

## Code Explanation

### Arduino Code (`BabyYodaDynamixelCode.ino`)

#### Main Functionalities:

-   Degree mechanisms:
    -   The code does not read the degree from the motor directly, rather, it converts the degree into my own coordinate. The new degree coordinate is the motor degree with a shift (no scaling), where the "initial degree" is shifted to 0 degree in my coordinate.
    -   The degree shift can be tune by changing the inital position of the motor in the `initPosition()` function. The `initID()` turns the motor into its actual degree and set that degree to our new 0.
    -   `initID()` makes sure that the motor returns to intial position from the proper direction is a smooth manner.
    -   If the joints and motors changed, the inital degrees needs to be tuned again. **If Right Arm motor and Left Arm motor ever fell off the gear, the inital position will need to be changed!** If the motor got stuck, it will not work.
    -   Press the reset button on the Arduino to make it go to intial position.
    -   The degrees are updated by code (the program knows how the motors are turned). It is stored in `Degree`.
    -   The each motor has its degree bounded by some limit to avoid motors being stucked. It is stored in `DegreeLimit` and checked by `assertDegree()`.
    -   The Gear ratio is accounted by the rotational speed, but the degree still represents the degree of the motor.
-   `rotateTo()`: rotate the motor to a certain degree with a certain speed.
-   `testMotions()`: make Bady Yoda follow a fixed path of motions.
-   Instructions to control motors:
    -   `control()`: recieves instructions from the serial port and convert it to motor movements.
    -   `start()`: make a certain motor move at a certain speed. The function checks whether this movement direction is allowed (Not allowed if any movement forward will exceed the degree limit). It computes the maximum run time of the motor and sets a future stop time. If the stop signal is not sent, the motor automatically stops when it reaches the limit.
    -   `stop()`: called when a stop instruction is sent or `checkStopTime()` stops the motor. This function stops the motor.

### `ReadFromXbox.py`

-   Reads Gamepad status from the Xbox controller and organize it, normalizing the joystick position. The useful informations are returned by `read()`.

-   When the file is runed, it outputs the results of `read()`.

-   `PosToState()` turns the position of the gamepad joystick into discrete states.

### `ProcessData.py`

-   Record the change in states. When the a certain state changes, sends according instructions via `sendCode()`.

### `SendMessage.py`

-   Connects to Serial and sents the data to the serial port.
-   Change `port` to the one for HC-05 when needed.

## Materials

-   Baby Yoda Model
-   Arduino Uno
-   Dynamixel Shield
-   6 Dynamixel XC330 Motors
-   HC-05 Bluetooth Module
-   12V Rechargable Battery
-   Wires
-   Switch
-   Cable (for powering Arduino or sending code to Arduino) or 9V battery (for powering Arduino)
-   Xbox One Controller
