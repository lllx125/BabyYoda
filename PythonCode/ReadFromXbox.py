from inputs import get_gamepad
import math
import threading
import time
import math


def PosToState(x, y):
    # did not push outward enough, do not activate
    if math.sqrt(x**2+y**2) < 0.5:
        return [0, 0]
    degree = math.degrees(math.atan2(y, x))

    if -30 < degree and degree <= 30:
        return [1, 0]
    elif 30 < degree and degree <= 60:
        return [1, 1]
    elif 60 < degree and degree <= 120:
        return [0, 1]
    elif 120 < degree and degree <= 150:
        return [-1, 1]
    elif 150 < degree and degree <= 180:
        return [-1, 0]
    elif -60 < degree and degree <= -30:
        return [1, -1]
    elif -120 < degree and degree <= -60:
        return [0, -1]
    elif -150 < degree and degree <= -120:
        return [-1, -1]
    elif -180 <= degree and degree <= -150:
        return [-1, 0]


class XboxController(object):
    MAX_TRIG_VAL = math.pow(2, 8)
    MAX_JOY_VAL = math.pow(2, 15)

    def __init__(self):

        self.LeftJoystickY = 0
        self.LeftJoystickX = 0
        self.RightJoystickY = 0
        self.RightJoystickX = 0
        self.LeftTrigger = 0
        self.RightTrigger = 0
        self.LeftBumper = 0
        self.RightBumper = 0
        self.A = 0
        self.X = 0
        self.Y = 0
        self.B = 0
        self.LeftThumb = 0
        self.RightThumb = 0
        self.Back = 0
        self.Start = 0
        self.LRDPad = 0
        self.UDDPad = 0

        self._monitor_thread = threading.Thread(
            target=self._monitor_controller, args=())
        self._monitor_thread.daemon = True
        self._monitor_thread.start()

    def read(self):  # return the buttons/triggers that you care about in this methode
        LR = -self.LRDPad  # Head Left Right
        UD = -self.UDDPad  # Head Up Down
        X = self.X  # Test Motion
        Y = self.Y  # Initialize
        B = self.B
        A = self.A
        Lx = self.LeftJoystickX  # Left Arm x
        Ly = self.LeftJoystickY  # Left Arm y
        Rx = self.RightJoystickX  # Right Arm x
        Ry = self.RightJoystickY  # Right Arm y
        # Head, Special Function, Left Arm, Right Arm
        return [[LR, UD], [X, Y, B, A], PosToState(Lx, Ly), PosToState(Rx, Ry)]

    def _monitor_controller(self):
        while True:
            events = get_gamepad()
            for event in events:
                if event.code == 'ABS_Y':
                    self.LeftJoystickY = event.state / \
                        XboxController.MAX_JOY_VAL  # normalize between -1 and 1
                elif event.code == 'ABS_X':
                    self.LeftJoystickX = event.state / \
                        XboxController.MAX_JOY_VAL  # normalize between -1 and 1
                elif event.code == 'ABS_RY':
                    self.RightJoystickY = event.state / \
                        XboxController.MAX_JOY_VAL  # normalize between -1 and 1
                elif event.code == 'ABS_RX':
                    self.RightJoystickX = event.state / \
                        XboxController.MAX_JOY_VAL  # normalize between -1 and 1
                elif event.code == 'ABS_Z':
                    self.LeftTrigger = event.state / \
                        XboxController.MAX_TRIG_VAL  # normalize between 0 and 1
                elif event.code == 'ABS_RZ':
                    self.RightTrigger = event.state / \
                        XboxController.MAX_TRIG_VAL  # normalize between 0 and 1
                elif event.code == 'BTN_TL':
                    self.LeftBumper = event.state
                elif event.code == 'BTN_TR':
                    self.RightBumper = event.state
                elif event.code == 'BTN_SOUTH':
                    self.A = event.state
                elif event.code == 'BTN_NORTH':
                    self.Y = event.state  # previously switched with X
                elif event.code == 'BTN_WEST':
                    self.X = event.state  # previously switched with Y
                elif event.code == 'BTN_EAST':
                    self.B = event.state
                elif event.code == 'BTN_THUMBL':
                    self.LeftThumb = event.state
                elif event.code == 'BTN_THUMBR':
                    self.RightThumb = event.state
                elif event.code == 'BTN_SELECT':
                    self.Back = event.state
                elif event.code == 'BTN_START':
                    self.Start = event.state
                elif event.code == 'ABS_HAT0Y':
                    self.LRDPad = event.state
                elif event.code == 'ABS_HAT0X':
                    self.UDDPad = event.state


if __name__ == '__main__':
    joy = XboxController()
    while True:
        print(joy.read(), end="\r")
