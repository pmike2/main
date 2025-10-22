#!/usr/bin/env python3

import os

THRESHOLD = 0.01

root1 = "../data/wav/bcl6"
root2 = "../data/wav/tmp"

ffts1 = [os.path.join(root1, "fft", x) for x in os.listdir(os.path.join(root1, "fft")) if os.path.splitext(x)[1] == ".txt" and not x.startswith(".")]

for fft_txt1 in ffts1:
	fft_txt2 = os.path.join(root2, "fft", os.path.basename(fft_txt1))
	if not os.path.isfile(fft_txt2):
		continue

	with open(fft_txt1) as f:
		b = f.readlines()
	l1 = [float(x.strip()) for x in b if x.strip()]

	with open(fft_txt2) as f:
		b = f.readlines()
	l2 = [float(x.strip()) for x in b if x.strip()]

	if len(l1) != len(l2):
		print(f"len diff : {os.path.basename(fft_txt1)}")
		continue
	
	for i in range(len(l1)):
		d = abs(l1[i] - l2[i])
		if d > THRESHOLD:
			print(f"diff {os.path.basename(fft_txt1)} : {d}")
