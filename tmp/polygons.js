import {deepcopy} from "./utils.js";
import {norm, normalized, diff, scalar_product, orthogonal, inertia_center, angle, rotate, simplify_coords, bbox} from "./geom.js";


const N_POINTS_PER_CIRCLE= 16;
const INDEX_POLYGON_TEXT_OFFSET_X= 0.01;
const INDEX_POLYGON_TEXT_OFFSET_Y= 0.05;
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
		"x" : "delete polygon"
	}
};


export class PolygonsContext {
	constructor(id, keypresseds) {
		this.id= id;
		
		this.polygons= [];
		this.keypresseds= keypresseds;
		this.editing_mode= null;
		this.editing_data= null;
		
		this.svg_main= document.getElementById(this.id);
		this.svg_main.setAttribute("viewBox", "0.0 0.0 1.0 1.0");
		this.svg_width= this.svg_main.getAttribute("width");
		this.svg_height= this.svg_main.getAttribute("height");
	
		this.svg_emprise= document.createElementNS("http://www.w3.org/2000/svg", "rect");
		this.svg_emprise.classList.add("svg_emprise");
		this.svg_emprise.setAttribute("width", "100%");
		this.svg_emprise.setAttribute("height", "100%");
		this.svg_main.appendChild(this.svg_emprise);
		
		this.svg_main_group= document.createElementNS("http://www.w3.org/2000/svg", "g");
		this.svg_main_group.classList.add("svg_main_group");
		// transform sert à avoir une origine en bas à gauche
		this.svg_main_group.setAttribute("transform", "matrix(1.0 0.0 0.0 -1.0 0.0 1.0)");
		this.svg_main.appendChild(this.svg_main_group);
		
		this.svg_emprise.addEventListener("mousedown", this.mouse_down_emprise.bind(this));
		this.svg_emprise.addEventListener("mouseup", this.mouse_up_emprise.bind(this));
		this.svg_emprise.addEventListener("mousemove", this.mouse_move.bind(this));
		this.svg_emprise.addEventListener("mouseout", this.mouse_out_emprise.bind(this));

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
			this.svg_main.appendChild(t);
	
			for (let key2 in INSTRUCTIONS[key]) {
				let t= document.createElementNS("http://www.w3.org/2000/svg", "text");
				t.setAttribute("x", text_base2_x);
				t.setAttribute("y", text_base_y+ compt* text_step_y);
				compt+= 1;
				t.innerHTML= key2+ " : "+ INSTRUCTIONS[key][key2];
				this.svg_main.appendChild(t);
			}
		}
	}

	
	send_event(detail) {
		let merged_detail= Object.assign({}, {"svg_id" : this.id}, detail);
		document.dispatchEvent(new CustomEvent("svg_polygons_changed", {detail : merged_detail}));
	}


	get_point_idx(svg_id) {
		return [
			parseInt(svg_id.split("polygon_")[1].split("_")[0]),
			parseInt(svg_id.split("point_")[1])
		];
	}
	
	
	get_line_idx(svg_id) {
		return [
			parseInt(svg_id.split("polygon_")[1].split("_")[0]),
			parseInt(svg_id.split("line_")[1])
		];
	}
	
	
	get_svg_polygon_id(idx_polygon) {
		return this.id+ "_polygon_"+ idx_polygon;
	}
	
	
	get_svg_point_id(idx_polygon, idx_point) {
		return this.id+ "_polygon_"+ idx_polygon+ "_point_"+ idx_point;
	}
	
	
	get_svg_line_id(idx_polygon, idx_line) {
		return this.id+ "_polygon_"+ idx_polygon+ "_line_"+ idx_line;
	}


	normalized_coords(x, y) {
		return {"x" : x/ this.svg_width, "y" : 1.0- y/ this.svg_height};
	}


	clear() {
		this.polygons= [];
		this.send_event({"type" : "clear"});
	}
	

	add_polygon() {
		this.polygons.push([]);
		this.send_event({"type" : "add_polygon"});
		return this.polygons.length- 1;
	}


	remove_polygon(idx_polygon) {
		this.polygons.splice(idx_polygon, 1);
		this.send_event({"type" : "remove_polygon", "idx_polygon" : idx_polygon});
	}
	

	duplicate_polygon(idx_polygon) {
		this.polygons.push(deepcopy(this.polygons[idx_polygon]));
		this.send_event({"type" : "duplicate_polygon", "idx_polygon" : idx_polygon});
		this.move_polygon(this.polygons.length- 1, {"x" : 0.1, "y" : 0.1});
	}

	
	set_polygon(idx_polygon, polygon) {
		this.polygons[idx_polygon]= [];
		for (let i=0; i<polygon.length; ++i) {
			if ("x" in polygon[i]) {
				this.polygons[idx_polygon].push({"x" : polygon[i].x, "y" : polygon[i].y});
			}
			else {
				this.polygons[idx_polygon].push({"x" : polygon[i][0], "y" : polygon[i][1]});
			}
		}
	}
	
	
	move_polygon(idx_polygon, v) {
		for (let i=0; i<this.polygons[idx_polygon].length; ++i) {
			this.polygons[idx_polygon][i].x+= v.x;
			this.polygons[idx_polygon][i].y+= v.y;
		}
		this.send_event({"type" : "move_polygon", "idx_polygon" : idx_polygon});
	}
		
	
	simplify_polygon(idx_polygon, treshold, min_dist) {
		let simplified= simplify_coords(this.polygons[idx_polygon], treshold, min_dist);
		this.polygons[idx_polygon]= simplified;
		this.send_event({"type" : "simplify_polygon", "idx_polygon" : idx_polygon});
	}
	

	add_point(idx_polygon, coord) {
		this.polygons[idx_polygon].push(coord);
		this.send_event({"type" : "add_point", "idx_polygon" : idx_polygon});
		return this.polygons[idx_polygon].length- 1;
	}
			

	add_point_on_line(idx_polygon, idx_line, coord) {
		this.polygons[idx_polygon].splice(idx_line+ 1, 0, coord);
		this.send_event({"type" : "add_point_on_line", "idx_polygon" : idx_polygon});
	}
	
	
	remove_point(idx_polygon, idx_point) {
		this.polygons[idx_polygon].splice(idx_point, 1);
		this.send_event({"type" : "remove_point", "idx_polygon" : idx_polygon});
	}
	
	
	move_point(idx_polygon, idx_point, coord) {
		this.polygons[idx_polygon][idx_point]= coord;
		this.send_event({"type" : "move_point", "idx_polygon" : idx_polygon});
	}
	

	set_editing_null() {
		this.editing_mode= null;
		this.editing_data= null;
	}


	get_index_polygon_text_position(idx_polygon) {
		let polygon_bbox= bbox(this.polygons[idx_polygon]);
		return {"x" : polygon_bbox.xmin- INDEX_POLYGON_TEXT_OFFSET_X, "y" : 1.0- polygon_bbox.ymin+ INDEX_POLYGON_TEXT_OFFSET_Y};
	}


	update_polygon_group(idx_polygon) {
		let polygon_group= document.getElementById(this.get_svg_polygon_id(idx_polygon));
		if (polygon_group=== null) {
			polygon_group= document.createElementNS("http://www.w3.org/2000/svg", "g");
			polygon_group.setAttribute("id", this.get_svg_polygon_id(idx_polygon));
			this.svg_main_group.appendChild(polygon_group);
		}
		else {
			polygon_group.innerHTML= "";
		}
		
		for (let i=0; i<this.polygons[idx_polygon].length; ++i) {
			polygon_group.innerHTML+= '<line x1="'+ this.polygons[idx_polygon][i].x+ '" y1="'+ this.polygons[idx_polygon][i].y+ 
				'" x2="'+ this.polygons[idx_polygon][(i+ 1) % this.polygons[idx_polygon].length].x+
				'" y2="'+ this.polygons[idx_polygon][(i+ 1) % this.polygons[idx_polygon].length].y+
				'" class="lines" id="'+ this.get_svg_line_id(idx_polygon, i)+ '" />\n';
		}
		for (let i=0; i<this.polygons[idx_polygon].length; ++i) {
			polygon_group.innerHTML+= '<circle cx="'+ this.polygons[idx_polygon][i].x+ '" cy="'+ this.polygons[idx_polygon][i].y+ 
				'" class="points" id="'+ this.get_svg_point_id(idx_polygon, i)+ '" />\n';
		}
		let index_polygon_text_position= this.get_index_polygon_text_position(idx_polygon);
		polygon_group.innerHTML+= '<text class="index_polygon_text" x="'+ index_polygon_text_position.x+ '" y="'+ index_polygon_text_position.y+ '">'+ idx_polygon+ '</text>\n';
		
		let svg_lines= polygon_group.getElementsByClassName("lines");
		for (let i=0; i<svg_lines.length; ++i) {
			svg_lines[i].addEventListener("mousedown", this.mouse_down_line.bind(this));
			svg_lines[i].addEventListener("mouseup", this.mouse_up_line.bind(this));
			svg_lines[i].addEventListener("mousemove", this.mouse_move.bind(this));
		}
		let svg_points= polygon_group.getElementsByClassName("points");
		for (let i=0; i<svg_points.length; ++i) {
			svg_points[i].addEventListener("mousedown", this.mouse_down_point.bind(this));
			svg_points[i].addEventListener("mouseup", this.mouse_up_point.bind(this));
			svg_points[i].addEventListener("mousemove", this.mouse_move.bind(this));
		}
	}

	
	add_last_point_to_polygon_group(idx_polygon) {
		let polygon_group= document.getElementById(this.get_svg_polygon_id(idx_polygon));
		let i= this.polygons[idx_polygon].length- 1;
		
		let line= document.createElementNS("http://www.w3.org/2000/svg", "line");
		polygon_group.insertBefore(line, polygon_group.getElementsByTagName("circle")[0]);
		line.setAttribute("x1", this.polygons[idx_polygon][i].x);
		line.setAttribute("y1", this.polygons[idx_polygon][i].y);
		line.setAttribute("x2", this.polygons[idx_polygon][i- 1].x);
		line.setAttribute("y2", this.polygons[idx_polygon][i- 1].y);
		line.setAttribute("id", this.get_svg_line_id(idx_polygon, i));
		line.classList.add("lines");
		line.addEventListener("mousedown", this.mouse_down_line.bind(this));
		line.addEventListener("mouseup", this.mouse_up_line.bind(this));
		line.addEventListener("mousemove", this.mouse_move.bind(this));
	
		let point= document.createElementNS("http://www.w3.org/2000/svg", "circle");
		polygon_group.appendChild(point);
		point.setAttribute("cx", this.polygons[idx_polygon][i].x);
		point.setAttribute("cy", this.polygons[idx_polygon][i].y);
		point.setAttribute("id", this.get_svg_point_id(idx_polygon, i));
		point.classList.add("points");
		point.addEventListener("mousedown", this.mouse_down_point.bind(this));
		point.addEventListener("mouseup", this.mouse_up_point.bind(this));
		point.addEventListener("mousemove", this.mouse_move.bind(this));

		let index_polygon_text_position= this.get_index_polygon_text_position(idx_polygon);
		polygon_group.getElementsByTagName("text")[0].setAttribute("x", index_polygon_text_position.x);
		polygon_group.getElementsByTagName("text")[0].setAttribute("y", index_polygon_text_position.y);
	}


	delete_polygon_group(idx_polygon) {
		let polygon2delete= document.getElementById(this.get_svg_polygon_id(idx_polygon));
		if (polygon2delete!== null) {
			polygon2delete.remove();
		}
		for (let i=idx_polygon+ 1; i<this.polygons.length ;++i) {
			let polygon= document.getElementById(this.get_svg_polygon_id(i));
			polygon.setAttribute("id", this.get_svg_polygon_id(i- 1));
			for (let j=0; j<this.polygons[i].length; ++j) {
				let point= document.getElementById(this.get_svg_point_id(i, j));
				point.setAttribute("id", this.get_svg_point_id(i- 1, j));
			}
			for (let j=0; j<this.polygons[i].length; ++j) {
				let line= document.getElementById(this.get_svg_line_id(i, j));
				line.setAttribute("id", this.get_svg_line_id(i- 1, j));
			}
		}
	}


	delete_all_polygon_groups() {
		this.svg_main_group.innerHTML= "";
	}
		
	
	update_point_position(idx_polygon, idx_point) {
		let svg_point= document.getElementById(this.get_svg_point_id(idx_polygon, idx_point));
		svg_point.setAttribute("cx", this.polygons[idx_polygon][idx_point].x);
		svg_point.setAttribute("cy", this.polygons[idx_polygon][idx_point].y);
	
		let svg_line_before;
		if (idx_point> 0) {
			svg_line_before= document.getElementById(this.get_svg_line_id(idx_polygon, idx_point- 1));
		}
		else {
			svg_line_before= document.getElementById(this.get_svg_line_id(idx_polygon, this.polygons[idx_polygon].length- 1));
		}
		svg_line_before.setAttribute("x2", this.polygons[idx_polygon][idx_point].x);
		svg_line_before.setAttribute("y2", this.polygons[idx_polygon][idx_point].y);
	
		let svg_line_after= document.getElementById(this.get_svg_line_id(idx_polygon, idx_point));
		svg_line_after.setAttribute("x1", this.polygons[idx_polygon][idx_point].x);
		svg_line_after.setAttribute("y1", this.polygons[idx_polygon][idx_point].y);

		let index_polygon_text_position= this.get_index_polygon_text_position(idx_polygon);
		let polygon_group= document.getElementById(this.get_svg_polygon_id(idx_polygon));
		polygon_group.getElementsByTagName("text")[0].setAttribute("x", index_polygon_text_position.x);
		polygon_group.getElementsByTagName("text")[0].setAttribute("y", index_polygon_text_position.y);
	}


	update_polygon_position(idx_polygon) {
		for (let i=0; i<this.polygons[idx_polygon].length; ++i) {
			this.update_point_position(idx_polygon, i);
		}
	}


	mouse_down_emprise(e) {
		if (DEBUG) { console.log("mouse_down_emprise") };
		
		let current_position= this.normalized_coords(e.offsetX, e.offsetY);
	
		if (this.keypresseds['x']) {
			this.delete_all_polygon_groups();
			this.clear();
		}
		else if (this.keypresseds['p']) {
			this.editing_mode= "CREATE_POLYGON";
			let polygon_idx= this.add_polygon();
			this.editing_data= {"create_polygon_polygon_idx" : polygon_idx};
			this.add_point(polygon_idx, current_position);
			this.update_polygon_group(polygon_idx);
		}
		else if (this.keypresseds['c']) {
			this.editing_mode= "CREATE_CIRCLE";
			let polygon_idx= this.add_polygon();
			for (let i=0; i<N_POINTS_PER_CIRCLE; ++i) {
				this.add_point(polygon_idx, current_position);
			}
			this.editing_data= {"create_circle_polygon_idx" : polygon_idx, "create_circle_start" : current_position};
			this.update_polygon_group(polygon_idx);
		}
		else if (this.keypresseds['r']) {
			this.editing_mode= "CREATE_RECTANGLE";
			let polygon_idx= this.add_polygon();
			for (let i=0; i<4; ++i) {
				this.add_point(polygon_idx, current_position);
			}
			this.editing_data= {"create_rectangle_polygon_idx" : polygon_idx, "create_rectangle_start" : current_position};
			this.update_polygon_group(polygon_idx);
		}
	}

	
	mouse_up_emprise(e) {
		if (DEBUG) { console.log("mouse_up_emprise") };
		
		//let current_position= this.normalized_coords(e.offsetX, e.offsetY);
		this.set_editing_null();
	}
	

	mouse_move(e) {
		if (DEBUG) { console.log("mouse_move") };
		
		let current_position= this.normalized_coords(e.offsetX, e.offsetY);
	
		if (this.editing_mode== "CREATE_POLYGON") {
			this.add_point(this.editing_data["create_polygon_polygon_idx"], current_position);
			this.add_last_point_to_polygon_group(this.editing_data["create_polygon_polygon_idx"]);
		}
		else if (this.editing_mode== "MOVE_POINT") {
			this.move_point(this.editing_data["move_point_polygon_idx"], this.editing_data["move_point_point_idx"], current_position);
			this.update_point_position(this.editing_data["move_point_polygon_idx"], this.editing_data["move_point_point_idx"]);
		}
		else if (this.editing_mode== "MOVE_POLYGON") {
			for (let i=0; i<this.polygons[this.editing_data["move_polygon_polygon_idx"]].length; ++i) {
				this.move_point(this.editing_data["move_polygon_polygon_idx"], i, {
					"x" : this.editing_data["move_polygon_init"][i].x+ current_position.x- this.editing_data["move_polygon_start"].x,
					"y" : this.editing_data["move_polygon_init"][i].y+ current_position.y- this.editing_data["move_polygon_start"].y
				});
			}
			this.update_polygon_position(this.editing_data["move_polygon_polygon_idx"]);
		}
		else if (this.editing_mode== "SCALE_POLYGON") {
			let dist= scalar_product(diff(current_position, this.editing_data["scale_polygon_start"]), this.editing_data["scale_polygon_ortho_v"]);
			for (let i=0; i<this.polygons[this.editing_data["scale_polygon_polygon_idx"]].length; ++i) {
				let u= normalized(diff(this.editing_data["scale_polygon_init"][i], this.editing_data["scale_polygon_inertia_center"]));
				this.move_point(this.editing_data["scale_polygon_polygon_idx"], i, {
					"x" : this.editing_data["scale_polygon_init"][i].x+ dist* u.x,
					"y" : this.editing_data["scale_polygon_init"][i].y+ dist* u.y
				});
				this.update_polygon_position(this.editing_data["scale_polygon_polygon_idx"]);
			}
		}
		else if (this.editing_mode== "ROTATE_POLYGON") {
			let a= angle(this.editing_data["rotate_polygon_inertia_center"], this.editing_data["rotate_polygon_start"], current_position);
			for (let i=0; i<this.polygons[this.editing_data["rotate_polygon_polygon_idx"]].length; ++i) {
				this.move_point(this.editing_data["rotate_polygon_polygon_idx"], i, rotate(this.editing_data["rotate_polygon_init"][i],
					this.editing_data["rotate_polygon_inertia_center"], a));
			}
			this.update_polygon_position(this.editing_data["rotate_polygon_polygon_idx"]);
		}
		else if (this.editing_mode== "MOVE_LINE") {
			this.move_point(this.editing_data["move_line_polygon_idx"], this.editing_data["move_line_p0_idx"], {
				"x" : this.editing_data["move_line_p0"].x+ current_position.x- this.editing_data["move_line_start"].x,
				"y" : this.editing_data["move_line_p0"].y+ current_position.y- this.editing_data["move_line_start"].y
			});
			this.move_point(this.editing_data["move_line_polygon_idx"], this.editing_data["move_line_p1_idx"], {
				"x" : this.editing_data["move_line_p1"].x+ current_position.x- this.editing_data["move_line_start"].x,
				"y" : this.editing_data["move_line_p1"].y+ current_position.y- this.editing_data["move_line_start"].y
			});
			this.update_point_position(this.editing_data["move_line_polygon_idx"], this.editing_data["move_line_p0_idx"]);
			this.update_point_position(this.editing_data["move_line_polygon_idx"], this.editing_data["move_line_p1_idx"]);
		}
		else if (this.editing_mode== "CREATE_CIRCLE") {
			let ray= norm(diff(this.editing_data["create_circle_start"], current_position));
			for (var i=0; i<this.polygons[this.editing_data["create_circle_polygon_idx"]].length; ++i) {
				this.move_point(this.editing_data["create_circle_polygon_idx"], i, {
					"x" : this.editing_data["create_circle_start"].x+ ray* Math.cos(2.0* Math.PI* i/ this.polygons[this.editing_data["create_circle_polygon_idx"]].length),
					"y" : this.editing_data["create_circle_start"].y+ ray* Math.sin(2.0* Math.PI* i/ this.polygons[this.editing_data["create_circle_polygon_idx"]].length)
				});
			}
			this.update_polygon_position(this.editing_data["create_circle_polygon_idx"]);
		}
		else if (this.editing_mode== "CREATE_RECTANGLE") {
			this.move_point(this.editing_data["create_rectangle_polygon_idx"], 1, {
				"x" : current_position.x, "y" : this.editing_data["create_rectangle_start"].y
			});
			this.move_point(this.editing_data["create_rectangle_polygon_idx"], 2, {
				"x" : current_position.x, "y" : current_position.y
			});
			this.move_point(this.editing_data["create_rectangle_polygon_idx"], 3, {
				"x" : this.editing_data["create_rectangle_start"].x, "y" : current_position.y
			});
			this.update_polygon_position(this.editing_data["create_rectangle_polygon_idx"]);
		}
	}


	mouse_out_emprise(e) {
		if (DEBUG) { console.log("mouse_out_emprise") };
		
		if ((e.offsetX< 2) || (e.offsetX> this.svg_width- 3) || (e.offsetY< 2) || (e.offsetY> this.svg_height- 3)) {
			this.set_editing_null();
		}
	}

	
	mouse_down_point(e) {
		if (DEBUG) { console.log("mouse_down_point") };
		
		let current_position= this.normalized_coords(e.offsetX, e.offsetY);
		let [polygon_idx, point_idx]= this.get_point_idx(e.target.id);
	
		if (this.keypresseds['x']) {
			this.remove_point(polygon_idx, point_idx);
			this.update_polygon_group(polygon_idx);
			this.set_editing_null();
		}
		else if (this.keypresseds['c']) {
			this.duplicate_polygon(polygon_idx);
			this.update_polygon_group(this.polygons.length- 1);
			this.set_editing_null();
		}
		else if (this.keypresseds['p']) {
			this.editing_mode= "MOVE_POLYGON";
			this.editing_data= {
				"move_polygon_polygon_idx" : polygon_idx,
				"move_polygon_start" : current_position,
				"move_polygon_init" : deepcopy(this.polygons[polygon_idx])
			};
		}
		else if (this.keypresseds['m']) {
			this.editing_mode= "MOVE_POINT";
			this.editing_data= {
				"move_point_polygon_idx" : polygon_idx,
				"move_point_point_idx" : point_idx
			};
		}
	}
	
	
	mouse_up_point(e) {
		if (DEBUG) { console.log("mouse_up_point") };
		
		let current_position= this.normalized_coords(e.offsetX, e.offsetY);
	
		if (this.editing_mode== "CREATE_POLYGON") {
			this.add_point(this.editing_data["create_polygon_polygon_idx"], current_position);
			this.simplify_polygon(this.editing_data["create_polygon_polygon_idx"], 0.01, 0.01);
			this.update_polygon_group(this.editing_data["create_polygon_polygon_idx"]);
		}
	
		this.set_editing_null();
	}
	
	
	mouse_down_line(e) {
		if (DEBUG) { console.log("mouse_down_line") };
		
		let current_position= this.normalized_coords(e.offsetX, e.offsetY);
		let [polygon_idx, line_idx]= this.get_line_idx(e.target.id);
	
		if (this.keypresseds['a']) {
			this.add_point_on_line(polygon_idx, line_idx, current_position);
			this.update_polygon_group(polygon_idx);
			this.set_editing_null();
		}
		else if (this.keypresseds['c']) {
			this.duplicate_polygon(polygon_idx);
			this.update_polygon_group(this.polygons.length- 1);
			this.set_editing_null();
		}
		else if (this.keypresseds['m']) {
			this.editing_mode= "MOVE_LINE";
			this.editing_data= {
				"move_line_polygon_idx" : polygon_idx,
				"move_line_p0_idx" : line_idx,
				"move_line_p1_idx" : (line_idx+ 1) % this.polygons[polygon_idx].length,
				"move_line_start" : current_position,
				"move_line_p0" : deepcopy(this.polygons[polygon_idx][line_idx]),
				"move_line_p1" : deepcopy(this.polygons[polygon_idx][(line_idx+ 1) % this.polygons[polygon_idx].length])
			};
		}
		else if (this.keypresseds['p']) {
			this.editing_mode= "MOVE_POLYGON";
			this.editing_data= {
				"move_polygon_polygon_idx" : polygon_idx,
				"move_polygon_start" : current_position,
				"move_polygon_init" : deepcopy(this.polygons[polygon_idx])
			};
		}
		else if (this.keypresseds['r']) {
			this.editing_mode= "ROTATE_POLYGON";
			this.editing_data= {
				"rotate_polygon_polygon_idx" : polygon_idx,
				"rotate_polygon_start" : current_position,
				"rotate_polygon_init" : deepcopy(this.polygons[polygon_idx]),
				"rotate_polygon_inertia_center" : inertia_center(this.polygons[polygon_idx])
			};
		}
		else if (this.keypresseds['s']) {
			this.editing_mode= "SCALE_POLYGON";
			this.editing_data= {
				"scale_polygon_polygon_idx" : polygon_idx,
				"scale_polygon_start" : current_position,
				"scale_polygon_init" : deepcopy(this.polygons[polygon_idx]),
				"scale_polygon_inertia_center" : inertia_center(this.polygons[polygon_idx]),
				"scale_polygon_ortho_v" : normalized(orthogonal(diff(this.polygons[polygon_idx][(line_idx+ 1) % this.polygons[polygon_idx].length], this.polygons[polygon_idx][line_idx])))
			};
		}
		else if (this.keypresseds['x']) {
			this.delete_polygon_group(polygon_idx);
			this.remove_polygon(polygon_idx);
			this.set_editing_null();
		}
	}
	
	
	mouse_up_line(e) {
		if (DEBUG) { console.log("mouse_up_line") };
		
		this.set_editing_null();
	}
}

