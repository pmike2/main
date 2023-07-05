#!/usr/bin/env python3

import os
import math
import time
import json
import argparse
from multiprocessing import Process, active_children

import soundfile as sf
import numpy as np
#import matplotlib.pyplot as plt
import pyqtgraph as pg
import pyqtgraph.exporters


DEFAULT_NPROCS= 8
POINT_MODE= "point"
LINE_MODE= "line"
HISTO_MODE= "histo"
DEFAULT_DRAW_MODE= LINE_MODE


def get_process_groups(l, n_procs):
	n= len(l)
	if n< n_procs:
		compts= [1]* n
	else:
		d= n// n_procs
		r= n- d* n_procs
		compts= [d]* n_procs
		for i in range(r):
			compts[i]+= 1

	groups= [ ]
	total_compt= 0
	for compt in compts:
		groups.append({"data" : l[total_compt:total_compt+ compt], "base_idx" : total_compt})
		total_compt+= compt

	return groups


def bloc_fft(bloc_group, sample_rate, logarithm, log_dir):
	blocs= bloc_group["data"]
	bloc_base_index= bloc_group["base_idx"]
	window= np.hanning(len(blocs[0]))
	
	for idx, bloc in enumerate(blocs):
		fft_log= os.path.join(log_dir, f"{bloc_base_index+ idx:04}.log")
		if os.path.isfile(fft_log):
			continue

		# calcul fft ; on applique une fenetre avant la FFT
		fft_bloc= np.fft.fft(bloc* window)

		# les fréquences qui vont être en axe X
		data_x= np.fft.fftfreq(n=fft_bloc.size, d=1/sample_rate)

		# le module des complexes vont être en Y ; on pourrait aussi analyser la phase des complexes
		data_y= np.abs(fft_bloc)

		# à ce niveau les tableaux ont dans leur 2eme partie les fréquences négatives ; on les ignore
		# donc en réalité les logs vont etre de taille --bloc-size / 2
		n_div_2= len(data_x)// 2
		data_x= data_x[:n_div_2]
		data_y= data_y[:n_div_2]

		# peak_coefficient est l'indice du module le plus grand ; peak_freq est la fréquence fondamentale
		peak_coefficient= np.argmax(data_y)
		peak_freq= data_x[peak_coefficient]

		# cette option réduit les écarts d'amplitude et permet de mieux voir
		if logarithm:
			data_y= np.log(data_y)

		# fichier de log
		os.system(f"rm {fft_log} 2>/dev/null")
		with open(fft_log, "w") as f:
			for i in range(len(data_x)):
				f.write(f"{data_x[i]}\t{data_y[i]}\n")


def general_info(log_dir, bloc_size, logarithm):
	log_info= os.path.join(log_dir, "_infos.json")
	if not os.path.isfile(log_info):
		info= {"bloc_size" : bloc_size, "logarithm" : logarithm}
		fft_logs= [os.path.join(log_dir, x) for x in os.listdir(log_dir) if os.path.splitext(x)[1].lower()== ".log" and not x.startswith(".")]
		xmin_total, xmax_total, ymin_total, ymax_total= 1e8, -1e8, 1e8, -1e8
		for fft_log in fft_logs:
			data= np.genfromtxt(fft_log)
			xmin, xmax= min(data[:, 0]), max(data[:, 0])
			ymin, ymax= min(data[:, 1]), max(data[:, 1])
			if xmin< xmin_total:
				xmin_total= xmin
			if xmax> xmax_total:
				xmax_total= xmax
			if ymin< ymin_total:
				ymin_total= ymin
			if ymax> ymax_total:
				ymax_total= ymax
		info["xmin"], info["xmax"], info["ymin"], info["ymax"]= xmin_total, xmax_total, ymin_total, ymax_total

		with open(log_info, "w") as f:
			f.write(json.dumps(info, indent=4))
	
	with open(log_info) as f:
		info= json.load(f)

	return info
	

# TROP LENT
# def bloc_plot_matplotlib(fft_log, fft_png, logarithm, tresh):
# 	if os.path.isfile(fft_png):
# 		return
	
# 	# lecture fichier
# 	data= np.genfromtxt(fft_log)
	
# 	# nettoyage figure
# 	plt.figure().clear()
# 	plt.close()
# 	plt.cla()
# 	plt.clf()
	
# 	# affichage uniquement si amplitude > tresh
# 	plt.xlim(0, 20000)
# 	if logarithm:
# 		color= [(0.0, 0.0, 0.0, 1.0) if x> math.log(tresh) else (0.0, 0.0, 0.0, 0.0) for x in data[:, 1]]
# 		plt.ylim(-10, 10)
# 	else:
# 		color= [(0.0, 0.0, 0.0, 1.0) if x> tresh else (0.0, 0.0, 0.0, 0.0) for x in data[:, 1]]
# 		plt.ylim(0, 1000)

# 	plt.scatter(data[:, 0], data[:, 1], s=1.0, c=color)
# 	#plt.show()
# 	os.system(f"rm {fft_png} 2>/dev/null")
# 	plt.savefig(fft_png)


def bloc_plot_pyqtgraph(bloc_group, log_dir, png_dir, draw_mode, xmin, xmax, ymin, ymax):
	blocs= bloc_group["data"]
	bloc_base_index= bloc_group["base_idx"]

	# fond blanc
	pg.setConfigOption('background', 'w')
	# antialiasing
	pg.setConfigOptions(antialias=True)

	for idx, bloc in enumerate(blocs):
		fft_log= os.path.join(log_dir, f"{bloc_base_index+ idx:04}.log")
		fft_png= os.path.join(png_dir, f"{bloc_base_index+ idx:04}.png")
		if os.path.isfile(fft_png):
			continue
	
		# lecture fichier
		data= np.genfromtxt(fft_log)

		# cf https://pyqtgraph.readthedocs.io/en/latest/api_reference/graphicsItems/plotdataitem.html
		color= (50, 50, 80)
		if draw_mode== POINT_MODE:
			plt= pg.plot(data[:, 0], data[:, 1], pen=None, symbol='o', symbolSize=2, symbolPen=None, symbolBrush=pg.mkBrush(color))
		elif draw_mode== LINE_MODE:
			plt= pg.plot(data[:, 0], data[:, 1], pen=pg.mkPen(color, width=1), symbol=None)
		elif draw_mode== HISTO_MODE:
			# en mode histo il faut que len(x) == len(y) + 1
			plt= pg.plot(np.append(data[:, 0], data[:, 0][-1]+ data[:, 0][1]), data[:, 1], stepMode="center", fillLevel=0, fillOutline=True, brush=color)
		
		plt.setXRange(xmin, xmax)
		plt.setYRange(ymin, ymax)
		
		exporter= pg.exporters.ImageExporter(plt.plotItem)
		#exporter.parameters()['width']= 100   # (note this also affects height parameter)
		
		os.system(f"rm {fft_png} 2>/dev/null")
		exporter.export(fft_png)


def wav_fft(wav, keep_logs=False, keep_pngs=False, clean_first=False, logarithm=False, bloc_size=None, n_procs=DEFAULT_NPROCS, draw_mode=DEFAULT_DRAW_MODE):
	# lecture wav
	data, sample_rate= sf.read(wav)

	# dossiers log et png intermédiaires
	root_dir= os.path.splitext(wav)[0]+ "_FFT"
	log_dir= os.path.join(root_dir, "logs")
	png_dir= os.path.join(root_dir, "pngs")
	mp4_dir= os.path.join(root_dir, "mp4")
	for d in (log_dir, png_dir, mp4_dir):
		if clean_first:
			os.system(f"rm -rf {d} 2>/dev/null")
		if not os.path.exists(d):
			os.system(f"mkdir -p {d}")

	# séparation left / right
	left, right= np.hsplit(data, 2)

	# [ [a], [b], ...] => [a, b, ...]
	left, right= np.squeeze(left), np.squeeze(right)

	# constitution des blocs
	if bloc_size is None:
		blocs= [left]
	else:
		blocs= []
		idx= 0
		while True:
			blocs.append(left[idx:idx+ bloc_size])
			# pour avoir de l'overlap de 50% on déplace de bloc_size//2
			idx+= bloc_size//2
			if idx+ bloc_size== len(left):
				break
			elif idx+ bloc_size> len(left):
				# il faut padder le dernier bloc avec des 0
				last_bloc= left[idx:len(left)- 1]
				blocs.append(np.pad(last_bloc, (0, bloc_size- len(last_bloc)), 'constant', constant_values=(0.0, )))
				break

	# groupes de blocs a traiter sur chaque process
	blocs_groups= get_process_groups(blocs, n_procs)

	# calcul FFT
	for bloc_group in blocs_groups:
		p= Process(target=bloc_fft, args=(bloc_group, sample_rate, logarithm, log_dir))
		p.start()

	while active_children():
		time.sleep(1)

	info= general_info(log_dir, bloc_size, logarithm)
	xmin, xmax= 0, 12000
	ymin, ymax= info["ymin"], info["ymax"]

	# génération pngs
	for bloc_group in blocs_groups:
		p= Process(target=bloc_plot_pyqtgraph, args=(bloc_group, log_dir, png_dir, draw_mode, xmin, xmax, ymin, ymax))
		p.start()

	while active_children():
		time.sleep(1)

	# assemblage des png en un mp4 ; on ajuste video_rate pour que la durée de la video soit celle de l'audio
	# comme on fait un overlap de 50% on a 2 fois plus de pngs donc il faut * sample_rate/ bloc_size par 2
	out_mp4_without_audio= os.path.join(mp4_dir, os.path.splitext(os.path.basename(wav))[0]+ "_tmp.mp4")
	video_rate= 2* sample_rate/ bloc_size
	os.system(f"ffmpeg -hide_banner -loglevel error -y -f image2 -r {video_rate} -i {png_dir}/%004d.png -vcodec mpeg4 -vb 20M {out_mp4_without_audio}")

	# ajout audio
	out_mp4_with_audio= os.path.join(mp4_dir, os.path.splitext(os.path.basename(wav))[0]+ ".mp4")
	os.system(f"ffmpeg -hide_banner -loglevel error -y -i {out_mp4_without_audio} -i {wav} -map 0 -map 1:a -c:v copy {out_mp4_with_audio}")

	# nettoyage
	os.system(f"rm {out_mp4_without_audio}")

	if not keep_pngs:
		os.system(f"rm -rf {png_dir}")

	if not keep_logs:
		os.system(f"rm -rf {log_dir}")


def main():
	parser= argparse.ArgumentParser(description="FFT")
	parser.add_argument("wav", help="chemin wav file ou dossier contenant des wavs")
	parser.add_argument("--keep-logs", action="store_true", help="on conserve les logs")
	parser.add_argument("--keep-pngs", action="store_true", help="on conserve les pngs")
	parser.add_argument("--clean-first", action="store_true", help="on repart de 0")
	parser.add_argument("--logarithm", action="store_true", help="affichage logarithmique")
	parser.add_argument("--bloc-size", default=None, type=int, help="taille bloc à analyser")
	parser.add_argument("--nprocs", default=DEFAULT_NPROCS, type=int, help="nombre de processeurs")
	parser.add_argument("--draw-mode", default=DEFAULT_DRAW_MODE, type=str, help="mode de dessin")
	args= parser.parse_args()

	l_wavs= [args.wav] if os.path.isfile(args.wav) else [os.path.join(args.wav, x) for x in os.listdir(args.wav) if os.path.splitext(x)[1].lower()== ".wav" and not x.startswith(".")]
	for wav in l_wavs:
		print(wav)
		wav_fft(
			wav,
			keep_logs=args.keep_logs,
			keep_pngs=args.keep_pngs,
			clean_first=args.clean_first,
			logarithm=args.logarithm,
			bloc_size=args.bloc_size,
			n_procs=args.nprocs,
			draw_mode=args.draw_mode
		)


if __name__== "__main__":
	main()
