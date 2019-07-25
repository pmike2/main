#!/usr/bin/env python
# -*- coding:utf-8 -*-

import os, sys, re, math
from functools import partial

import json

from OSC import OSCClient, OSCMessage

from Tkinter import *
from ttk import *
from tkFileDialog import *


HOST= "127.0.0.1"
PORT= 12000
N_CHANNELS= 8
N_ACTIONS= 4
N_ITEMS= 16


class A2VConfig(object):
	def __init__(self):
		self.root_json= None
		self.vars= {"at_load" : [ ], "channels" : [ ]}

		self.client= OSCClient()
		self.client.connect((HOST, PORT))

		self.main_window= Tk()
		
		self.nb= Notebook(self.main_window)
		self.nb.pack(fill=BOTH, padx=2, pady=3)

		self.frame_general= Frame(self.nb)
		self.nb.add(self.frame_general, text="general")
		
		bouton_load= Button(self.frame_general, text="load", command=self.load)
		bouton_load.pack();
		bouton_save= Button(self.frame_general, text="save", command=self.save)
		bouton_save.pack();
		bouton_sync= Button(self.frame_general, text="sync", command=self.sync)
		bouton_sync.pack();

		self.frame_atload= Frame(self.nb)
		self.nb.add(self.frame_atload, text="at_load")
		
		for idx_item in range(N_ITEMS):
			frame_item= Frame(self.frame_atload)
			frame_item.pack(side=LEFT, padx=10, pady=10)
			
			self.vars["at_load"].append({"active" : IntVar(), "type" : StringVar(), "ch_obj" : StringVar(), "ch_mtl" : StringVar(), "spotlight" : IntVar()})

			self.vars["at_load"][idx_item]["active_obj"]= Checkbutton(frame_item, text="active", variable=self.vars["at_load"][idx_item]["active"], command=partial(self.at_load, idx_item, "active"))
			self.vars["at_load"][idx_item]["active_obj"].pack()

			self.vars["at_load"][idx_item]["type_obj"]= Combobox(frame_item, values=("OBJ", "LIGHT", "VMAT"), textvariable=self.vars["at_load"][idx_item]["type"], state="readonly")
			self.vars["at_load"][idx_item]["type_obj"].bind("<<ComboboxSelected>>", partial(self.at_load, idx_item, "type"))
			self.vars["at_load"][idx_item]["type_obj"].pack()
			
			self.vars["at_load"][idx_item]["frame"]= Frame(frame_item)
			self.vars["at_load"][idx_item]["frame"].pack(side=LEFT, padx=10, pady=10)
			

		self.frames_channels= [ ]
		for idx_channel in range(N_CHANNELS):
			frame_channel= Frame(self.nb)
			self.vars["channels"].append([ ])
			for idx_action in range(N_ACTIONS):
				frame_action= Frame(frame_channel)
				frame_action.pack(side=LEFT, padx=10, pady=10)
				self.vars["channels"][idx_channel].append({
					"active" : IntVar(),
					 "type" : StringVar(),
					 "idx" : DoubleVar(),
					 "target" : StringVar(),
					 "mult_offset" : DoubleVar(),
					 "add_offset" : DoubleVar()
				})
				self.vars["channels"][idx_channel][idx_action]["active_obj"]= Checkbutton(frame_action, text="active", variable=self.vars["channels"][idx_channel][idx_action]["active"], command=partial(self.send_param, idx_channel, idx_action, "active"))
				self.vars["channels"][idx_channel][idx_action]["active_obj"].pack()

				self.vars["channels"][idx_channel][idx_action]["type_obj"]= Combobox(frame_action, values=("OBJ", "LIGHT", "VMAT"), textvariable=self.vars["channels"][idx_channel][idx_action]["type"], state="readonly")
				self.vars["channels"][idx_channel][idx_action]["type_obj"].bind("<<ComboboxSelected>>", partial(self.send_param, idx_channel, idx_action, "type"))
				self.vars["channels"][idx_channel][idx_action]["type_obj"].pack()
				
				self.vars["channels"][idx_channel][idx_action]["idx_obj"]= Scale(frame_action, from_=0, to=N_ITEMS- 1, variable=self.vars["channels"][idx_channel][idx_action]["idx"], command=partial(self.send_param, idx_channel, idx_action, "idx"))
				self.vars["channels"][idx_channel][idx_action]["idx_obj"].pack()

				self.vars["channels"][idx_channel][idx_action]["target_obj"]= Combobox(frame_action, values=[ ], textvariable=self.vars["channels"][idx_channel][idx_action]["target"], state="readonly")
				self.vars["channels"][idx_channel][idx_action]["target_obj"].bind("<<ComboboxSelected>>", partial(self.send_param, idx_channel, idx_action, "target"))
				self.vars["channels"][idx_channel][idx_action]["target_obj"].pack()

				self.vars["channels"][idx_channel][idx_action]["mult_offset_obj"]= Scale(frame_action, from_=0, to=1, variable=self.vars["channels"][idx_channel][idx_action]["mult_offset"], command=partial(self.send_param, idx_channel, idx_action, "mult_offset"))
				self.vars["channels"][idx_channel][idx_action]["mult_offset_obj"].pack()

				self.vars["channels"][idx_channel][idx_action]["add_offset_obj"]= Scale(frame_action, from_=0, to=1, variable=self.vars["channels"][idx_channel][idx_action]["add_offset"], command=partial(self.send_param, idx_channel, idx_action, "add_offset"))
				self.vars["channels"][idx_channel][idx_action]["add_offset_obj"].pack()

			self.nb.add(frame_channel, text="channel "+ str(idx_channel))

		self.main_window.mainloop()

	
	def get_string(self):
		return json.dumps(self.root_json, sort_keys=True, indent=4, separators=(',', ': '))

	
	def load(self):
		ch_json= askopenfilename(title="Ouvrir un json", filetypes=[('json files','.json'),('all files','.*')])
		if ch_json is None:
			return
		f= open(ch_json)
		buf= f.read()
		f.close()
		self.root_json= json.loads(buf)
	

	def save(self):
		if self.root_json is None:
			return
		f= asksaveasfile(mode='w', defaultextension=".json")
		if f is None:
			return
		f.write(self.get_string())
		f.close()
	
	
	def sync(self):
		if self.root_json is None:
			return
		ch_json_tmp= os.path.join(os.environ["HOME"], "a2v_tmp.json")
		f= open(ch_json_tmp, "w")
		f.write(self.get_string())
		f.close()
		self.client.send(OSCMessage("/sync", ch_json_tmp))
	
	
	def at_load(self, *l):
		idx_item, key= l[:2]
		val= self.vars["at_load"][idx_item][key].get()
		
		if key== "type":
			frame_item= self.vars["at_load"][idx_item]["frame"].master
			self.vars["at_load"][idx_item]["frame"].destroy()
			self.vars["at_load"][idx_item]["frame"]= Frame(frame_item)
			self.vars["at_load"][idx_item]["frame"].pack(side=LEFT, padx=10, pady=10)
			
			if val== "OBJ":
				self.vars["at_load"][idx_item]["ch_obj_obj"]= Combobox(self.vars["at_load"][idx_item]["frame"], values=("CUBE", "TORUS"), textvariable=self.vars["at_load"][idx_item]["ch_obj"], state="readonly")
				self.vars["at_load"][idx_item]["ch_obj_obj"].bind("<<ComboboxSelected>>", partial(self.at_load, idx_item, "ch_obj"))
				self.vars["at_load"][idx_item]["ch_obj_obj"].pack()

				self.vars["at_load"][idx_item]["ch_mtl_obj"]= Combobox(self.vars["at_load"][idx_item]["frame"], values=("CUBE", "TORUS"), textvariable=self.vars["at_load"][idx_item]["ch_mtl"], state="readonly")
				self.vars["at_load"][idx_item]["ch_mtl_obj"].bind("<<ComboboxSelected>>", partial(self.at_load, idx_item, "ch_mtl"))
				self.vars["at_load"][idx_item]["ch_mtl_obj"].pack()
			
			elif val== "LIGHT":
				self.vars["at_load"][idx_item]["spotlight_obj"]= Checkbutton(self.vars["at_load"][idx_item]["frame"], text="spotlight", variable=self.vars["at_load"][idx_item]["spotlight"], command=partial(self.at_load, idx_item, "spotlight"))
				self.vars["at_load"][idx_item]["spotlight_obj"].pack()
			
			elif val== "VMAT":
				pass # TODO
		
		# on envoie tout en string et on décode au cas par cas dans a2v
		val= str(val)

		print "at_load", idx_item, key, val
		self.client.send(OSCMessage("/at_load", [idx_item, key, val]))
		
		
	def send_param(self, *l):
		idx_channel, idx_action, key= l[:3]
		val= self.vars["channels"][idx_channel][idx_action][key].get()
		
		if key== "idx":
			val= int(math.floor(val))
		
		if key== "type":
			if val== "OBJ":
				self.vars["channels"][idx_channel][idx_action]["target_obj"]["values"]= ("alpha", "diffuse")
			elif val== "LIGHT":
				self.vars["channels"][idx_channel][idx_action]["target_obj"]["values"]= ("color", "haha")
			elif val== "VMAT":
				self.vars["channels"][idx_channel][idx_action]["target_obj"]["values"]= ("alpha", )
		
		# on envoie tout en string et on décode au cas par cas dans a2v
		val= str(val)
		
		print "param", idx_channel, idx_action, key, val, type(val)
		self.client.send(OSCMessage("/param", [idx_channel, idx_action, key, val]))
		
	

a2vc= A2VConfig()

