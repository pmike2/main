#!/usr/bin/env python3

import os
import math
import time
import argparse
from multiprocessing import Process, active_children

import soundfile as sf
import numpy as np
#import matplotlib.pyplot as plt
import pyqtgraph as pg
import pyqtgraph.exporters


DEFAULT_NPROCS= 8


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
	for idx, bloc in enumerate(blocs):
		fft_log= os.path.join(log_dir, f"{bloc_base_index+ idx:04}.log")
		if os.path.isfile(fft_log):
			continue

		# calcul fft
		fft_bloc= np.fft.fft(bloc)

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


def bloc_plot_pyqtgraph(bloc_group, logarithm, log_dir, png_dir):
	blocs= bloc_group["data"]
	bloc_base_index= bloc_group["base_idx"]

	# fond blanc
	pg.setConfigOption('background', 'w')

	for idx, bloc in enumerate(blocs):
		fft_log= os.path.join(log_dir, f"{bloc_base_index+ idx:04}.log")
		fft_png= os.path.join(png_dir, f"{bloc_base_index+ idx:04}.png")
		if os.path.isfile(fft_png):
			continue
	
		# lecture fichier
		data= np.genfromtxt(fft_log)

		#plt= pg.plot(data[:, 0], data[:, 1], pen=None, symbol='o')
		plt= pg.plot(data[:, 0], data[:, 1], pen=pg.mkPen((30, 50, 50), width=1), symbol=None)
		
		plt.setXRange(0, 20000)
		if logarithm:
			plt.setYRange(-10, 10)
		else:
			plt.setYRange(0, 1000)
		
		exporter= pg.exporters.ImageExporter(plt.plotItem)
		#exporter.parameters()['width']= 100   # (note this also affects height parameter)
		
		os.system(f"rm {fft_png} 2>/dev/null")
		exporter.export(fft_png)


def wav_fft(wav, keep_logs=False, keep_pngs=False, clean_first=False, logarithm=False, bloc_size=None, n_procs=DEFAULT_NPROCS):
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
		n_blocs= math.ceil(len(left)/ bloc_size)
		for i in range(n_blocs- 1):
			blocs.append(left[i* bloc_size:(i+ 1)* bloc_size])
		# il faut padder le dernier bloc avec des 0
		last_bloc= left[(n_blocs- 1)* bloc_size:n_blocs* bloc_size]
		blocs.append(np.pad(last_bloc, (0, bloc_size- len(last_bloc)), 'constant', constant_values=(0.0, )))

	# groupes de blocs a traiter sur chaque process
	blocs_groups= get_process_groups(blocs, n_procs)

	# calcul FFT
	for bloc_group in blocs_groups:
		p= Process(target=bloc_fft, args=(bloc_group, sample_rate, logarithm, log_dir))
		p.start()

	while active_children():
		time.sleep(1)

	# génération pngs
	for bloc_group in blocs_groups:
		p= Process(target=bloc_plot_pyqtgraph, args=(bloc_group, logarithm, log_dir, png_dir))
		p.start()

	while active_children():
		time.sleep(1)

	# assemblage des png en un mp4 ; on ajuste video_rate pour que la durée de la video soit celle de l'audio
	out_mp4_without_audio= os.path.join(mp4_dir, os.path.splitext(os.path.basename(wav))[0]+ "_tmp.mp4")
	video_rate= sample_rate/ bloc_size
	os.system(f"ffmpeg -hide_banner -loglevel error -y -f image2 -r {video_rate} -i {png_dir}/%004d.png -vcodec mpeg4 -vb 20M {out_mp4_without_audio}")

	# ajout audio
	out_mp4_with_audio= os.path.join(mp4_dir, os.path.splitext(os.path.basename(wav))[0]+ ".mp4")
	os.system(f"ffmpeg -hide_banner -loglevel error -y -i {out_mp4_without_audio} -i {wav} -map 0 -map 1:a -c:v copy -shortest {out_mp4_with_audio}")

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
	args= parser.parse_args()

	l_wavs= [args.wav] if os.path.isfile(args.wav) else [os.path.join(args.wav, x) for x in os.listdir(args.wav) if os.path.splitext(x)[1].lower()== ".wav" and not x.startswith(".")]
	for wav in l_wavs:
		print(wav)
		wav_fft(wav, keep_logs=args.keep_logs, keep_pngs=args.keep_pngs, clean_first=args.clean_first, logarithm=args.logarithm, bloc_size=args.bloc_size, n_procs=args.nprocs)


if __name__== "__main__":
	main()
