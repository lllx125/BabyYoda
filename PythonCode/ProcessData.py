from ReadFromXbox import XboxController

joy = XboxController()

state = [[0, 0], [0, 0, 0, 0], [0, 0], [0, 0]]


def ProcessHead(y, x):
    code = ""
    Y = state[0][0]
    X = state[0][1]

    if Y != y:
        if y == 0:
            code += 'f'
        elif y == 1:
            code += 'B'
        elif y == -1:
            code += 'F'
    if X != x:
        if x == 0:
            
            code += 'l'
        elif x == 1:
            code += 'R'
        elif x == -1:
            code += 'L'
    return code


def ProcessSpecialFunctions(x, y, b, a):
    code = ""
    X = state[1][0]
    Y = state[1][1]
    B = state[1][2]
    A = state[1][3]
    # f is an array of length 4, each index represents X, Y, B, A
    if X == 0 and x == 1:  # X on press, do Test Motion
        code += "T"
    if Y == 0 and y == 1:  # Y on press, do Initialize
        code += "I"

    return code


def ProcessLArm(x, y):
    code = ""
    X = state[2][0]
    Y = state[2][1]
    if Y != y:
        if y == 0:
            code += 'q'
        elif y == 1:
            code += 'Q'
        elif y == -1:
            code += 'Z'
    if X != x:
        if x == 0:
            code += 's'
        elif x == 1:
            code += 'S'
        elif x == -1:
            code += 'A'
    return code


def ProcessRArm(x, y):
    code = ""
    X = state[3][0]
    Y = state[3][1]
    if Y != y:
        if y == 0:
            code += 'p'
        elif y == 1:
            code += 'P'
        elif y == -1:
            code += 'M'
    if X != x:
        if x == 0:
            code += 'j'
        elif x == 1:
            code += 'K'
        elif x == -1:
            code += 'J'
    return code


def sendCode():
    global state  # Tell Python we want to use the global 'state' variable
    current = joy.read()
    code = ""
    head = current[0]
    specialFunctions = current[1]
    LArm = current[2]
    RArm = current[3]
    code += ProcessHead(head[0], head[1])
    code += ProcessSpecialFunctions(
        specialFunctions[0], specialFunctions[1], specialFunctions[2], specialFunctions[3])
    code += ProcessLArm(LArm[0], LArm[1])
    code += ProcessRArm(RArm[0], RArm[1])
    # Update state before returning
    state = current

    return code
