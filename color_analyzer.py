#!/usr/bin/env python3

import os, sys, math
from pprint import pprint as pp
import colorsys

import numpy
from osgeo import gdal
from PyQt6.QtWidgets import QApplication, QMainWindow, QLabel, QGridLayout, QWidget
from PyQt6.QtGui import QPixmap, QPainter, QPen, QColor, QPainterPath
from PyQt6.QtCore import Qt


MAX_WIDTH= 500
PT_DIST_THRESH= 40.0
PT_SIZE= 5
PT_SIZE_SMALL= 8
N_HISTO = 5
GET_PIXEL_SIZE= 6


class Histo(QWidget):
	def __init__(self):
		super().__init__()

		self._histo_rgb= []
		self._histo_hsv= []


	def paintEvent(self, e):
		if not self._histo_hsv:
			return
		
		painter= QPainter()
		#painter= QPainter()
		painter.begin(self)
		pen= QPen()
		pen.setWidth(1)
		pen.setColor(QColor(100, 100, 100, 200))
		painter.setPen(pen)
		painter.setBrush(QColor(200, 250, 10, 200))
		#painter.drawEllipse(50, 50, 20, 20)

		for i in range(N_HISTO):
			x= i*30
			y= 20
			w= 20
			h= int(self._histo_hsv[i][2]* 100)
			painter.drawRect(x, y, w, h)

		painter.end()


class ColorAnalyzer(QWidget):

	def __init__(self):
		super().__init__()

		#self._img_path= "/Users/home/Desktop/pm/paint/DSC_5036.JPG"
		self._img_path= "/Users/home/Desktop/pm/prog/color_test.jpg"

		self._ds= gdal.Open(self._img_path)
		#self._data= ds.ReadAsArray()

		self._im= QPixmap(self._img_path)
		self._width, self._height, self._ratio= self._im.width(), self._im.height(), 1.0
		if self._width> MAX_WIDTH:
			self._ratio= float(MAX_WIDTH)/ self._width
			self._height= int(self._height* self._ratio)
			self._width= MAX_WIDTH
			self._im= self._im.scaled(self._width, self._height, Qt.AspectRatioMode.KeepAspectRatio)

		self._label = QLabel()
		self._label.setPixmap(self._im)

		self._histo_w= Histo()
		#self._histo_w.setGeometry(0, 0, 100, 100)
		#self._histo_w.resize(800, 600)

		self._grid = QGridLayout()
		self._grid.setContentsMargins(0, 0, 0, 0)
		self._grid.addWidget(self._label, 1, 1)
		self._grid.addWidget(self._histo_w, 1, 2)
		
		self.setLayout(self._grid)

		self.setMouseTracking(True)

		self.setGeometry(50, 50, 800, 200)
		self.setWindowTitle("ColorAnalyzer")
		self.show()

		self._pts= []
		self._pts_histo= []
		self._mouse_pressed= False


	def get_pixel(self, x, y, size):
		x_ratioed, y_ratioed= x/ self._ratio, y/ self._ratio
		a= self._ds.ReadAsArray(x_ratioed- size* 0.5, y_ratioed- size* 0.5, size, size)
		l= a.tolist()
		res= []
		for idx_band in range(3):
			value= 0.0
			for row in l[idx_band]:
				for pix in row:
					value+= pix
			value/= size* size
			res.append(value)
		
		res= [x/ 255.0 for x in res]

		return res
	

	def compute_histo(self):
		self._pts_histo= []
		path_lengths= [math.dist(self._pts[i], self._pts[i+ 1]) for i in range(len(self._pts)- 1)]
		total_path_length= sum(path_lengths)
		for idx_histo in range(N_HISTO- 1):
			x, t, idx_path= 0.0, None, None
			for i in range(len(path_lengths)):
				x+= path_lengths[i]
				diff= x- total_path_length* idx_histo/ (N_HISTO- 1)
				if diff>= 0.0:
					t= 1.0- diff/ path_lengths[i]
					idx_path= i
					break
			#print(f"{x}, {t}, {idx_path}")
			v= (self._pts[idx_path+ 1][0]- self._pts[idx_path][0], self._pts[idx_path+ 1][1]- self._pts[idx_path][1])
			x, y= self._pts[idx_path][0]+ v[0]* t, self._pts[idx_path][1]+ v[1]* t
			self._pts_histo.append((x, y))
		
		self._pts_histo.append(self._pts[-1])

		self._histo_w._histo_rgb= [self.get_pixel(x, y, GET_PIXEL_SIZE) for x, y in self._pts_histo]
		self._histo_w._histo_hsv= [colorsys.rgb_to_hsv(r, g, b) for r, g, b in self._histo_w._histo_rgb]
		
		#pp(self._pts_histo)
		pp(self._histo_w._histo_rgb)
		pp(self._histo_w._histo_hsv)


	def mousePressEvent(self, e):
		x= int(e.position().x())
		y= int(e.position().y())
		#print(f'x: {x},  y: {y}')

		self._mouse_pressed= True

		#self.init_label()
		self._label.setPixmap(self._im)

		self._pts= [(x, y)]
		
	
	def mouseMoveEvent(self, e):
		if not self._mouse_pressed:
			return
		
		x= int(e.position().x())
		y= int(e.position().y())
		#print(f'x: {x},  y: {y}')

		d= math.dist(self._pts[-1], (x, y))
		if d> PT_DIST_THRESH:
			self._pts.append((x, y))


	def mouseReleaseEvent(self, e):
		x= int(e.position().x())
		y= int(e.position().y())
		#print(f'x: {x},  y: {y}')

		self._mouse_pressed= False
		self.compute_histo()


	def paintEvent(self, e):
		if not self._pts:
			return
		
		canvas= self._label.pixmap()
		painter= QPainter(canvas)
		pen= QPen()
		pen.setWidth(1)
		pen.setColor(QColor(100, 100, 100, 200))
		painter.setPen(pen)

		path= QPainterPath()
		path.moveTo(self._pts[0][0], self._pts[0][1])
		for pt in self._pts:
			path.lineTo(pt[0], pt[1])

		painter.drawPath(path)

		#painter.setBrush(QColor(100, 100, 100, 200))
		#painter.drawEllipse(int(self._pts[0][0]- PT_SIZE* 0.5), int(self._pts[0][1]- PT_SIZE* 0.5), PT_SIZE, PT_SIZE)
		if not self._mouse_pressed:
			#painter.drawEllipse(int(self._pts[-1][0]- PT_SIZE* 0.5), int(self._pts[-1][1]- PT_SIZE* 0.5), PT_SIZE, PT_SIZE)
			
			painter.setBrush(QColor(200, 100, 100, 100))
			#for i in range(1, len(self._pts)- 1):
			#	painter.drawEllipse(int(self._pts[i][0]- PT_SIZE_SMALL* 0.5), int(self._pts[i][1]- PT_SIZE_SMALL* 0.5), PT_SIZE_SMALL, PT_SIZE_SMALL)
			painter.setBrush(QColor(100, 200, 100, 100))
			for i in range(len(self._pts_histo)):
				painter.drawEllipse(int(self._pts_histo[i][0]- PT_SIZE_SMALL* 0.5), int(self._pts_histo[i][1]- PT_SIZE_SMALL* 0.5), PT_SIZE_SMALL, PT_SIZE_SMALL)

		painter.end()
		self._label.setPixmap(canvas)

		self._histo_w.update()


	def closeEvent(self, e):
		self._ds= None


if __name__ == '__main__':
	app= QApplication(sys.argv)
	ca= ColorAnalyzer()
	sys.exit(app.exec())

#ds= gdal.Open('/Users/home/Desktop/pm/paint/DSC_5036.JPG')
#data= ds.ReadAsArray()
#pp(data)
#ds = None

