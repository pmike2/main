#!/usr/bin/env python
# -*- coding:utf-8 -*-

# Comparaison de 2 dossiers contenant le code perso

import os, sys
from subprocess import Popen, PIPE


def cmp_roots(root1, root2):
	for r1, d1s, f1s in os.walk(root1):
		dir_ok= True
		for d in (".git", ".vscode", "sandbox"):
			if d in r1:
				dir_ok= False
				break
		if not dir_ok:
			continue
		
		for f1 in f1s:
			if f1.startswith("."):
				continue
			if os.path.splitext(f1)[1].upper() not in (".C", ".CPP", ".H", ".HPP", ".PY", ".SH", ".PNG", ".JPG", ".XCF", ".XML", ".JSON", ".TXT", ".OBJ", ".MTL", "DAE", "BLEND", ".TTF") and f1!= "Makefile":
				continue
			
			f1_abs= os.path.join(r1, f1)
			f2_abs= f1_abs.replace(root1, root2)
			if not os.path.isfile(f2_abs):
				print "%s n'existe pas" % f2_abs
				continue
			cmd= "diff %s %s" % (f1_abs, f2_abs)
			pop= Popen(cmd, shell=True, bufsize=-1, stdout=PIPE)
			pipe= pop.stdout
			buf= pipe.read()
			pipe.close()
			if buf.strip():
				print "diff %s %s" % (f1_abs, f2_abs)
				#print buf.strip()


assert len(sys.argv)== 3, "Donner les 2 dossiers a comparer"

root1, root2= sys.argv[1:]
cmp_roots(root1, root2)
cmp_roots(root2, root1)
