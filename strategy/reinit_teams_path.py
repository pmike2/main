#!/usr/bin/env python3

import os
import json
from copy import deepcopy as deep

teams_json = "/Volumes/Data/perso/dev/main/strategy/data/maps/last_map/teams.json"

default_path = {
	"goal": [0.0, 0.0, 0.0],
	"idx_path": 0,
	"intervals": [],
	"intervals_los": [],
	"nodes": [],
	"pts": [],
	"pts_los": [],
	"start": [0.0, 0.0, 0.0],
	"use_line_of_sight": False
}

default_status = "WAITING"

with open(teams_json) as f:
	teams_data = json.load(f)

for team in teams_data["teams"]:
	for unit in team["units"]:
		unit["path"] = deep(default_path)
		unit["status"] = default_status

with open(teams_json, 'w') as f:
	json.dump(teams_data, f, ensure_ascii=False, indent=4)
