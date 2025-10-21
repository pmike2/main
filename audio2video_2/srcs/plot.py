#!/usr/bin/env python3

import os
import sys
import math
import json
from pprint import pprint as pp

import matplotlib.pyplot as plt
import numpy as np

#root = "../data/wav/sine_100"
#root = "../data/wav/sine_1000"
#root = "../data/wav/sine_noise"
root = "../data/wav/bcl1"

json_path = os.path.join(root, os.path.basename(root) + ".json")
wav_txt_path = os.path.join(root, os.path.basename(root) + ".txt")
root_fft = os.path.join(root, "fft")

with open(json_path) as f:
	js = json.load(f)

n_samples = js["frames"]
sample_rate = js["samplerate"]
block_size = js["block_size"]
delta_offset = js["delta_offset"]
#duration = n_samples / sample_rate

with open(wav_txt_path) as f:
	b = f.readlines()
wav_data = [float(x.strip()) for x in b if x.strip()]

fft_files = [os.path.join(root_fft, x) for x in os.listdir(root_fft) if os.path.splitext(x)[1] == ".txt" and not x.startswith(".")]
fft_files.sort(key=lambda x : int(os.path.splitext(os.path.basename(x))[0]))
fft_data = []
for i, fft_file in enumerate(fft_files):
	with open(fft_file) as f:
		b = f.readlines()
	fft_data.append([float(x.strip()) for x in b if x.strip()])

fft_data_by_freq = list(zip(*fft_data))

# freq_limits = [100.0, 200.0, 300.0, 500.0, 1000.0, 2000.0, 5000.0, 20000.0]
# fft_data_by_freq_grouped = [[]] * len(freq_limits)
# for idx_freq, data in enumerate(fft_data_by_freq):
# 	freq = idx_freq * sample_rate / block_size
# 	idx_limit_ok = None
# 	for idx_limit, freq_limit in enumerate(freq_limits):
# 		if freq< freq_limit:
# 			idx_limit_ok = idx_limit
# 			break
# 	fft_data_by_freq_grouped[idx_limit_ok]

transients = {}
for idx_freq, data in enumerate(fft_data_by_freq):
	transients[idx_freq] = []
	freq = idx_freq * sample_rate / block_size
	max_data = max(data)
	for idx in range(len(data)- 1):
		if data[idx + 1] - data[idx] > max_data * 0.8:
		#if abs(data[idx + 1] - data[idx]) > max_data * 0.8:
			transients[idx_freq].append(idx + 1)

#pp(transients)
#sys.exit()

freq_limits = [100.0, 300.0, 600.0, 1000.0, 2000.0, 3000.0, 7000.0, 50000.0]
transients_grouped = []
for i in range(len(freq_limits)):
	transients_grouped.append([])

for idx_freq, l_idx_block in transients.items():
	freq = idx_freq * sample_rate / block_size
	idx_limit_ok = None
	for idx_limit, freq_limit in enumerate(freq_limits):
		#print(f"freq={freq} ; freq_limit={freq_limit}")
		if freq< freq_limit:
			idx_limit_ok = idx_limit
			break
	#print(f"idx_freq={idx_freq} ; freq={freq} ; idx_limit_ok={idx_limit_ok} b={1 in l_idx_block} ; b2={1 in transients_grouped[0]}")
	transients_grouped[idx_limit_ok]+= l_idx_block

#print(1 in transients_grouped[0])

for i in range(len(transients_grouped)):
	#print(f"i={i} ; len={len(transients_grouped[i])}")
	transients_grouped[i] = sorted(list(set(transients_grouped[i])))
	#print(f"i={i} ; len={len(transients_grouped[i])}")

#pp(transients_grouped)
#sys.exit()

envelopes = []
threshold = 0.1
for idx_freq_group, l_block_idx_transient in enumerate(transients_grouped):
	if idx_freq_group == 0:
		freq_min = 0.0
	else:
		freq_min = freq_limits[idx_freq_group - 1]
	
	freq_max = freq_limits[idx_freq_group]
	
	for block_idx_transient in l_block_idx_transient:
		d = {"idx_freq_group" : idx_freq_group, "freq_min" : freq_min, "freq_max" : freq_max, "block_idx" : block_idx_transient, "amplitudes" : []}

		amplitudes = []
		for block_idx in range(block_idx_transient, len(data)):
			amplitudes.append(0.0)
		
		n_freqs = 0
		for idx_freq, data in enumerate(fft_data_by_freq):
			freq = idx_freq * sample_rate / block_size
			if freq>= freq_min and freq <= freq_max:
				n_freqs += 1
				for block_idx in range(block_idx_transient, len(data)):
					#print(len(amplitudes))
					#print(len(data))
					#print(block_idx)
					#print("------")
					amplitudes[block_idx - block_idx_transient]+= data[block_idx]
		
		block_idx_end = len(data)
		for block_idx in range(block_idx_transient, len(data)):
			amplitudes[block_idx - block_idx_transient] /= n_freqs
			if amplitudes[block_idx - block_idx_transient] < threshold:
				block_idx_end = block_idx - block_idx_transient
				break
		
		d["amplitudes"] = amplitudes[:block_idx_end]

		envelopes.append(d)

#pp(envelopes)
#sys.exit()

fig, ax = plt.subplots()
fig.set_size_inches(20.0, 8.0)

x_wav = np.linspace(0.0, n_samples, n_samples)
y_wav = np.array(wav_data, dtype='float32')
ax.plot(x_wav, y_wav, linewidth=0.2, label=f'wav')

#x_fft = np.array([block_size/ 2+ i * delta_offset for i in range(len(fft_data_by_freq[0]))], dtype='float32')
#l_freqs = [500.0, 5000.0]
#for freq in l_freqs:
#	idx_freq = int(math.ceil(freq * float(block_size) / float(sample_rate)))
	#print(f"freq={freq} ; idx_freq={idx_freq}")
#	y_fft = np.array(fft_data_by_freq[idx_freq], dtype='float32') * 0.001 * idx_freq
	#y_fft = np.array(fft_data_by_freq[idx_freq], dtype='float32') / np.max(fft_data_by_freq[idx_freq])
	#y_fft = np.log(np.array(fft_data_by_freq[idx_freq], dtype='float32'))
#	ax.plot(x_fft, y_fft, linewidth=0.5, label=f'{freq} Hz')

#l = [[block_size/ 2+ x * delta_offset, y * 0.001] for y in transients_grouped.keys() for x in transients_grouped[y]]
#l = [[block_size/ 2+ x * delta_offset, y * 0.1] for y in range(len(transients_grouped)) for x in transients_grouped[y]]
#x_transients = np.array(list(zip(*l))[0], dtype='float32')
#y_transients = np.array(list(zip(*l))[1], dtype='float32')
#ax.scatter(x_transients, y_transients, s=2.0, c='r')

colors = ["red", "blueviolet", "blue", "darkturquoise", "green", "orange", "yellow", "ivory"]
for envelope in envelopes:
	x_env = np.array([block_size/ 2+ delta_offset * (envelope["block_idx"] + i) for i in range(len(envelope["amplitudes"]))], dtype='float32')
	y_env = np.array(envelope["amplitudes"], dtype='float32') / np.max(envelope["amplitudes"])
	ax.plot(x_env, y_env, linewidth=0.5, c=colors[envelope["idx_freq_group"]])

ax.legend()

plt.show()
