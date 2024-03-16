import serial
import numpy
import matplotlib.pyplot as plt
from drawnow import *

Temperature = []
espData = serial.Serial('COM3', 500000)
plt.ion()
cnt = 0

def makeFig():
    plt.title('My Live Streaming Sensor Data')
    plt.grid(True)
    plt.xlabel('Time')
    plt.ylabel('Temperature (°C)')
    plt.plot(Temperature, label='Temp')
    plt.legend(loc='upper left')
    plt.xlim(max(0, len(Temperature) - 50), len(Temperature))  # Update x-axis limits

while True:
    while (espData.inWaiting() == 0):
        pass
    arduinoString = espData.readline().decode('utf-8').strip()
    dataArray = arduinoString.split(',')
    if len(dataArray) == 1:
        temp = float(dataArray[0])
        Temperature.append(temp)
        drawnow(makeFig)
        plt.pause(0.000001)