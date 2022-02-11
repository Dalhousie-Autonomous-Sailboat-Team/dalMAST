
import struct
import numpy as np
import matplotlib.pyplot as plt

with open("test03.rof", mode="rb") as file:
    fileContent = file.read()
    header = fileContent[:28]
    data = fileContent[28:]
    
    sample_period = int(struct.unpack("I", header[16:20])[0])
    num_samples = int(struct.unpack("I", header[20:24])[0])
    
    channel_samples = struct.unpack("i" * 6 * num_samples, data)
    channel_1_volt = np.array(channel_samples[0::6])/10000
    channel_1_amp = np.array(channel_samples[1::6])/10000
    
    time = sample_period * np.arange(0, num_samples)
    
    plt.figure()
    plt.plot(time, channel_1_volt)
    plt.grid()
    plt.xlabel("time / s")
    plt.ylabel("voltage / V")
    
    plt.figure()
    plt.plot(time, 1000 * channel_1_amp)
    plt.grid()
    plt.xlabel("time / s")
    plt.ylabel("current / mA")
    
    print("Average current draw:", int(1000 * np.average(channel_1_amp)), "mA")
    
    
    
    plt.show()