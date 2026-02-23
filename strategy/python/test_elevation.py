#!/usr/bin/env python3

import socket
import json

import streamlit as st

print("start")

if "connexion" not in st.session_state:
    st.session_state["connexion"] = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    st.session_state["connexion"].connect(("127.0.0.1", 8080))

if 'age' not in st.session_state:
    st.session_state['age'] = 0

def age_cbck():
    print(st.session_state['age'])
    #st.session_state["connexion"].send(json.dumps({"age" : st.session_state['age']}).encode("utf-8"))
    st.session_state["connexion"].send("haha\0".encode("utf-8"))
    st.write("I'm ", st.session_state['age'], "years old")

st.slider("Howxxx old are you?", 0, 130, 25, key="age", on_change=age_cbck)
