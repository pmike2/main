#!/usr/bin/env python3

import os, sys

with open(sys.argv[1]) as f:
	b=f.readlines()

l=[int(x.strip()) for x in b]
for i in range(len(l)-1):
	print(l[i+1]-l[i])

