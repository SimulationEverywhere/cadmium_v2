import matplotlib.pyplot as plt
import serial

# serial port parameters
PORT = '/dev/ttyACM0'
BAUDRATE = 9600

tblk_list = []
outblk_list = []
tdin_list = []
outdin_list = []

with serial.Serial(PORT, BAUDRATE, timeout=1) as ser:
    for i in range(0,20):
        line = ser.readline()
        print(line.decode("utf-8").split(";"))
