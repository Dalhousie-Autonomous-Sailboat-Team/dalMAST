# -*- coding: utf-8 -*-

import numpy as np
import matplotlib.pyplot as plt

# Define the controller constants
motor_min_speed =  75.0 / 255.0
motor_max_speed = 250.0 / 255.0
error_min_deg   =   5.0
error_max_deg   =  20.0

# Define the transfer function
def h(x):
    return np.interp(x, [error_min_deg, error_max_deg], [motor_min_speed, motor_max_speed])

# Define a range of inputs and plot the output
x = np.arange(0.0, 30.0, 0.1)
plt.plot(x, 100 * h(x))
plt.xlabel('angle error / degree')
plt.ylabel('motor speed / 100')
plt.ylim([0.0, 100.0])
plt.title('motor controller transfer function')
plt.grid(True)
plt.savefig("../figures/p_control.pdf")
plt.show()