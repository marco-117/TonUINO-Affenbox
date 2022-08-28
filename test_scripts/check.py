from pickle import TRUE
import serial, time, sys
arduino = serial.Serial('/dev/ttyUSB0', 115200, timeout=.1)
arduino.reset_input_buffer()
arduino.reset_output_buffer()
arduino.close()
arduino.open()
sc = ''
s = ""
i = 1
while i < 50:
  s = arduino.readline()
  if s :
    print (s)
  i += 1

arduino.close()
sys.exit(0)
