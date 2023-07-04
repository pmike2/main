#!/usr/bin/env python3

import os, sys
import math
from pprint import pprint as pp

import mido

with mido.open_input('Gestionnaire IAC Bus test') as inport:
	for msg in inport:
		print(msg)

#print(mido.get_output_names())
