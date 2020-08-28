import sys
import glob
import serial
from serial.tools.list_ports import comports

# allPorts = comports()
# for port in allPorts:
#     print(port)

print("Number of ports: ")
num_ports = int(input())
ports = []
text = []
for i in range(num_ports):
    print("Port ", i, ":")
    port = input()
    ports.append(serial.Serial(port, 115200, timeout=1))
    text.append([])

buffersize = 8
for i in range(buffersize):
    for j in range(num_ports):
        text[j].append("")
while(True):
    for i in range(num_ports):
        byte = ports[i].read().decode("utf-8")
        if (byte != '\n'):
            text[i][buffersize-1] += byte
        else:
            for j in range(buffersize-1):
                text[i][j] = text[i][j+1]
            text[i][buffersize-1] = ''
    for i in range(buffersize):
        for j in range(num_ports):
            print('{:<60s}'.format(text[j][i]), end='')
        print()
    


