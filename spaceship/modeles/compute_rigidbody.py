#!/usr/bin/env python
# -*- coding:utf-8 -*-

import os, sys, re, math

# script à l'origine pour spaceship
# sert à calculer la matrice d'inertie d'un .obj
#

# nombre de subdivs dans les 3 dimensions
NSUBDIV= 60

# utilisé par verif pour faire un .obj avec des ptits cubes
VAL= 0.1
CUBE_VERTICES =	[
	VAL, -VAL, -VAL,
	VAL, -VAL, VAL,
	-VAL, -VAL, VAL,
	-VAL, -VAL, -VAL,
	VAL, VAL, -VAL,
	VAL, VAL, VAL,
	-VAL, VAL, VAL,
	-VAL, VAL, -VAL
]

CUBE_FACES= [
	2, 4, 1,
	8, 6, 5,
	5, 2, 1,
	6, 3, 2,
	3, 8, 4,
	1, 8, 5,
	2, 3, 4,
	8, 7, 6,
	5, 6, 2,
	6, 7, 3,
	3, 7, 8,
	1, 4, 8
]


# pt 3D
class Pt(object):
	def __init__(self, x, y, z):
		self.x= x
		self.y= y
		self.z= z
	
	def __repr__(self):
		return "(%.4f, %.4f, %.4f)" % (self.x, self.y, self.z)

	def __str__(self):
		return self.__repr__()


# produit scalaire
def scalar(pt1, pt2):
	return pt1.x* pt2.x+ pt1.y* pt2.y+ pt1.z* pt2.z


# diff de 2 pts
def vecdiff(pt1, pt2):
	return Pt(pt1.x- pt2.x, pt1.y- pt2.y, pt1.z- pt2.z)
	

# lecture du .obj en faces
def parse_obj(ch_obj):
	f= open(ch_obj)
	b= f.readlines()
	f.close()

	pts= [ ]
	faces_idx= [ ]
	for line in b:
		if line.strip().startswith("v"):
			ls= map(lambda x : float(x), line.strip().split()[1:])
			pts.append(Pt(ls[0], ls[1], ls[2]))
		if line.strip().startswith("f"):
			# commence à 1
			faces_idx.append(map(lambda x : int(x)- 1, line.strip().split()[1:]))

	#print len(pts)
	#print len(faces_idx)

	faces= [ ]
	for face_idx in faces_idx:
		faces.append([pts[face_idx[0]], pts[face_idx[1]], pts[face_idx[2]]])

	return faces


# passage idx -> point 3D
def idx2pos(i, j, k, radius):
	step= 2.0* radius/ NSUBDIV
	x= -radius+ step* i+ step* 0.5
	y= -radius+ step* j+ step* 0.5
	z= -radius+ step* k+ step* 0.5
	return Pt(x, y, z)
	

# renvoie rayon
def get_extent(faces):
	xmin, xmax, ymin, ymax, zmin, zmax= 1e8, -1e8, 1e8, -1e8, 1e8, -1e8
	radius= -1e8
	for face in faces:
		for i in range(3):
			if face[i].x< xmin: xmin= face[i].x
			if face[i].x> xmax: xmax= face[i].x
			if face[i].y< ymin: ymin= face[i].y
			if face[i].y> ymax: ymax= face[i].y
			if face[i].z< zmin: zmin= face[i].z
			if face[i].z> zmax: zmax= face[i].z

	if radius< abs(xmin): radius= abs(xmin)
	if radius< abs(xmax): radius= abs(xmax)
	if radius< abs(ymin): radius= abs(ymin)
	if radius< abs(ymax): radius= abs(ymax)
	if radius< abs(zmin): radius= abs(zmin)
	if radius< abs(zmax): radius= abs(zmax)

	#return (xmin, xmax, ymin, ymax, zmin, zmax, radius)
	return radius


# renvoie un booléen : est-ce que le segment intersecte la face
def segment_intersect_face(seg, face):
	# calcul normale à la face
	u= vecdiff(face[1], face[0])
	v= vecdiff(face[2], face[0])
	norm= Pt(
		u.y* v.z- v.y* u.z,
		v.x* u.z- u.x* v.z,
		u.x* v.y- v.x* u.y
	)
	
	scal1= scalar(norm, vecdiff(face[0], seg[0]))
	scal2= scalar(norm, vecdiff(seg[1] , seg[0]))
	# segment et face parallèles
	if abs(scal2)< 1e-8:
		return False
	
	scaldiv= scal1/ scal2
	if scaldiv< 0.0 or scaldiv> 1.0:
		return False

	proj= Pt(
		seg[0].x+ scaldiv* (seg[1].x- seg[0].x), 
		seg[0].y+ scaldiv* (seg[1].y- seg[0].y), 
		seg[0].z+ scaldiv* (seg[1].z- seg[0].z),
	)
	
	w= vecdiff(proj, face[0])
	
	uv= scalar(u, v)
	uu= scalar(u, u)
	vv= scalar(v, v)
	vw= scalar(v, w)
	uw= scalar(u, w)
	denom= uv* uv- uu* vv
	s= (uv* vw- vv* uw)/ denom
	t= (uv* uw- uu* vw)/ denom
	
	if s>= 0 and t>= 0 and s+ t<= 1:
		return True
	
	return False
	

# est-ce que le point est à l'intérieur de l'obj
def is_inside(pt, faces):
	pt_inf= Pt(
		100.0+ pt.x* 1000.0,
		100.0+ pt.y* 1000.0,
		100.0+ pt.z* 1000.0
	)
	seg= [pt, pt_inf]
	count= 0
	for face in faces:
		#print face
		if segment_intersect_face(seg, face):
			#print "segment_intersect_face", seg, face
			count+= 1
	#print count
	if count % 2== 0:
		return False

	return True
	

# est-ce que le centre de chaque ptit cube est dans l'objet
def indices_inside(faces, radius, ch_txt):
	to_w= ""
	for i in range(NSUBDIV):
		for j in range(NSUBDIV):
			for k in range(NSUBDIV):
				print i, j, k
				pt= idx2pos(i, j, k, radius)
				if is_inside(pt, faces):
					#print i, j, k
					to_w+= "%s\t%s\t%s\n" % (i, j, k)
	
	f= open(ch_txt, "w")
	f.write(to_w)
	f.close()


# calcul matrice d'inertie
def compute_inertia_matrix(ch_txt, radius):
	m= [
		0., 0., 0.,
		0., 0., 0.,
		0., 0., 0.
	]

	# je choisis la masse de sorte que la matrice d'inertie ait des coeffs proches de 1 sur la diagonale
	mass= 12.0
	submass= mass/ (NSUBDIV* NSUBDIV* NSUBDIV)
	step= 2.0* radius/ NSUBDIV
	
	f= open(ch_txt)
	b= f.readlines()
	f.close()
	for line in b:
		if not line.strip():
			continue

		i, j, k= map(lambda x : int(x), line.strip().split())
		pt= idx2pos(i, j, k, radius)
		
		m[0]+= (pt.y* pt.y+ pt.z* pt.z)* submass;
		m[1]+= -pt.x* pt.y* submass;
		m[2]+= -pt.x* pt.z* submass;
		m[3]+= -pt.x* pt.y* submass;
		m[4]+= (pt.x* pt.x+ pt.z* pt.z)* submass;
		m[5]+= -pt.y* pt.z* submass;
		m[6]+= -pt.x* pt.z* submass;
		m[7]+= -pt.y* pt.z* submass;
		m[8]+= (pt.x* pt.x+ pt.y* pt.y)* submass;
	
	return m


# calcul volume
def compute_volume(ch_txt, radius):
	volume= 0.0

	f= open(ch_txt)
	b= f.readlines()
	f.close()
	for line in b:
		if not line.strip():
			continue

		i, j, k= map(lambda x : int(x), line.strip().split())
		volume+= (2.0* radius/ NSUBDIV)* (2.0* radius/ NSUBDIV)* (2.0* radius/ NSUBDIV)
	
	return volume


# calcul barycentre
def compute_bary(ch_txt, radius):
	bary= Pt(0.0, 0.0, 0.0)
	n= 0
	step= 2.0* radius/ NSUBDIV
	
	f= open(ch_txt)
	b= f.readlines()
	f.close()
	for line in b:
		if not line.strip():
			continue

		i, j, k= map(lambda x : int(x), line.strip().split())
		pt= idx2pos(i, j, k, radius)
		
		bary.x+= pt.x; bary.y+= pt.y; bary.z+= pt.z;
		n+= 1
	
	bary.x/= n; bary.y/= n; bary.z/= n;
	
	return bary
	

# ----------------------------------------------------------------------------------------
# écriture du txt contenant sur chaque ligne un triplet de ptit cube dans l'intérieur de l'objet
def main1():
	ch_obj= sys.argv[1]
	ch_txt= os.path.join(os.path.dirname(ch_obj), "indices_inside_%s.txt" % NSUBDIV)
	faces= parse_obj(ch_obj)
	radius= get_extent(faces)

	indices_inside(faces, radius, ch_txt)


# symmétrisation du txt par rapport à la 1ère coordonnée. En effet le .obj de spaceship pointe vers y+, les ailes sur l'axe x
def simmetrify():
	ch_obj= sys.argv[1]
	ch_txt= os.path.join(os.path.dirname(ch_obj), "indices_inside_%s.txt" % NSUBDIV)
	faces= parse_obj(ch_obj)
	radius= get_extent(faces)
	f= open(ch_txt)
	b= f.readlines()
	f.close()
	idxs= [ ]
	for line in b:
		if not line.strip():
			continue
		idxs.append([int(x) for x in line.split()])
	
	new_idxs= [ ]
	for i, j, k in idxs:
		key= "%s_%s_%s" % (i, j, k)
		if key not in new_idxs:
			new_idxs.append(key)
		symkey= "%s_%s_%s" % (NSUBDIV- i- 1, j, k)
		if symkey not in new_idxs:
			new_idxs.append(symkey)
	
	new_idxs.sort()
	new_idxs= [ [int(x.split("_")[0]), int(x.split("_")[1]), int(x.split("_")[2])] for x in new_idxs ]
	to_w= ""
	for i, j, k in new_idxs:
		to_w+= "%s\t%s\t%s\n" % (i, j, k)
	ch_txt_sym= os.path.join(os.path.dirname(ch_obj), "indices_inside_sym_%s.txt" % NSUBDIV)
	f= open(ch_txt_sym, "w")
	f.write(to_w)
	f.close()
	

# calcul matrice etc en fonction du txt symmétrisé
def main2():
	ch_obj= sys.argv[1]
	ch_txt= os.path.join(os.path.dirname(ch_obj), "indices_inside_sym_%s.txt" % NSUBDIV)
	faces= parse_obj(ch_obj)
	radius= get_extent(faces)

	inertia_matrix= compute_inertia_matrix(ch_txt, radius)
	volume= compute_volume(ch_txt, radius)
	bary= compute_bary(ch_txt, radius)
	ch_result= os.path.splitext(ch_obj)[0]+ ".txt"
	to_w= ""
	to_w+= "V %s\n" % volume
	to_w+= "B %s %s %s\n" % (bary.x, bary.y, bary.z)
	to_w+= "I %s %s %s %s %s %s %s %s %s\n" % (
		inertia_matrix[0], inertia_matrix[1], inertia_matrix[2],
		inertia_matrix[3], inertia_matrix[4], inertia_matrix[5],
		inertia_matrix[6], inertia_matrix[7], inertia_matrix[8])
	f= open(ch_result, "w")
	f.write(to_w)
	f.close()


# génération d'un obj avec les ptits cubes du txt
def verif():
	ch_obj= sys.argv[1]
	faces= parse_obj(ch_obj)
	radius= get_extent(faces)
	ch_txt= os.path.join(os.path.dirname(ch_obj), "indices_inside_sym_%s.txt" % NSUBDIV)
	f= open(ch_txt)
	b= f.readlines()
	f.close()
	
	to_w= ""
	count= 0
	for line in b:
		if not line.strip():
			continue
		
		i, j, k= map(lambda x : int(x), line.strip().split())
		pt= idx2pos(i, j, k, radius)
		
		to_w+= "\n"
		to_w+= "o cube_%s_%s_%s\n" % (i, j, k)

		for i in range(len(CUBE_VERTICES)/ 3):
			x= CUBE_VERTICES[3* i+ 0]
			y= CUBE_VERTICES[3* i+ 1]
			z= CUBE_VERTICES[3* i+ 2]
			
			to_w+= "v %s %s %s\n" % (pt.x+ x, pt.y+ y, pt.z+ z)
	
		for i in range(len(CUBE_FACES)/ 3):
			to_w+= "f %s %s %s\n" % (8* count+ CUBE_FACES[3* i], 8* count+ CUBE_FACES[3* i+ 1], 8* count+ CUBE_FACES[3* i+ 2])

		count+= 1
		#break
	
	ch_obj_result= os.path.join(os.path.dirname(ch_obj), "indices_inside_verif_%s.obj" % NSUBDIV)
	f= open(ch_obj_result, "w")
	f.write(to_w)
	f.close()
	

# ----------------------------------------------------------------------------------------
# tests
def test1():
	seg= [
		Pt(-1.0, 6.0, 4.6),
		Pt(1000.0, 1000.0, 1000.0)
	]
	face= [
		Pt(-6.1678, 1.4360, -0.4767),
		Pt(-6.1692, 0.5595, -0.6677),
		Pt(-6.3667, 1.0902, -0.4969)
	]
	print segment_intersect_face(seg, face)


def test2():
	p0= Pt(0.0, 0.0, 0.0)
	p1= Pt(1.0, 0.0, 0.0)
	p2= Pt(0.0, 1.0, 0.0)
	p3= Pt(0.0, 0.0, 1.0)

	face1= [p0, p1, p2]
	face2= [p0, p1, p3]
	face3= [p0, p2, p3]
	face4= [p1, p2, p3]

	faces= [face1, face2, face3, face4]

	pt= Pt(0.1, 0.1, 0.1)

	print is_inside(pt, faces)


def test3():
	ch_obj= sys.argv[1]
	faces= parse_obj(ch_obj)
	radius= get_extent(faces)
	i, j, k= 0, 1, 0
	pt= idx2pos(i, j, k, radius)
	print is_inside(pt, faces)

	

# ----------------------------------------------------------------------------------------
#main1()
#simmetrify()
#main2()
#verif()

#test1()
#test2()
#test3()
