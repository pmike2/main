#!/usr/bin/env python3

import datetime
from decimal import *

import streamlit as st
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


st.set_page_config(layout="wide")

global conn
conn = st.connection("postgresql", type="sql")


selection = st.segmented_control(
	"A37",
	["Eleves", "Modeles", "Cours", "Ajout seance", "Ajout paiement eleve", "Ajout paiement intervenant", "Comptes"],
	selection_mode="single",
)


if selection == "Eleves":
	col1, col2, col3 = st.columns(3)

	col1.write("Eleves")
	req = 'SELECT nom, prenom, mail, phone FROM eleve ORDER BY nom'
	df = conn.query(req)
	event = col1.dataframe(df, hide_index=True, on_select="rerun", selection_mode=["single-row-required"], width="stretch")
	nom = df.loc[event.selection["rows"][0], "nom"]
	prenom = df.loc[event.selection["rows"][0], "prenom"]

	col2.write("Cours")
	req2 = f"SELECT s.date, c.intitule AS cours from seance s JOIN participation p ON p.id_seance=s.id JOIN eleve e ON p.id_eleve=e.id JOIN cours c ON s.id_cours=c.id WHERE e.nom='{nom}' AND e.prenom='{prenom}'"
	df2 = conn.query(req2)
	col2.dataframe(df2, hide_index=True, width="stretch")

	col3.write("Paiements")
	req3 = f"SELECT p.date, p.intervenant_ou_a37 AS paiement, f.nom AS forfait, c.intitule AS cours, f.n_seances FROM paiement p JOIN forfait f ON p.id_forfait=f.id JOIN cours c ON f.id_cours=c.id JOIN eleve e ON p.id_eleve=e.id WHERE e.nom='{nom}' AND e.prenom='{prenom}'"
	df3 = conn.query(req3)
	col3.dataframe(df3, hide_index=True, width="stretch", column_config={"n_seances" : None})

	d = {}
	for row in df2.itertuples():
		if row.cours not in d.keys():
			d[row.cours] = 0
		d[row.cours] -= 1
	
	for row in df3.itertuples():
		if row.cours not in d.keys():
			d[row.cours] = 0
		d[row.cours] += row.n_seances
	
	for key, value in d.items():
		col3.write(f"Reste {value} cours pour {key}")


if selection == "Modeles":
	req = 'SELECT nom, prenom, mail, phone FROM modele ORDER BY nom'
	df = conn.query(req)
	st.dataframe(df, hide_index=True, width="stretch")


if selection == "Cours":
	col1, col2, col3 = st.columns(3)

	col1.write("Cours")
	req = 'SELECT c.intitule as cours, c.annee, c.duree, c.pourcentage_a37 AS pourcent, i.nom, i.prenom, i.mail, i.phone FROM cours c JOIN intervenant i ON c.id_intervenant=i.id ORDER BY cours'
	df = conn.query(req)
	event = col1.dataframe(df, hide_index=True, on_select="rerun", selection_mode=["single-row-required"], width="stretch")
	cours = df.loc[event.selection["rows"][0], "cours"]
	annee = df.loc[event.selection["rows"][0], "annee"]
	
	col2.write("Forfaits")
	req2 = f"SELECT f.nom, f.n_seances, f.cout_total, f.cout_par_seance AS pu from forfait f JOIN cours c ON f.id_cours=c.id WHERE c.intitule='{cours}' AND c.annee='{annee}'"
	df2 = conn.query(req2)
	col2.dataframe(df2, hide_index=True, width="stretch")

	col3.write("Séances")
	req3 = f"SELECT s.id, s.date, s.notes FROM seance s JOIN cours c ON s.id_cours=c.id WHERE c.intitule='{cours}' AND c.annee='{annee}' ORDER BY s.date"
	df3 = conn.query(req3)
	event_seance = col3.dataframe(df3, hide_index=True, on_select="rerun", selection_mode=["single-row-required"], column_config={"id" : None}, width="stretch")
	id_seance = df3.loc[event_seance.selection["rows"][0], "id"]

	col1b, col2b = st.columns(2)

	req4 = f"SELECT e.nom, e.prenom FROM eleve e JOIN participation p ON p.id_eleve=e.id JOIN seance s ON p.id_seance=s.id WHERE s.id={id_seance} ORDER BY nom"
	df4 = conn.query(req4)
	col1b.dataframe(df4, hide_index=True, width="stretch")

	req5 = f"SELECT m.prenom, p.intervenant_ou_a37 AS paiement, p.montant FROM paiement_modele p JOIN modele m ON p.id_modele=m.id JOIN seance s ON p.id_seance=s.id WHERE s.id={id_seance}"
	df5 = conn.query(req5)
	col2b.dataframe(df5, hide_index=True, width="stretch")


if selection == "Ajout seance":
	col1, col2, col3 = st.columns(3)

	req = "SELECT id, intitule, annee FROM cours"
	df = conn.query(req)
	event_cours = col1.dataframe(df, hide_index=True, on_select="rerun", selection_mode=["single-row-required"], column_config={"id" : None}, width="stretch")
	id_cours = df.loc[event_cours.selection["rows"][0], "id"]

	date = col2.date_input("Date", datetime.date.today())
	notes = col3.text_input("notes")

	col1b, col2b, col3b, col4b = st.columns(4)

	rows_model = db.query("SELECT prenom FROM modele ORDER BY prenom")

	is_model = col1b.checkbox("Modele")
	selected_model = col2b.selectbox("Modèle", [x["prenom"] for x in rows_model], disabled=not is_model)
	intervenant_ou_a37 = col3b.radio("paye par", ["INTERVENANT", "A37"], disabled=not is_model)
	montant = col4b.number_input("Montant", value=40, icon=":material/euro:")

	rows_students = db.query("SELECT prenom FROM eleve ORDER BY prenom")

	selected_students = st.pills("Participants", [x["prenom"] for x in rows_students], selection_mode="multi")

	if st.button("Ajouter"):
		req_insert = f"INSERT INTO seance(id_cours, date, notes) VALUES ({id_cours}, '{date}', '{notes}') RETURNING id"
		row = db.query(req_insert)
		id_seance = row[0]["id"]

		for student in selected_students:
			req_insert_student = f"INSERT INTO participation(id_eleve, id_seance) SELECT id, {id_seance} FROM eleve WHERE prenom='{student}'"
			#print(req_insert_student)
			db.query(req_insert_student, fetch=False)

		if is_model:
			req_insert_model = f"INSERT INTO paiement_modele(id_modele, id_seance, intervenant_ou_a37, montant) SELECT id, {id_seance}, '{intervenant_ou_a37}'::intervenant_ou_a37_type, {montant} FROM modele WHERE prenom='{selected_model}'"
			#print(req_insert_model)
			db.query(req_insert_model, fetch=False)

		db.commit()

		st.toast("Ajout de séance effectué !")


if selection == "Ajout paiement eleve":
	rows_students = db.query("SELECT prenom FROM eleve ORDER BY prenom")
	selected_student = st.selectbox("Eleve", [x["prenom"] for x in rows_students])
	
	date = st.date_input("Date", datetime.date.today())

	req = 'SELECT c.id, c.intitule as cours, c.annee FROM cours c ORDER BY cours'
	df = conn.query(req)
	event_cours = st.dataframe(df, hide_index=True, on_select="rerun", selection_mode=["single-row-required"], column_config={"id" : None}, width="stretch")
	id_cours = df.loc[event_cours.selection["rows"][0], "id"]

	req2 = f"SELECT f.id, f.nom from forfait f JOIN cours c ON f.id_cours=c.id WHERE c.id='{id_cours}'"
	df2 = conn.query(req2)
	event_forfait = st.dataframe(df2, hide_index=True, on_select="rerun", selection_mode=["single-row-required"], column_config={"id" : None}, width="stretch")
	id_forfait = df2.loc[event_forfait.selection["rows"][0], "id"]

	intervenant_ou_a37 = st.radio("payé à", ["INTERVENANT", "A37"])

	if st.button("Ajouter"):
		req_insert_paiement = f"INSERT INTO paiement(id_eleve, id_forfait, date, intervenant_ou_a37) SELECT id, {id_forfait}, '{date}', '{intervenant_ou_a37}'::intervenant_ou_a37_type FROM eleve WHERE prenom = '{selected_student}'"
		#print(req_insert_paiement)
		db.query(req_insert_paiement, fetch=False)

		db.commit()

		st.toast("Ajout de paiement effectué !")


if selection == "Ajout paiement intervenant":
	req = 'SELECT id, nom, prenom FROM intervenant ORDER BY nom'
	df = conn.query(req)
	event_intervenant = st.dataframe(df, hide_index=True, on_select="rerun", selection_mode=["single-row-required"], column_config={"id" : None}, width="stretch")
	id_intervenant = df.loc[event_intervenant.selection["rows"][0], "id"]

	date = st.date_input("Date", datetime.date.today())

	montant = st.number_input("Montant", icon=":material/euro:")

	if st.button("Ajouter"):
		req_insert_paiement = f"INSERT INTO paiement_intervenant(id_intervenant, date, montant) VALUES ({id_intervenant}, '{date}', {montant})"
		#print(req_insert_paiement)
		db.query(req_insert_paiement, fetch=False)

		db.commit()

		st.toast("Ajout de paiement effectué !")


if selection == "Comptes":
	req = 'SELECT c.id, c.intitule as cours, c.annee, c.pourcentage_a37 FROM cours c ORDER BY cours'
	df = conn.query(req)
	event_cours = st.dataframe(df, hide_index=True, on_select="rerun", selection_mode=["single-row-required"], column_config={"id" : None}, width="stretch")
	id_cours = df.loc[event_cours.selection["rows"][0], "id"]
	pourcentage_a37 = df.loc[event_cours.selection["rows"][0], "pourcentage_a37"]

	req_eleve2a37 = f"SELECT COALESCE(SUM(f.cout_total), 0.0) AS total FROM forfait f JOIN paiement p ON p.id_forfait=f.id JOIN cours c ON f.id_cours=c.id WHERE c.id = {id_cours} AND p.intervenant_ou_a37 = 'A37'"
	row = db.query(req_eleve2a37)
	eleve2a37 = row[0]["total"]
	st.write(f"A37 a perçu :blue[{eleve2a37}] :material/euro:")

	req_eleve2inter = f"SELECT COALESCE(SUM(f.cout_total), 0.0) AS total FROM forfait f JOIN paiement p ON p.id_forfait=f.id JOIN cours c ON f.id_cours=c.id WHERE c.id = {id_cours} AND p.intervenant_ou_a37 = 'INTERVENANT'"
	row = db.query(req_eleve2inter)
	eleve2inter = row[0]["total"]
	st.write(f"Intervenant a perçu :green[{eleve2inter}] :material/euro:")

	req_a372model = f"SELECT COALESCE(SUM(p.montant), 0.0) AS total FROM paiement_modele p JOIN seance s ON p.id_seance=s.id JOIN cours c ON s.id_cours=c.id WHERE c.id = {id_cours} AND p.intervenant_ou_a37 = 'A37'"
	row = db.query(req_a372model)
	a372model = row[0]["total"]
	st.write(f"A37 a payé les modèles :red[{a372model}] :material/euro:")

	req_inter2model = f"SELECT COALESCE(SUM(p.montant), 0.0) AS total FROM paiement_modele p JOIN seance s ON p.id_seance=s.id JOIN cours c ON s.id_cours=c.id WHERE c.id = {id_cours} AND p.intervenant_ou_a37 = 'INTERVENANT'"
	row = db.query(req_inter2model)
	inter2model = row[0]["total"]
	st.write(f"Intervenant a payé les modèles :orange[{inter2model}] :material/euro:")

	req_a372inter = f"SELECT COALESCE(SUM(p.montant), 0.0) AS total FROM paiement_intervenant p JOIN intervenant i ON p.id_intervenant = i.id JOIN cours c ON c.id_intervenant = i.id WHERE c.id = {id_cours}"
	row = db.query(req_a372inter)
	a372inter = row[0]["total"]
	st.write(f"A37 a payé l'intervenant :yellow[{a372inter}] :material/euro:")

	#benefice_total = eleve2a37 + eleve2inter - (a372model + inter2model)
	benefice_total = eleve2a37 + eleve2inter
	st.write(f"Le bénéfice total est :blue[{eleve2a37}] + :green[{eleve2inter}] = :violet[{benefice_total}] :material/euro:")

	a_payer2inter = eleve2a37 - a372model - a372inter - benefice_total * Decimal(pourcentage_a37) / Decimal(100.0)
	st.write(f"A37 doit régler à l'intervenant :blue[{eleve2a37}] - :red[{a372model}] - :yellow[{a372inter}] - :violet[{benefice_total}] * {pourcentage_a37} % = **{a_payer2inter}  :material/euro:**")
