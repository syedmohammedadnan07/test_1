"""
To plot the recorded sound, you can use a scripting language like Python with libraries such as Matplotlib. First, you need to read the recorded data from the file `recorded.raw`, and then plot it.
"""
import matplotlib.pyplot as plt
import numpy as np

# Read the recorded data
with open('recorded.raw', 'rb') as file:
    data = np.fromfile(file, dtype=np.float32)

# Plot the data
plt.plot(data)
plt.title("Recorded Audio Waveform")
plt.xlabel("Sample")
plt.ylabel("Amplitude")
plt.show()
