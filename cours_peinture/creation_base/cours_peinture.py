#!/usr/bin/env python3

import datetime
from pprint import pprint as pp

import psycopg2
import psycopg2.extras


class PgDatabase():
	"""Base de données PostgreSQL."""
	def __init__(self, host, dbname):
		"""Constructeur."""
		self._conn = psycopg2.connect("host=%s port=%s dbname=%s user=%s" % (host, "5432", dbname, "pmbeau2"))
		self._curs = self._conn.cursor(cursor_factory=psycopg2.extras.RealDictCursor)
	
	def close(self):
		"""Fermeture accès."""
		self._curs.close()
		self._conn.close()
	
	def query(self, query, fetch=True):
		"""Requete."""
		self._curs.execute(query)
		if fetch:
			return [dict(row) for row in self._curs.fetchall()]

	def commit(self):
		"""Commit requete."""
		self._conn.commit()


db = PgDatabase("127.0.0.1", "a37")


def get_eleve_id(prenom):
	res = db.query(f"SELECT id FROM eleve WHERE prenom = '{prenom}'")
	if len(res) != 1:
		raise RuntimeError(f"ERR get_eleve_id {prenom}")
	return res[0]['id']


def get_modele_id(prenom):
	res = db.query(f"SELECT id FROM modele WHERE prenom = '{prenom}'")
	if len(res) != 1:
		raise RuntimeError(f"ERR get_modele_id {prenom}")
	return res[0]['id']


def get_seance_id(d):
	res = db.query(f"SELECT id FROM seance WHERE date = '{d}'")
	if len(res) != 1:
		raise RuntimeError(f"ERR get_seance_id {d}")
	return res[0]['id']


def get_cours_id(cours_intitule, cours_annee):
	res = db.query(f"SELECT id FROM cours WHERE intitule = '{cours_intitule}' AND annee = '{cours_annee}'")
	if len(res) != 1:
		raise RuntimeError(f"ERR get_cours_id {cours_intitule} / {cours_annee}")
	return res[0]['id']


def get_forfait_id(nom, cours_intitule, cours_date):
	id_cours = get_cours_id(cours_intitule, cours_date)
	res = db.query(f"SELECT id FROM forfait WHERE nom = '{nom}' AND id_cours = {id_cours}")
	if len(res) != 1:
		raise RuntimeError(f"ERR get_forfait_id {nom} / {cours_intitule} / {cours_date}")
	return res[0]['id']


def insert_participations():
	with open("/Volumes/Data/perso/dev/main/cours_peinture/creation_base/participation.txt") as f:
		b = f.readlines()

	header = b[0].split("\t")
	seances = {}
	for line in b[1:]:
		ls = line.split("\t")
		d = str(datetime.datetime.strptime(ls[0], "%d/%m/%y").date())
		seances[d] = []
		for idx, eleve in enumerate(header[4:]):
			if ls[idx + 4].strip():
				seances[d].append(eleve.strip())

	#pp(seances)

	for d, eleves in seances.items():
		id_seance = get_seance_id(d)
		for eleve in eleves:
			try:
				id_eleve = get_eleve_id(eleve)
			except:
				print(f"ERR {eleve}")
				continue
		
			db.query(f"INSERT INTO participation(id_eleve, id_seance) VALUES ({id_eleve}, {id_seance})", False)

	db.commit()


def insert_paiements():
	l = [ 
		["Ghilaine", "decouverte", "2026-03-12", "A37"],
		["Denis", "decouverte", "2026-03-12", "A37"],
		["Clémentine", "decouverte", "2026-03-12", "A37"],
		["Sandrine", "decouverte", "2026-03-12", "A37"],
		["Frederic", "decouverte", "2026-03-12", "A37"],
		["Danielle", "decouverte", "2026-03-12", "A37"],

		["Ghilaine", "10_cours", "2026-03-19", "A37"],
		["Clémentine", "10_cours", "2026-03-19", "A37"],
		["Denis", "4_cours", "2026-03-19", "A37"],

		["Sandrine", "1_cours", "2026-04-09", "A37"],

		["Denis", "4_cours", "2026-04-16", "A37"],
		["Olivier", "4_cours", "2026-04-16", "A37"],

		["Sandrine", "1_cours", "2026-05-07", "INTERVENANT"],
		["SandrineFille", "1_cours", "2026-05-07", "INTERVENANT"],

		["Denis", "4_cours", "2026-06-04", "A37"],

		#["Ghilaine", "10_cours", "2026-09-17", "A37"],
	]

	cours_intitule = "Portrait alla prima"
	for eleve, nom, date, type_paiment in l:
		id_forfait = get_forfait_id(nom, cours_intitule, "2026-01-01")
		id_eleve = get_eleve_id(eleve)
		db.query(f"INSERT INTO paiement(id_eleve, id_forfait, date, intervenant_ou_a37) VALUES ({id_eleve}, {id_forfait}, '{date}', '{type_paiment}'::intervenant_ou_a37_type)", False)

	db.commit()


def insert_paiements_modeles():
	l = [
		["Aurore", "2026-03-12", "A37"],
		["Aurore", "2026-03-19", "A37"],
		["Nina", "2026-03-26", "A37"],
		["Marcabrune", "2026-04-09", "A37"],
		["Ludovic", "2026-05-28", "INTERVENANT"],
		#["Marcabrune", "2026-09-17", "INTERVENANT"],
	]
	for modele, date, type_paiment in l:
		id_modele = get_modele_id(modele)
		id_seance = get_seance_id(date)
		db.query(f"INSERT INTO paiement_modele(id_modele, id_seance, intervenant_ou_a37) VALUES ({id_modele}, {id_seance}, '{type_paiment}'::intervenant_ou_a37_type)", False)

	db.commit()


#print(get_eleve_id("Ghilaine"))
#print(get_seance_id('2026-03-12'))
insert_participations()
#insert_paiements()
#insert_paiements_modeles()
