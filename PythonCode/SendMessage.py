import serial
import time
import ProcessData

# Replace 'COM7' with the appropriate port for your system:
# On Windows: something like 'COM5', 'COM7', etc.
# On Linux/macOS: something like '/dev/rfcomm0' or '/dev/tty.HC-05-DevB'
port = 'COM4'
baud_rate = 9600  # Match this with your Arduino and HC-05 configuration

try:
    # Open the serial connection
    ser = serial.Serial(port, baud_rate, timeout=1)
    time.sleep(2)  # Give the connection a second to settle
    # The message you want to send
    while True:
        message = ProcessData.sendCode()
        if message:
            ser.write(message.encode('utf-8'))
        print("Message sent:", message, end="\r")

except serial.SerialException as e:
    print("Could not open serial port:", e)

finally:
    if 'ser' in locals() and ser.is_open:
        ser.close()
