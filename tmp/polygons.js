import {deepcopy} from "./utils.js";
import {norm, normalized, diff, scalar_product, orthogonal, inertia_center, angle, rotate, simplify_coords} from "./geom.js";


const N_POINTS_PER_CIRCLE= 16;
const DEBUG= false;
const INSTRUCTIONS= {
	"fond" : {
		"c" : "create circle",
		"p" : "create polygon",
		"r" : "create rectangle",
		"x" : "clear all"
	},
	"point" : {
		"c" : "copy/paste polygon",
		"m" : "move point",
		"p" : "move polygon",
		"x" : "delete point"
	},
	"line" : {
		"a" : "add point",
		"c" : "copy/paste polygon",
		"m" : "move line",
		"p" : "move polygon",
		"r" : "rotate polygon",
		"s" : "scale polygon",
		"x" : "delete point"
	}
};

var svg_main, svg_main_group, svg_emprise;
var svg_width, svg_height;
var polygons= [];
var keypresseds= {};
var editing_mode= null;
var editing_data= null;


export function normalized_coords(x, y) {
	return {"x" : x/ svg_width, "y" : 1.0- y/ svg_height};
}


function clear() {
	polygons= [];
}


function add_polygon() {
	polygons.push([]);
	return polygons.length- 1;
}


function remove_polygon(idx_polygon) {
	polygons.splice(idx_polygon, 1);
}


function duplicate_polygon(idx_polygon) {
	polygons.push(deepcopy(polygons[idx_polygon]));
	move_polygon(polygons.length- 1, {"x" : 0.1, "y" : 0.1});
}

/*
function set_polygon(idx_polygon, polygon) {
	polygons[idx_polygon]= [];
	for (let i=0; i<polygon.length; ++i) {
		if ("x" in polygon[i]) {
			polygons[idx_polygon].push({"x" : polygon[i].x, "y" : polygon[i].y});
		}
		else {
			polygons[idx_polygon].push({"x" : polygon[i][0], "y" : polygon[i][1]});
		}
	}
}
*/

function move_polygon(idx_polygon, v) {
	for (let i=0; i<polygons[idx_polygon].length; ++i) {
		polygons[idx_polygon][i].x+= v.x;
		polygons[idx_polygon][i].y+= v.y;
	}
}


function simplify_polygon(idx_polygon, treshold, min_dist) {
	let simplified= simplify_coords(polygons[idx_polygon], treshold, min_dist);
	polygons[idx_polygon]= simplified;
}


function add_point(idx_polygon, coord) {
	polygons[idx_polygon].push(coord);
	return polygons[idx_polygon].length- 1;
}


function add_point_on_line(idx_polygon, idx_line, coord) {
	polygons[idx_polygon].splice(idx_line+ 1, 0, coord);
}


function remove_point(idx_polygon, idx_point) {
	polygons[idx_polygon].splice(idx_point, 1);
}


function move_point(idx_polygon, idx_point, coord) {
	polygons[idx_polygon][idx_point]= coord;
}


function get_point_idx(svg_id) {
	return [
		parseInt(svg_id.split("polygon_")[1].split("_")[0]),
		parseInt(svg_id.split("point_")[1])
	];
}


function get_line_idx(svg_id) {
	return [
		parseInt(svg_id.split("polygon_")[1].split("_")[0]),
		parseInt(svg_id.split("line_")[1])
	];
}


function get_svg_polygon_id(idx_polygon) {
	return "polygon_"+ idx_polygon;
}


function get_svg_point_id(idx_polygon, idx_point) {
	return "polygon_"+ idx_polygon+ "_point_"+ idx_point;
}


function get_svg_line_id(idx_polygon, idx_line) {
	return "polygon_"+ idx_polygon+ "_line_"+ idx_line;
}


function set_editing_null() {
	editing_mode= null;
	editing_data= null;
}


function update_polygon_group(idx_polygon) {
	let polygon_group= document.getElementById(get_svg_polygon_id(idx_polygon));
	if (polygon_group=== null) {
		polygon_group= document.createElementNS("http://www.w3.org/2000/svg", "g");
		polygon_group.setAttribute("id", get_svg_polygon_id(idx_polygon));
		svg_main_group.appendChild(polygon_group);
	}
	else {
		polygon_group.innerHTML= "";
	}
	
	for (let i=0; i<polygons[idx_polygon].length; ++i) {
		polygon_group.innerHTML+= '<line x1="'+ polygons[idx_polygon][i].x+ '" y1="'+ polygons[idx_polygon][i].y+ 
			'" x2="'+ polygons[idx_polygon][(i+ 1) % polygons[idx_polygon].length].x+
			'" y2="'+ polygons[idx_polygon][(i+ 1) % polygons[idx_polygon].length].y+
			'" class="lines" id="'+ get_svg_line_id(idx_polygon, i)+ '" />\n';
	}
	for (let i=0; i<polygons[idx_polygon].length; ++i) {
		polygon_group.innerHTML+= '<circle cx="'+ polygons[idx_polygon][i].x+ '" cy="'+ polygons[idx_polygon][i].y+ 
			'" class="points" id="'+ get_svg_point_id(idx_polygon, i)+ '" />\n';
	}
	let svg_lines= polygon_group.getElementsByClassName("lines");
	for (let i=0; i<svg_lines.length; ++i) {
		svg_lines[i].addEventListener("mousedown", mouse_down_line);
		svg_lines[i].addEventListener("mouseup", mouse_up_line);
		svg_lines[i].addEventListener("mousemove", mouse_move);
	}
	let svg_points= polygon_group.getElementsByClassName("points");
	for (let i=0; i<svg_points.length; ++i) {
		svg_points[i].addEventListener("mousedown", mouse_down_point);
		svg_points[i].addEventListener("mouseup", mouse_up_point);
		svg_points[i].addEventListener("mousemove", mouse_move);
	}
}


function add_last_point_to_polygon_group(idx_polygon) {
	let polygon_group= document.getElementById(get_svg_polygon_id(idx_polygon));
	let i= polygons[idx_polygon].length- 1;
	
	let line= document.createElementNS("http://www.w3.org/2000/svg", "line");
	polygon_group.insertBefore(line, polygon_group.getElementsByTagName("circle")[0]);
	line.setAttribute("x1", polygons[idx_polygon][i].x);
	line.setAttribute("y1", polygons[idx_polygon][i].y);
	line.setAttribute("x2", polygons[idx_polygon][i- 1].x);
	line.setAttribute("y2", polygons[idx_polygon][i- 1].y);
	line.setAttribute("id", get_svg_line_id(idx_polygon, i));
	line.classList.add("lines");
	line.addEventListener("mousedown", mouse_down_line);
	line.addEventListener("mouseup", mouse_up_line);
	line.addEventListener("mousemove", mouse_move);

	let point= document.createElementNS("http://www.w3.org/2000/svg", "circle");
	polygon_group.appendChild(point);
	point.setAttribute("cx", polygons[idx_polygon][i].x);
	point.setAttribute("cy", polygons[idx_polygon][i].y);
	point.setAttribute("id", get_svg_point_id(idx_polygon, i));
	point.classList.add("points");
	point.addEventListener("mousedown", mouse_down_point);
	point.addEventListener("mouseup", mouse_up_point);
	point.addEventListener("mousemove", mouse_move);
}


function delete_polygon_group(idx_polygon) {
	let polygon2delete= document.getElementById(get_svg_polygon_id(idx_polygon));
	if (polygon2delete!== null) {
		polygon2delete.remove();
	}
	for (let i=idx_polygon+ 1; i<polygons.length ;++i) {
		let polygon= document.getElementById(get_svg_polygon_id(i));
		polygon.setAttribute("id", get_svg_polygon_id(i- 1));
		for (let j=0; j<polygons[i].length; ++j) {
			let point= document.getElementById(get_svg_point_id(i, j));
			point.setAttribute("id", get_svg_point_id(i- 1, j));
		}
		for (let j=0; j<polygons[i].length; ++j) {
			let line= document.getElementById(get_svg_line_id(i, j));
			line.setAttribute("id", get_svg_line_id(i- 1, j));
		}
	}
}


function delete_all_polygon_groups() {
	svg_main_group.innerHTML= "";
}


function update_point_position(idx_polygon, idx_point) {
	let svg_point= document.getElementById(get_svg_point_id(idx_polygon, idx_point));
	svg_point.setAttribute("cx", polygons[idx_polygon][idx_point].x);
	svg_point.setAttribute("cy", polygons[idx_polygon][idx_point].y);

	let svg_line_before;
	if (idx_point> 0) {
		svg_line_before= document.getElementById(get_svg_line_id(idx_polygon, idx_point- 1));
	}
	else {
		svg_line_before= document.getElementById(get_svg_line_id(idx_polygon, polygons[idx_polygon].length- 1));
	}
	svg_line_before.setAttribute("x2", polygons[idx_polygon][idx_point].x);
	svg_line_before.setAttribute("y2", polygons[idx_polygon][idx_point].y);

	let svg_line_after= document.getElementById(get_svg_line_id(idx_polygon, idx_point));
	svg_line_after.setAttribute("x1", polygons[idx_polygon][idx_point].x);
	svg_line_after.setAttribute("y1", polygons[idx_polygon][idx_point].y);
}


function update_polygon_position(idx_polygon) {
	for (let i=0; i<polygons[idx_polygon].length; ++i) {
		update_point_position(idx_polygon, i);
	}
}


export function svg_init(svg_main_id, polygons_) {
	polygons= polygons_;
	
	svg_main= document.getElementById(svg_main_id);
	svg_main.setAttribute("viewBox", "0.0 0.0 1.0 1.0");
	svg_width= svg_main.getAttribute("width");
	svg_height= svg_main.getAttribute("height");

	svg_emprise= document.createElementNS("http://www.w3.org/2000/svg", "rect");
	svg_emprise.classList.add("svg_emprise");
	svg_emprise.setAttribute("width", "100%");
	svg_emprise.setAttribute("height", "100%");
	svg_main.appendChild(svg_emprise);
	
	svg_main_group= document.createElementNS("http://www.w3.org/2000/svg", "g");
	svg_main_group.classList.add("svg_main_group");
	// transform sert à avoir une origine en bas à gauche
	svg_main_group.setAttribute("transform", "matrix(1.0 0.0 0.0 -1.0 0.0 1.0)");
	svg_main.appendChild(svg_main_group);
	
	svg_emprise.addEventListener("mousedown", mouse_down_emprise);
	svg_emprise.addEventListener("mouseup", mouse_up_emprise);
	svg_emprise.addEventListener("mousemove", mouse_move);
	svg_emprise.addEventListener("mouseout", mouse_out_emprise);

	document.addEventListener("keydown", function(event) {
		keypresseds[event.key]= true;
	});
	document.addEventListener("keyup", function(event) {
		keypresseds[event.key]= false;
	});

	let text_base_x= 0.7;
	let text_base2_x= 0.72;
	let text_base_y= 0.03;
	let text_step_y= 0.03;
	let compt= 0;
	for (let key in INSTRUCTIONS) {
		let t= document.createElementNS("http://www.w3.org/2000/svg", "text");
		t.setAttribute("x", text_base_x);
		t.setAttribute("y", text_base_y+ compt* text_step_y);
		compt+= 1;
		t.innerHTML= key;
		svg_main.appendChild(t);

		for (let key2 in INSTRUCTIONS[key]) {
			let t= document.createElementNS("http://www.w3.org/2000/svg", "text");
			t.setAttribute("x", text_base2_x);
			t.setAttribute("y", text_base_y+ compt* text_step_y);
			compt+= 1;
			t.innerHTML= key2+ " : "+ INSTRUCTIONS[key][key2];
			svg_main.appendChild(t);
		}
	}
}


function mouse_down_emprise(e) {
	if (DEBUG) { console.log("mouse_down_emprise") };
	
	let current_position= normalized_coords(e.offsetX, e.offsetY);

	if (keypresseds['x']) {
		delete_all_polygon_groups();
		clear();
	}
	else if (keypresseds['p']) {
		editing_mode= "CREATE_POLYGON";
		let polygon_idx= add_polygon();
		editing_data= {"create_polygon_polygon_idx" : polygon_idx};
		add_point(polygon_idx, current_position);
		update_polygon_group(polygon_idx);
	}
	else if (keypresseds['c']) {
		editing_mode= "CREATE_CIRCLE";
		let polygon_idx= add_polygon();
		for (let i=0; i<N_POINTS_PER_CIRCLE; ++i) {
			add_point(polygon_idx, current_position);
		}
		editing_data= {"create_circle_polygon_idx" : polygon_idx, "create_circle_start" : current_position};
		update_polygon_group(polygon_idx);
	}
	else if (keypresseds['r']) {
		editing_mode= "CREATE_RECTANGLE";
		let polygon_idx= add_polygon();
		for (let i=0; i<4; ++i) {
			add_point(polygon_idx, current_position);
		}
		editing_data= {"create_rectangle_polygon_idx" : polygon_idx, "create_rectangle_start" : current_position};
		update_polygon_group(polygon_idx);
	}
}


function mouse_up_emprise(e) {
	if (DEBUG) { console.log("mouse_up_emprise") };
	
	let current_position= normalized_coords(e.offsetX, e.offsetY);
	
	/*if (editing_mode== "CREATE_POLYGON") {
		add_point(editing_data["create_polygon_polygon_idx"], current_position);
		simplify_polygon(editing_data["create_polygon_polygon_idx"], 0.01, 0.01);
		update_polygon_group(editing_data["create_polygon_polygon_idx"]);
	}*/
	
	set_editing_null();
}


function mouse_move(e) {
	if (DEBUG) { console.log("mouse_move") };
	
	let current_position= normalized_coords(e.offsetX, e.offsetY);

	if (editing_mode== "CREATE_POLYGON") {
		add_point(editing_data["create_polygon_polygon_idx"], current_position);
		add_last_point_to_polygon_group(editing_data["create_polygon_polygon_idx"]);
	}
	else if (editing_mode== "MOVE_POINT") {
		move_point(editing_data["move_point_polygon_idx"], editing_data["move_point_point_idx"], current_position);
		update_point_position(editing_data["move_point_polygon_idx"], editing_data["move_point_point_idx"]);
	}
	else if (editing_mode== "MOVE_POLYGON") {
		for (let i=0; i<polygons[editing_data["move_polygon_polygon_idx"]].length; ++i) {
			move_point(editing_data["move_polygon_polygon_idx"], i, {
				"x" : editing_data["move_polygon_init"][i].x+ current_position.x- editing_data["move_polygon_start"].x,
				"y" : editing_data["move_polygon_init"][i].y+ current_position.y- editing_data["move_polygon_start"].y
			});
		}
		update_polygon_position(editing_data["move_polygon_polygon_idx"]);
	}
	else if (editing_mode== "SCALE_POLYGON") {
		let dist= scalar_product(diff(current_position, editing_data["scale_polygon_start"]), editing_data["scale_polygon_ortho_v"]);
		for (let i=0; i<polygons[editing_data["scale_polygon_polygon_idx"]].length; ++i) {
			let u= normalized(diff(editing_data["scale_polygon_init"][i], editing_data["scale_polygon_inertia_center"]));
			move_point(editing_data["scale_polygon_polygon_idx"], i, {
				"x" : editing_data["scale_polygon_init"][i].x+ dist* u.x,
				"y" : editing_data["scale_polygon_init"][i].y+ dist* u.y
			});
			update_polygon_position(editing_data["scale_polygon_polygon_idx"]);
		}
	}
	else if (editing_mode== "ROTATE_POLYGON") {
		let a= angle(editing_data["rotate_polygon_inertia_center"], editing_data["rotate_polygon_start"], current_position);
		for (let i=0; i<polygons[editing_data["rotate_polygon_polygon_idx"]].length; ++i) {
			move_point(editing_data["rotate_polygon_polygon_idx"], i, rotate(editing_data["rotate_polygon_init"][i], editing_data["rotate_polygon_inertia_center"], a));
		}
		update_polygon_position(editing_data["rotate_polygon_polygon_idx"]);
	}
	else if (editing_mode== "MOVE_LINE") {
		move_point(editing_data["move_line_polygon_idx"], editing_data["move_line_p0_idx"], {
			"x" : editing_data["move_line_p0"].x+ current_position.x- editing_data["move_line_start"].x,
			"y" : editing_data["move_line_p0"].y+ current_position.y- editing_data["move_line_start"].y
		});
		move_point(editing_data["move_line_polygon_idx"], editing_data["move_line_p1_idx"], {
			"x" : editing_data["move_line_p1"].x+ current_position.x- editing_data["move_line_start"].x,
			"y" : editing_data["move_line_p1"].y+ current_position.y- editing_data["move_line_start"].y
		});
		update_point_position(editing_data["move_line_polygon_idx"], editing_data["move_line_p0_idx"]);
		update_point_position(editing_data["move_line_polygon_idx"], editing_data["move_line_p1_idx"]);
	}
	else if (editing_mode== "CREATE_CIRCLE") {
		let ray= norm(diff(editing_data["create_circle_start"], current_position));
		for (var i=0; i<polygons[editing_data["create_circle_polygon_idx"]].length; ++i) {
			move_point(editing_data["create_circle_polygon_idx"], i, {
				"x" : editing_data["create_circle_start"].x+ ray* Math.cos(2.0* Math.PI* i/ polygons[editing_data["create_circle_polygon_idx"]].length),
				"y" : editing_data["create_circle_start"].y+ ray* Math.sin(2.0* Math.PI* i/ polygons[editing_data["create_circle_polygon_idx"]].length)
			});
		}
		update_polygon_position(editing_data["create_circle_polygon_idx"]);
	}
	else if (editing_mode== "CREATE_RECTANGLE") {
		move_point(editing_data["create_rectangle_polygon_idx"], 1, {
			"x" : current_position.x, "y" : editing_data["create_rectangle_start"].y
		});
		move_point(editing_data["create_rectangle_polygon_idx"], 2, {
			"x" : current_position.x, "y" : current_position.y
		});
		move_point(editing_data["create_rectangle_polygon_idx"], 3, {
			"x" : editing_data["create_rectangle_start"].x, "y" : current_position.y
		});
		update_polygon_position(editing_data["create_rectangle_polygon_idx"]);
	}
}


function mouse_out_emprise(e) {
	if (DEBUG) { console.log("mouse_out_emprise") };
	
	if ((e.offsetX< 2) || (e.offsetX> svg_width- 3) || (e.offsetY< 2) || (e.offsetY> svg_height- 3)) {
		set_editing_null();
	}
}


function mouse_down_point(e) {
	if (DEBUG) { console.log("mouse_down_point") };
	
	let current_position= normalized_coords(e.offsetX, e.offsetY);
	let [polygon_idx, point_idx]= get_point_idx(e.target.id);

	if (keypresseds['x']) {
		remove_point(polygon_idx, point_idx);
		update_polygon_group(polygon_idx);
		set_editing_null();
	}
	else if (keypresseds['c']) {
		duplicate_polygon(polygon_idx);
		update_polygon_group(polygons.length- 1);
		set_editing_null();
	}
	else if (keypresseds['p']) {
		editing_mode= "MOVE_POLYGON";
		editing_data= {
			"move_polygon_polygon_idx" : polygon_idx,
			"move_polygon_start" : current_position,
			"move_polygon_init" : deepcopy(polygons[polygon_idx])
		};
	}
	else if (keypresseds['m']) {
		editing_mode= "MOVE_POINT";
		editing_data= {
			"move_point_polygon_idx" : polygon_idx,
			"move_point_point_idx" : point_idx
		};
	}
	//if (e.shiftKey) {/*shift is down*/
	/*if (e.altKey) {}
	if (e.ctrlKey) {}
	if (e.metaKey) {}*/
}


function mouse_up_point(e) {
	if (DEBUG) { console.log("mouse_up_point") };
	
	let current_position= normalized_coords(e.offsetX, e.offsetY);

	if (editing_mode== "CREATE_POLYGON") {
		add_point(editing_data["create_polygon_polygon_idx"], current_position);
		simplify_polygon(editing_data["create_polygon_polygon_idx"], 0.01, 0.01);
		update_polygon_group(editing_data["create_polygon_polygon_idx"]);
	}

	set_editing_null();
}


function mouse_down_line(e) {
	if (DEBUG) { console.log("mouse_down_line") };
	
	let current_position= normalized_coords(e.offsetX, e.offsetY);
	let [polygon_idx, line_idx]= get_line_idx(e.target.id);

	if (keypresseds['a']) {
		add_point_on_line(polygon_idx, line_idx, current_position);
		update_polygon_group(polygon_idx);
		set_editing_null();
	}
	else if (keypresseds['c']) {
		duplicate_polygon(polygon_idx);
		update_polygon_group(polygons.length- 1);
		set_editing_null();
	}
	else if (keypresseds['m']) {
		editing_mode= "MOVE_LINE";
		editing_data= {
			"move_line_polygon_idx" : polygon_idx,
			"move_line_p0_idx" : line_idx,
			"move_line_p1_idx" : (line_idx+ 1) % polygons[polygon_idx].length,
			"move_line_start" : current_position,
			"move_line_p0" : deepcopy(polygons[polygon_idx][line_idx]),
			"move_line_p1" : deepcopy(polygons[polygon_idx][(line_idx+ 1) % polygons[polygon_idx].length])
		};
	}
	else if (keypresseds['p']) {
		editing_mode= "MOVE_POLYGON";
		editing_data= {
			"move_polygon_polygon_idx" : polygon_idx,
			"move_polygon_start" : current_position,
			"move_polygon_init" : deepcopy(polygons[polygon_idx])
		};
	}
	else if (keypresseds['r']) {
		editing_mode= "ROTATE_POLYGON";
		editing_data= {
			"rotate_polygon_polygon_idx" : polygon_idx,
			"rotate_polygon_start" : current_position,
			"rotate_polygon_init" : deepcopy(polygons[polygon_idx]),
			"rotate_polygon_inertia_center" : inertia_center(polygons[polygon_idx])
		};
	}
	else if (keypresseds['s']) {
		editing_mode= "SCALE_POLYGON";
		editing_data= {
			"scale_polygon_polygon_idx" : polygon_idx,
			"scale_polygon_start" : current_position,
			"scale_polygon_init" : deepcopy(polygons[polygon_idx]),
			"scale_polygon_inertia_center" : inertia_center(polygons[polygon_idx]),
			"scale_polygon_ortho_v" : normalized(orthogonal(diff(polygons[polygon_idx][(line_idx+ 1) % polygons[polygon_idx].length], polygons[polygon_idx][line_idx])))
		};
	}
	else if (keypresseds['x']) {
		delete_polygon_group(polygon_idx);
		remove_polygon(polygon_idx);
		set_editing_null();
	}
}


function mouse_up_line(e) {
	if (DEBUG) { console.log("mouse_up_line") };
	
	set_editing_null();
}
