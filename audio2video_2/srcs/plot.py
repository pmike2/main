#!/usr/bin/env python3

import os

import matplotlib.pyplot as plt
import numpy as np

script_dir = os.path.dirname(__file__)

XMAX = 100

#data = os.path.join(script_dir, "..", "data", "wav", "test.txt")
data = os.path.join(script_dir, "..", "data", "fft", "test_0.txt")

with open(data) as f:
	b = f.readlines()
l = [float(x.strip()) for x in b if x.strip()]

x = np.linspace(0, XMAX, len(l))
y = np.array(l, dtype='float32')

fig, ax = plt.subplots()
ax.plot(x, y, linewidth=0.3)
plt.show()
