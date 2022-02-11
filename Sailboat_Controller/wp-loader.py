import serial
import sys

serialAddress = "COM7"
baudRate = 9600
numAttempts = 20

# Define a function to compute the checksum for a message
def CalculateChecksum(msg):
    checksum = msg.encode()[0]
    for i, byte in enumerate(msg.encode()):
        if i > 0:
            checksum = (0xff & checksum) ^ (0xff & byte)
    return checksum

# Open the serial port
port = serial.Serial(serialAddress, baudRate, timeout=5.0)

print("Putting SAMD20 into LOAD mode...")

for i in range(numAttempts):
    # Change the controller mode to LOAD
    port.write("$DALSAIL,001,2*5d\r\n".encode())   
    # Look for an acknowledgment
    ack = port.readline()
    print("Received: ", ack.decode())
    # Check the acknowledgment
    if ack.decode() == "$DALSAIL,000,0*5e\r\n":
        break
else:
    # No acknowledgments received...
    print("After %d tries, the script failed" % numAttempts)
    port.close()
    sys.exit()
    
print("Writing way points to SAMD20...")

wp_idx = 0
with open("OUTPUT.txt", 'r') as f:
    for line in f:
        fields = line.split(",")
        # Generate random way point
        lat = float(fields[0])
        lon = float(fields[1])
        rad = float(fields[2])
        next_idx = int(fields[3])
        
        # Create the message
        msg = "DALSAIL,004,%d,%d,%d,%d,%d" % (wp_idx, int(lat * 1000000), int(lon * 1000000), int(rad), next_idx)
        
        # Increment the idx
        wp_idx = wp_idx + 1
        
        print(msg)
        
        # Calculate the checksum
        checksum = msg.encode()[0]
        for i, byte in enumerate(msg.encode()):
            if i > 0:
                checksum = (0xff & checksum) ^ (0xff & byte)
            
        # Put the message into NMEA format
        nmea_msg = "$%s*%02x\r\n" % (msg, checksum)
        
        for j in range(numAttempts):
            
            # Send the message
            port.write(nmea_msg.encode())
              
            # Look for an acknowledgment
            ack = port.readline()
            print("Received: ", ack.decode())
            
            # Check the acknowledgment
            if ack.decode() == "$DALSAIL,000,0*5e\r\n":
                break
        else:
            # No acknowledgments received...
            print("After %d tries, the script failed" % numAttempts)
            port.close()
            sys.exit()

print("Putting SAMD20 into AUTO mode...")

for i in range(numAttempts):
    # Change the controller mode to LOAD
    port.write("$DALSAIL,001,0*5f\r\n".encode())   
    # Look for an acknowledgment
    ack = port.readline()
    print("Received: ", ack.decode())
    # Check the acknowledgment
    if ack.decode() == "$DALSAIL,000,0*5e\r\n":
        break
else:
    # No acknowledgments received...
    print("After %d tries, the script failed" % numAttempts)
    port.close()
    sys.exit()

port.close()

