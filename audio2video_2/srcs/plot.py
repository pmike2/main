#!/usr/bin/env python3

import os
import sys
import json
from pprint import pprint as pp

import matplotlib.pyplot as plt
import numpy as np


FREQ_GROUPS_LIMITS = [100.0, 300.0, 600.0, 1000.0, 2000.0, 3000.0, 7000.0, 50000.0]
TRANSIENT_THRESHOLD = 0.5
ENVELOPE_END_RATIO = 0.008
MIN_DIST_BETWEEN_TRANSIENT = 20
BAND_COLORS = ["red", "blueviolet", "blue", "darkturquoise", "green", "orange", "olive", "gold"]


#root = "../data/wav/sine_100"
#root = "../data/wav/sine_1000"
#root = "../data/wav/sine_noise"
root = "../data/wav/bcl6"


json_path = os.path.join(root, os.path.basename(root) + ".json")
wav_txt_path = os.path.join(root, os.path.basename(root) + ".txt")
root_fft = os.path.join(root, "fft")

with open(json_path) as f:
	js = json.load(f)

n_samples = js["frames"]
sample_rate = js["samplerate"]
block_size = js["block_size"]
delta_offset = js["delta_offset"]
n_blocks = js["n_blocks"]
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

transients = {}
for idx_freq, data in enumerate(fft_data_by_freq):
	transients[idx_freq] = []
	freq = idx_freq * sample_rate / block_size
	min_data, max_data = min(data), max(data)
	for idx in range(n_blocks- 1):
		if data[idx + 1] - data[idx] > (max_data - min_data) * TRANSIENT_THRESHOLD:
			transients[idx_freq].append(idx + 1)

#pp(transients)
#sys.exit()

transients_grouped = []
for i in range(len(FREQ_GROUPS_LIMITS)):
	transients_grouped.append([])

for idx_freq, l_idx_block in transients.items():
	freq = idx_freq * sample_rate / block_size
	idx_limit_ok = None
	for idx_limit, freq_limit in enumerate(FREQ_GROUPS_LIMITS):
		if freq< freq_limit:
			idx_limit_ok = idx_limit
			break
	transients_grouped[idx_limit_ok]+= l_idx_block

for i in range(len(transients_grouped)):
	transients_grouped[i] = sorted(list(set(transients_grouped[i])))

#pp(transients_grouped)
#sys.exit()

transients_grouped_pruned = []
#for i in range(len(FREQ_GROUPS_LIMITS)):
#	transients_grouped_pruned.append([])

for idx_freq_group, l_block_idx_transient in enumerate(transients_grouped):
	if len(l_block_idx_transient) == 0:
		transients_grouped_pruned.append([])
		continue
	
	l = [l_block_idx_transient[0]]
	last_block_idx = 0
	for i, block_idx_transient in enumerate(l_block_idx_transient):
		if i == 0:
			continue
		if block_idx_transient > last_block_idx + MIN_DIST_BETWEEN_TRANSIENT:
			last_block_idx = block_idx_transient
			l.append(block_idx_transient)

	transients_grouped_pruned.append(l)

#pp(transients_grouped_pruned)
#sys.exit()

envelopes = []
for idx_freq_group, l_block_idx_transient in enumerate(transients_grouped_pruned):
	if idx_freq_group == 0:
		freq_min = 0.0
	else:
		freq_min = FREQ_GROUPS_LIMITS[idx_freq_group - 1]
	
	freq_max = FREQ_GROUPS_LIMITS[idx_freq_group]
	
	for i, block_idx_transient in enumerate(l_block_idx_transient):
		d = {"idx_freq_group" : idx_freq_group, "freq_min" : freq_min, "freq_max" : freq_max, "block_idx" : block_idx_transient, "amplitudes" : []}

		if i == len(l_block_idx_transient) - 1:
			block_idx_transient_next = n_blocks
		else:
			block_idx_transient_next = l_block_idx_transient[i + 1]

		amplitudes = []
		for block_idx in range(block_idx_transient, block_idx_transient_next):
			amplitudes.append(0.0)
		
		n_freqs = 0
		min_data, max_data = 1e5, -1e5
		for idx_freq, data in enumerate(fft_data_by_freq):
			freq = idx_freq * sample_rate / block_size
			if freq>= freq_min and freq <= freq_max:
				n_freqs += 1
				for block_idx in range(block_idx_transient, block_idx_transient_next):
					amplitudes[block_idx - block_idx_transient]+= data[block_idx]
					if data[block_idx] < min_data:
						min_data = data[block_idx]
					if data[block_idx] > max_data:
						max_data = data[block_idx]
		
		block_idx_end = block_idx_transient_next
		for block_idx in range(block_idx_transient + 1, block_idx_transient_next):
			amplitudes[block_idx - block_idx_transient] /= n_freqs
			if amplitudes[block_idx - block_idx_transient] < min_data + (max_data - min_data) * ENVELOPE_END_RATIO:
				block_idx_end = block_idx
				break
		
		d["amplitudes"] = amplitudes[:block_idx_end - block_idx_transient]

		envelopes.append(d)

#pp(envelopes)
#sys.exit()

js["envelopes"] = envelopes
with open(json_path, "w") as f:
	json.dump(js, f)

fig, ax = plt.subplots()
fig.set_size_inches(20.0, 8.0)

#x_wav = np.linspace(0.0, n_samples, n_samples)
x_wav = np.linspace(0.0, n_samples, len(wav_data))
y_wav = np.array(wav_data, dtype='float32')
ax.plot(x_wav, y_wav, linewidth=0.4, label=f'wav', c='lightgrey')

#x_fft = np.array([block_size/ 2+ i * delta_offset for i in range(len(fft_data_by_freq[0]))], dtype='float32')
#l_freqs = [500.0, 5000.0]
#for freq in l_freqs:
#	idx_freq = int(math.ceil(freq * float(block_size) / float(sample_rate)))
	#print(f"freq={freq} ; idx_freq={idx_freq}")
#	y_fft = np.array(fft_data_by_freq[idx_freq], dtype='float32') * 0.001 * idx_freq
	#y_fft = np.array(fft_data_by_freq[idx_freq], dtype='float32') / np.max(fft_data_by_freq[idx_freq])
	#y_fft = np.log(np.array(fft_data_by_freq[idx_freq], dtype='float32'))
#	ax.plot(x_fft, y_fft, linewidth=0.5, label=f'{freq} Hz')

l = [[block_size/ 2+ x * delta_offset, y * 0.1] for y in range(len(transients_grouped_pruned)) for x in transients_grouped_pruned[y]]
x_transients = np.array(list(zip(*l))[0], dtype='float32')
y_transients = np.array(list(zip(*l))[1], dtype='float32')
ax.scatter(x_transients, y_transients, s=2.0, c='r')

for envelope in envelopes:
	x_env = np.array([block_size/ 2+ delta_offset * (envelope["block_idx"] + i) for i in range(len(envelope["amplitudes"]))], dtype='float32')
	y_env = np.array(envelope["amplitudes"], dtype='float32') / np.max(envelope["amplitudes"])
	ax.plot(x_env, y_env, linewidth=0.5, c=BAND_COLORS[envelope["idx_freq_group"]])

ax.legend()

plt.show()
