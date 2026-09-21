#!/usr/bin/env python3

import streamlit as st


conn = st.connection("postgresql", type="sql")

#left, middle, right = st.columns(3)

selection = st.segmented_control(
	"A37",
	["Eleves", "Cours", "Autre"],
	selection_mode="single",
)

#if 'show_eleve' not in st.session_state:
#	st.session_state.show_eleve = False

#def show_hide_eleve():
#	st.session_state.show_eleve = not st.session_state.show_eleve

if selection == "Eleves":
	col1, col2 = st.columns(2)
	#st.button('Eleves', on_click=show_hide_eleve)
	df = conn.query('SELECT id, nom, prenom FROM eleve', ttl="10m")
	#if st.session_state.show_eleve:
	for row in df.itertuples():
		#st.write(f"{row.nom} / {row.prenom}")
		if col1.button(f"{row.prenom} {row.nom}", key=f"button_eleve_{row.id}"):
			#print(row.nom)
			df = conn.query(f"SELECT * FROM eleve WHERE id={row.id}")
			#print(df.to_dict())
			for key in ("nom", "prenom", "mail", "phone"):
				col2.write(df[key][0])
			pass


def f():
	placeholder = st.empty()

	if left.button("Eleve", width="stretch"):
		#left.markdown("eleve.")
		with placeholder.container():
			df = conn.query('SELECT * FROM eleve', ttl="10m")
			for row in df.itertuples():
				st.write(f"{row.nom} / {row.prenom}")
				if st.button(row.nom, key=f"button_eleve_{row.id}"):
					#print(row.nom)
					pass

	if middle.button("Cours", width="stretch"):
		#middle.markdown("cours.")
		placeholder.empty()

	if right.button("bla", width="stretch"):
		right.markdown("bla.")


