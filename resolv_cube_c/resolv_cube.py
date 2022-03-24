#!/usr/bin/env python
# -*- coding:utf-8 -*- 

import os, sys, re, math
from multiprocessing import Pool, cpu_count


LONG= "LONG"
COURT= "COURT"
SEGMENTS= [LONG, LONG, LONG, LONG, COURT, COURT, COURT, LONG, LONG, COURT, COURT, LONG, COURT, LONG, COURT, COURT, LONG]


def cube_idx2xyz(idx):
	x= idx % 3
	y= ((idx- x)/ 3) % 3
	z= (idx- x- 3* y)/ 9
	return (x, y, z)
	

def cube_xyz2idx(x, y, z):
	return x+ 3* y+ 9* z


def get_digits(n, b):
	x= n
	res= [ ]
	while True:
		res.append(x % b)
		x/= b
		if x== 0:
			break
	res.reverse()
	return res


def next_indices(idx, instr, seg_type):
	coords= [ ]
	x, y, z= cube_idx2xyz(idx)
	if instr[1]== "-":
		eps= -1
	elif instr[1]== "+":
		eps= 1
	if instr[0]== "x":
		coords.append((x+ eps, y, z))
		if seg_type== LONG:
			coords.append((x+ eps* 2, y, z))
	elif instr[0]== "y":
		coords.append((x, y+ eps, z))
		if seg_type== LONG:
			coords.append((x, y+ eps* 2, z))
	elif instr[0]== "z":
		coords.append((x, y, z+ eps))
		if seg_type== LONG:
			coords.append((x, y, z+ eps* 2))
	for xyz in coords:
		for coord in xyz:
			if (coord< 0) or (coord> 2):
				return None

	return [ cube_xyz2idx(x, y, z) for x, y, z in coords ]


def next_instruction(instr, digit):
	if instr[0]== "x":
		return ["y+", "y-", "z+", "z-"][digit]
	elif instr[0]== "y":
		return ["x+", "x-", "z+", "z-"][digit]
	elif instr[0]== "z":
		return ["x+", "x-", "y+", "y-"][digit]


def test_compt(compt, indices0, instr0):
	current_idx= indices0[-1]
	instructions= [instr0]
	cube= [False]* 27
	for idx in indices0:
		cube[idx]= True
	digits= get_digits(compt, 4)

	for idx_segment in range(len(SEGMENTS)- 1):
		if idx_segment>= len(digits):
			digit= 0
		else:
			digit= digits[idx_segment]
		instr= next_instruction(instructions[-1], digit)
		instructions.append(instr)
		indices= next_indices(current_idx, instr, SEGMENTS[idx_segment+ 1])
		if indices is None:
			return False
		for idx in indices:
			if cube[idx] is True:
				return False
			cube[idx]= True
		current_idx= indices[-1]
	
	return True


def main():
	for x0, y0 in ((0, 0), (1, 0), (1, 1)):
		instr0= "z+"
		indices0= [cube_xyz2idx(x0, y0, 0)]
		indices0+= next_indices(cube_xyz2idx(x0, y0, 0), instr0, SEGMENTS[0])
		
		for compt in xrange(int(math.pow(4, len(SEGMENTS)- 1))):
			if compt % 10000000== 0:
				print compt

			if test_compt(compt, indices0, instr0):
				print x0, y0, compt


def pool_cb(is_solution):
	if is_solution:
		print "ok"


def main_multi():
	#print cpu_count()
	pool= Pool(processes=8)
	#pool= Pool()
	for x0, y0 in ((0, 0), (1, 0), (1, 1)):
		instr0= "z+"
		indices0= [cube_xyz2idx(x0, y0, 0)]
		indices0+= next_indices(cube_xyz2idx(x0, y0, 0), instr0, SEGMENTS[0])
		
		for compt in xrange(int(math.pow(4, len(SEGMENTS)- 1))):
			if compt % 10000000== 0:
				print compt
			
			pool.apply_async(test_compt, (compt, indices0, instr0), callback=pool_cb)
	
	pool.close()
	pool.join()
	

# ----------------------------------------------------------------------------------------
#import timeit
#print timeit.timeit("cube_idx2xyz(26)", number=1000000, setup="from __main__ import cube_idx2xyz")
#print timeit.timeit("cube_xyz2idx(2, 2, 2)", number=1000000, setup="from __main__ import cube_xyz2idx")
#print timeit.timeit("get_digits(4294967295, 4)", number=1000000, setup="from __main__ import get_digits")
#print timeit.timeit("next_indices(1, 'x-', COURT)", number=1000000, setup="from __main__ import next_indices, COURT")
#print timeit.timeit("next_instruction('x+', 2)", number=1000000, setup="from __main__ import next_instruction")

#print cube_idx2xyz(26)
#print cube_xyz2idx(2, 2, 2)
#print get_digits(4294967295, 4)
#print next_indices(1, "x-", COURT)
#print next_instruction("x+", 2)

main()
#main_multi()

