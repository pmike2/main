#!/usr/bin/env python3

import os, sys, re
import subprocess

import soundfile as sf
import numpy as np


def conv(in1, in2, out):
	data1, sample_rate1 = sf.read(in1)
	data2, sample_rate2 = sf.read(in2)

	#print(sample_rate1)
	#print(sample_rate2)

	left1, right1= np.hsplit(data1, 2)
	left1, right1= np.squeeze(left1), np.squeeze(right1)

	left2, right2= np.hsplit(data2, 2)
	left2, right2= np.squeeze(left2), np.squeeze(right2)

	left_conv= np.convolve(left1, left2)
	right_conv= np.convolve(right1, right2)

	left_conv= left_conv/ left_conv.max()
	right_conv= right_conv/ right_conv.max()

	conv= np.stack((left_conv, right_conv), axis=-1)

	sample_rate_conv= sample_rate1
	sf.write(out, conv, sample_rate_conv)


def test_1sample_multireverbs():
	sample2process= "/Volumes/Data/perso/son/loops/2022_12_02_006.wav"
	root_reverb= "/Volumes/Data/perso/son/test_convolution/Impulses/04_Made_for_Drums"
	reverbs= [os.path.join(root_reverb, x) for x in sorted(os.listdir(root_reverb)) if not x.startswith(".")]
	for reverb in reverbs:
		print(reverb)
		out= os.path.join("/Volumes/Data/perso/son/test_convolution/sample_vs_impulse", f"test1_{os.path.basename(os.path.splitext(reverb)[0])}.wav")
		conv(sample2process, reverb, out)


def test_sample_vs_samples():
	sample2process= "/Volumes/Data/perso/son/loops/2022_12_02_006.wav"
	root_samples= "/Volumes/Data/perso/son/loops"
	samples= [os.path.join(root_samples, x) for x in sorted(os.listdir(root_samples)) if not x.startswith(".")]
	for sample2 in samples:
		print(sample2)
		out= os.path.join("/Volumes/Data/perso/son/test_convolution/sample_vs_sample", f"test1_{os.path.basename(os.path.splitext(sample2)[0])}.wav")
		conv(sample2process, sample2, out)


test_1sample_multireverbs()
#test_sample_vs_samples()
