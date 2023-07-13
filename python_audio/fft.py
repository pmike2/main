#!/usr/bin/env python3

import os
import math
import random
import time
import json
import argparse
from multiprocessing import Process, active_children

import soundfile as sf
import numpy as np
import pyqtgraph as pg
import pyqtgraph.exporters
from PIL import Image


DEFAULT_NPROCS= 8
POINT_MODE= "point"
LINE_MODE= "line"
HISTO_MODE= "histo"
DEFAULT_DRAW_MODE= LINE_MODE
# pour délimiter ymax on ignore les bins situés avant cet idx
# sinon on a toutes les grandes amplitudes des basses fréquences qui empechent de bien voir les autres
DEFAULT_START_IDX_YMAX= 0
DEFAULT_BLOC_SIZE= 2048
DEFAULT_HOP_SIZE= 1024


class FFT:
	def __init__(self, wav, keep_logs=False, keep_pngs=False, clean_first=False, logarithm=False, bloc_size=DEFAULT_BLOC_SIZE,
		hop_size=DEFAULT_HOP_SIZE, n_procs=DEFAULT_NPROCS, draw_mode=DEFAULT_DRAW_MODE, start_idx_ymax=DEFAULT_START_IDX_YMAX, debug=False):
		self._wav= wav
		self._keep_logs= keep_logs
		self._keep_pngs= keep_pngs
		self._clean_first= clean_first
		self._logarithm= logarithm
		self._bloc_size= bloc_size
		self._hop_size= hop_size
		self._n_procs= n_procs
		self._draw_mode= draw_mode
		self._start_idx_ymax= start_idx_ymax
		self._debug= debug

		self._root_dir= os.path.splitext(wav)[0]+ "_FFT"
		self._log_dir= os.path.join(self._root_dir, "logs")
		self._png_dir= os.path.join(self._root_dir, "pngs")
		self._result_dir= os.path.join(self._root_dir, "result")
		self._debug_dir= os.path.join(self._root_dir, "debug")
		self._log_info= os.path.join(self._root_dir, "infos.json")
		self._mp4= os.path.join(self._result_dir, os.path.splitext(os.path.basename(self._wav))[0]+ ".mp4")
		self._mp4_without_audio= os.path.splitext(self._mp4)[0]+ "_no_audio.mp4"
		self._obj= os.path.join(self._result_dir, os.path.splitext(os.path.basename(self._wav))[0]+ ".obj")
		self._png= os.path.join(self._result_dir, os.path.splitext(os.path.basename(self._wav))[0]+ ".png")

		self._blocs= []
		self._blocs_groups= []
		self._fft_logs= []
		self._nbins= self._bloc_size// 2

		os.system(f"rm {self._log_info} 2>/dev/null")

		for d in (self._log_dir, self._png_dir, self._result_dir, self._debug_dir):
			if self._clean_first:
				os.system(f"rm -rf {d} 2>/dev/null")
			if not os.path.exists(d):
				os.system(f"mkdir -p {d}")
		
		# lecture wav
		self._wav_data, self._sample_rate= sf.read(self._wav)

		# construction des blocs de données sur lesquels appliquer une FFT
		self.get_blocs()

		# groupes de blocs a traiter sur chaque process
		self.get_blocs_groups()

		# calcul FFT
		for bloc_group in self._blocs_groups:
			p= Process(target=self.bloc_fft, args=(bloc_group, ))
			p.start()

		while active_children():
			time.sleep(1)
		
		self._fft_logs= [os.path.join(self._log_dir, x) for x in os.listdir(self._log_dir) if os.path.splitext(x)[1].lower()== ".log" and not x.startswith(".")]

		# recup infos générales
		self.general_info()

		# génération pngs
		for bloc_group in self._blocs_groups:
			p= Process(target=self.bloc_plot_pyqtgraph, args=(bloc_group, ))
			p.start()

		while active_children():
			time.sleep(1)
		
		# création mp4
		self.gen_mp4()

		# obj file
		self.gen_obj()

		# raster file
		self.gen_png()

		# nettoyage
		if not self._keep_pngs:
			os.system(f"rm -rf {self._png_dir}")

		if not self._keep_logs:
			os.system(f"rm -rf {self._log_dir}")
		
		if not self._debug:
			os.system(f"rm -rf {self._debug_dir}")


	def get_blocs(self):
		# séparation left / right
		left, right= np.hsplit(self._wav_data, 2)

		# [ [a], [b], ...] => [a, b, ...]
		left, right= np.squeeze(left), np.squeeze(right)

		self._blocs= []
		idx= 0
		while True:
			self._blocs.append(left[idx:idx+ self._bloc_size])
			# overlap == bloc_size- hop_size ; si hop_size == bloc_size, pas d'overlap, si hop_size == bloc_size// 2, 50% d'overlap
			idx+= self._hop_size
			if idx+ self._bloc_size== len(left):
				break
			elif idx+ self._bloc_size> len(left):
				# il faut padder le dernier bloc avec des 0
				last_bloc= left[idx:len(left)- 1]
				self._blocs.append(np.pad(last_bloc, (0, self._bloc_size- len(last_bloc)), 'constant', constant_values=(0.0, )))
				break


	def get_blocs_groups(self):
		n= len(self._blocs)
		if n< self._n_procs:
			compts= [1]* n
		else:
			d= n// self._n_procs
			r= n- d* self._n_procs
			compts= [d]* self._n_procs
			for i in range(r):
				compts[i]+= 1

		self._blocs_groups= [ ]
		total_compt= 0
		for compt in compts:
			self._blocs_groups.append({"data" : self._blocs[total_compt:total_compt+ compt], "base_idx" : total_compt})
			total_compt+= compt


	def bloc_fft(self, bloc_group):
		blocs= bloc_group["data"]
		bloc_base_index= bloc_group["base_idx"]
		window= np.hanning(len(blocs[0]))
		
		for idx, bloc in enumerate(blocs):
			fft_log= os.path.join(self._log_dir, f"{bloc_base_index+ idx:04}.log")
			debug_log= os.path.join(self._debug_dir, f"{bloc_base_index+ idx:04}.log")
			if os.path.isfile(fft_log):
				continue

			# on enleve le DC offset
			# pour plus de précision utiliser from scipy.signal import detrend ; cf https://forecastegy.com/posts/detrending-time-series-data-python
			detrended= bloc- np.mean(bloc)
			# on applique une fenetre de type Hanning
			windowed= detrended* window

			#windowed= bloc* window
			#detrended= windowed- np.mean(windowed)
			
			if self._debug:
				with open(debug_log, "w") as f:
					for x in windowed:
						f.write(f"{x}\n")

			# calcul fft
			fft_bloc= np.fft.fft(windowed)

			# les fréquences qui vont être en axe X
			data_x= np.fft.fftfreq(n=fft_bloc.size, d=1/ self._sample_rate)

			# le module des complexes vont être en Y ; on pourrait aussi analyser la phase des complexes
			data_y= np.abs(fft_bloc)

			# à ce niveau les tableaux ont dans leur 2eme partie les fréquences négatives ; on les ignore
			# donc en réalité les logs vont etre de taille --bloc-size / 2 == self._nbins
			data_x= data_x[:self._nbins]
			data_y= data_y[:self._nbins]

			# peak_coefficient est l'indice du module le plus grand ; peak_freq est la fréquence fondamentale
			peak_coefficient= np.argmax(data_y)
			peak_freq= data_x[peak_coefficient]

			# cette option réduit les écarts d'amplitude et permet de mieux voir
			if self._logarithm:
				data_y= 20* np.log(data_y)
				#data_y= pow(data_y, 0.5)

			# fichier de log
			os.system(f"rm {fft_log} 2>/dev/null")
			with open(fft_log, "w") as f:
				for i in range(len(data_x)):
					f.write(f"{data_x[i]}\t{data_y[i]}\n")


	def general_info(self):
		if not os.path.isfile(self._log_info):
			info= {}
			
			for key, val in self.__dict__.items():
				if isinstance(val, (str, int, float, bool)):
					info[key[1:]]= val
			
			xmin_total, xmax_total, ymin_total, ymax_total= 1e8, -1e8, 1e8, -1e8
			for fft_log in self._fft_logs:
				data= np.genfromtxt(fft_log)
				xmin, xmax= min(data[:, 0]), max(data[:, 0])
				ymin, ymax= min(data[:, 1]), max(data[self._start_idx_ymax:, 1])
				if xmin< xmin_total:
					xmin_total= xmin
				if xmax> xmax_total:
					xmax_total= xmax
				if ymin< ymin_total:
					ymin_total= ymin
				if ymax> ymax_total:
					ymax_total= ymax
			info["xmin"], info["xmax"], info["ymin"], info["ymax"]= xmin_total, xmax_total, ymin_total, ymax_total

			with open(self._log_info, "w") as f:
				f.write(json.dumps(info, indent=4))
		
		with open(self._log_info) as f:
			self._info= json.load(f)
	

	def bloc_plot_pyqtgraph(self, bloc_group):
		blocs= bloc_group["data"]
		bloc_base_index= bloc_group["base_idx"]

		# fond blanc
		pg.setConfigOption('background', 'w')
		# antialiasing
		pg.setConfigOptions(antialias=True)

		for idx, bloc in enumerate(blocs):
			fft_log= os.path.join(self._log_dir, f"{bloc_base_index+ idx:04}.log")
			fft_png= os.path.join(self._png_dir, f"{bloc_base_index+ idx:04}.png")
			if os.path.isfile(fft_png):
				continue
		
			# lecture fichier
			data= np.genfromtxt(fft_log)

			# cf https://pyqtgraph.readthedocs.io/en/latest/api_reference/graphicsItems/plotdataitem.html
			color= (50, 50, 80)
			if self._draw_mode== POINT_MODE:
				plt= pg.plot(data[:, 0], data[:, 1], pen=None, symbol='o', symbolSize=2, symbolPen=None, symbolBrush=pg.mkBrush(color))
			elif self._draw_mode== LINE_MODE:
				plt= pg.plot(data[:, 0], data[:, 1], pen=pg.mkPen(color, width=1), symbol=None)
			elif self._draw_mode== HISTO_MODE:
				# en mode histo il faut que len(x) == len(y) + 1
				plt= pg.plot(np.append(data[:, 0], data[:, 0][-1]+ data[:, 0][1]), data[:, 1], stepMode="center", fillLevel=0, fillOutline=True, brush=color)
			
			plt.setXRange(self._info["xmin"], self._info["xmax"])
			plt.setYRange(self._info["ymin"], self._info["ymax"])
			
			exporter= pg.exporters.ImageExporter(plt.plotItem)
			#exporter.parameters()['width']= 100   # (note this also affects height parameter)
			
			os.system(f"rm {fft_png} 2>/dev/null")
			exporter.export(fft_png)


	def gen_mp4(self):
		# assemblage des png en un mp4 ; on ajuste video_rate pour que la durée de la video soit celle de l'audio
		video_rate= self._sample_rate/ self._hop_size
		os.system(f"ffmpeg -hide_banner -loglevel error -y -f image2 -r {video_rate} -i {self._png_dir}/%004d.png -vcodec mpeg4 -vb 20M {self._mp4_without_audio}")

		# ajout audio
		os.system(f"ffmpeg -hide_banner -loglevel error -y -i {self._mp4_without_audio} -i {self._wav} -map 0 -map 1:a -c:v copy {self._mp4}")

		os.system(f"rm {self._mp4_without_audio}")


	def gen_obj(self):
		delta_freq= 0.001
		delta_time= 0.1
		amp_factor= 0.01
		
		vertices= []
		for idx_log, fft_log in enumerate(self._fft_logs):
			data= np.genfromtxt(fft_log)
			y= idx_log* delta_time
			for idx_freq in range(self._nbins):
				freq, amplitude= data[idx_freq]
				x= freq* delta_freq
				z= amplitude* amp_factor
				vertices.append((x, y, z))
		
		faces= []
		for idx_log in range(len(self._fft_logs)- 1):
			for idx_bin in range(self._nbins- 1):
				# idx vertex dans obj file commence à 1, pas à 0
				i= idx_log* self._nbins+ idx_bin+ 1
				j= i+ 1
				k= i+ self._nbins+ 1
				faces.append((i, j, k))
				j= i+ self._nbins+ 1
				k= i+ self._nbins
				faces.append((i, j, k))
		
		with open(self._obj, "w") as f:
			for x, y, z in vertices:
				f.write(f"v {x} {y} {z}\n")
			for i, j, k in faces:
				f.write(f"f {i} {j} {k}\n")


	def gen_png(self):
		amplitudes= np.zeros((len(self._fft_logs), self._nbins))
		for idx_log, fft_log in enumerate(self._fft_logs):
			data= np.genfromtxt(fft_log)
			amplitudes[idx_log]= data[:, 1]

		#print(all_amplitudes.shape)
		im= Image.fromarray(np.uint8(255* (amplitudes- np.min(amplitudes))/ (np.max(amplitudes)- np.min(amplitudes))))
		im.save(self._png, "PNG")


def main():
	parser= argparse.ArgumentParser(description="FFT")
	parser.add_argument("wav", help="chemin wav file ou dossier contenant des wavs")
	parser.add_argument("--keep-logs", action="store_true", help="on conserve les logs")
	parser.add_argument("--keep-pngs", action="store_true", help="on conserve les pngs")
	parser.add_argument("--clean-first", action="store_true", help="on repart de 0")
	parser.add_argument("--logarithm", action="store_true", help="affichage logarithmique")
	parser.add_argument("--bloc-size", default=DEFAULT_BLOC_SIZE, type=int, help="taille bloc à analyser")
	parser.add_argument("--hop-size", default=DEFAULT_HOP_SIZE, type=int, help="taille sauts à effectuer entre chaque bloc")
	parser.add_argument("--nprocs", default=DEFAULT_NPROCS, type=int, help="nombre de processeurs")
	parser.add_argument("--draw-mode", default=DEFAULT_DRAW_MODE, type=str, help="mode de dessin")
	parser.add_argument("--start-idx-ymax", default=DEFAULT_START_IDX_YMAX, type=int, help="bins avant cet indice sont ignorés pour délimiter le ymax des png")
	parser.add_argument("--debug", action="store_true", help="debug")
	args= parser.parse_args()

	l_wavs= [args.wav] if os.path.isfile(args.wav) else [os.path.join(args.wav, x) for x in os.listdir(args.wav) if os.path.splitext(x)[1].lower()== ".wav" and not x.startswith(".")]
	for wav in l_wavs:
		print(wav)
		fft= FFT(
			wav,
			keep_logs=args.keep_logs,
			keep_pngs=args.keep_pngs,
			clean_first=args.clean_first,
			logarithm=args.logarithm,
			bloc_size=args.bloc_size,
			hop_size=args.hop_size,
			n_procs=args.nprocs,
			draw_mode=args.draw_mode,
			start_idx_ymax=args.start_idx_ymax,
			debug=args.debug
		)


if __name__== "__main__":
	main()
