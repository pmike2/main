import {deepcopy} from "./utils.js";
import {segment_intersects_segment, functionalize_coords, simplify_coords} from "./geom.js";

const COORD_MIN= {"x" : 0.0, "y" : 0.0};
const COORD_MAX= {"x" : 1.0, "y" : 1.0};
const DEBUG= false;
const MARGIN_X= 0.1;
const MARGIN_Y= 0.1;
const INSTRUCTIONS= {
	"fond" : {
		"t" : "create timeline",
		"x" : "reinit"
	},
	"point" : {
		"m" : "move point",
		"t" : "move timeline",
		"x" : "delete point"
	},
	"line" : {
		"a" : "add point",
		"m" : "move line",
		"t" : "move timeline"
	}
};



export class TimeLineContext {
	constructor(id, keypresseds) {
		this.id= id;
		
		this.checkpoints= [];
		this.keypresseds= keypresseds;
		this.editing_mode= null;
		this.editing_data= null;
		
		this.svg_main= document.getElementById(this.id);
		//this.svg_main.setAttribute("viewBox", "0.0 0.0 1.0 1.0");
		//this.svg_width= this.svg_main.getAttribute("width");
		//this.svg_height= this.svg_main.getAttribute("height");
		this.svg_width= this.svg_main.getAttribute("width");
		this.svg_height= this.svg_main.getAttribute("height");
		let xmin= -MARGIN_X;
		let width= 1.0+ 2.0* MARGIN_X;
		let ymin= -MARGIN_Y;
		let height= 1.0+ 2.0* MARGIN_Y;
		this.svg_main.setAttribute("viewBox", xmin+ " "+ ymin+ " "+ width+ " "+ height);
		
		this.svg_emprise_with_margin= document.createElementNS("http://www.w3.org/2000/svg", "rect");
		this.svg_emprise_with_margin.classList.add("svg_emprise_with_margin");
		this.svg_emprise_with_margin.setAttribute("x", xmin);
		this.svg_emprise_with_margin.setAttribute("y", ymin);
		this.svg_emprise_with_margin.setAttribute("width", width);
		this.svg_emprise_with_margin.setAttribute("height", height);
		this.svg_main.appendChild(this.svg_emprise_with_margin);

		this.svg_emprise= document.createElementNS("http://www.w3.org/2000/svg", "rect");
		this.svg_emprise.classList.add("svg_emprise");
		this.svg_emprise.setAttribute("x", 0.0);
		this.svg_emprise.setAttribute("y", 0.0);
		this.svg_emprise.setAttribute("width", 1.0);
		this.svg_emprise.setAttribute("height", 1.0);
		this.svg_main.appendChild(this.svg_emprise);
		
		this.svg_main_group= document.createElementNS("http://www.w3.org/2000/svg", "g");
		this.svg_main_group.classList.add("svg_main_group");
		// transform sert à avoir une origine en bas à gauche
		this.svg_main_group.setAttribute("transform", "matrix(1.0 0.0 0.0 -1.0 0.0 1.0)");
		this.svg_main.appendChild(this.svg_main_group);
		
		this.svg_emprise_with_margin.addEventListener("mousedown", this.mouse_down.bind(this));
		this.svg_emprise_with_margin.addEventListener("mouseup", this.mouse_up.bind(this));
		this.svg_emprise_with_margin.addEventListener("mousemove", this.mouse_move.bind(this));
		this.svg_emprise_with_margin.addEventListener("mouseout", this.mouse_out.bind(this));

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


	send_event() {
		document.dispatchEvent(new CustomEvent("svg_timeline_changed", {detail : {"svg_id" : this.id}}));
	}


	get_point_idx(svg_id) {
		return parseInt(svg_id.split("point_")[1]);
	}
	
	
	get_line_idx(svg_id) {
		return parseInt(svg_id.split("line_")[1]);
	}


	get_svg_point_id(idx_point) {
		return this.id+ "_point_"+ idx_point;
	}
	
	
	get_svg_line_id(idx_line) {
		return this.id+ "_line_"+ idx_line;
	}


	normalized_coords(x, y) {
		return {"x" : (1.0+ 2* MARGIN_X)* x/ this.svg_width- MARGIN_X, "y" : 1.0- (1.0+ 2* MARGIN_Y)* y/ this.svg_height+ MARGIN_Y};
	}


	init_checkpoints() {
		this.checkpoints= [COORD_MIN, COORD_MAX];
		this.send_event();
	}


	set_checkpoints(checkpoints) {
		this.checkpoints= deepcopy(checkpoints);
		this.functionalize();
	}


	move_checkpoints(v) {
		for (let i=0; i<this.checkpoints.length; ++i) {
			this.checkpoints[i].x+= v.x;
			this.checkpoints[i].y+= v.y;
		}
		this.send_event();
	}
		
	
	simplify_checkpoints(treshold, min_dist) {
		let simplified= simplify_coords(this.checkpoints, treshold, min_dist);
		this.checkpoints= simplified;
	}


	crop_x() {
		let first_x_ok= -1;
		for (let i=0; i<this.checkpoints.length; ++i) {
			if (this.checkpoints[i].x>= 0.0) {
				first_x_ok= i;
				break;
			}
		}
		if (first_x_ok< 0) {
			this.init_checkpoints();
			return;
		}
		else if (first_x_ok== 0) {
			if (this.checkpoints[0].x!= 0.0) {
				this.checkpoints.splice(0, 0, {"x" : 0.0, "y" : this.checkpoints[0].y});
			}
		}
		else {
			let inter= segment_intersects_segment(this.checkpoints[first_x_ok- 1], this.checkpoints[first_x_ok], {"x" : 0.0, "y" : 0.0}, {"x" : 0.0, "y" : 1.0});
			if (inter=== null) {
				this.checkpoints.splice(0, first_x_ok, {"x" : 0.0, "y" : this.checkpoints[0].y});
			}
			else {
				this.checkpoints.splice(0, first_x_ok, {"x" : 0.0, "y" : inter.y});
			}
		}

		let last_x_ok= -1;
		for (let i=this.checkpoints.length- 1; i>=0; --i) {
			if (this.checkpoints[i].x<= 1.0) {
				last_x_ok= i;
				break;
			}
		}
		if (last_x_ok< 0) {
			this.init_checkpoints();
			return;
		}
		else if (last_x_ok== this.checkpoints.length- 1) {
			if (this.checkpoints[last_x_ok].x!= 1.0) {
				this.checkpoints.splice(last_x_ok+ 1, 0, {"x" : 1.0, "y" : this.checkpoints[last_x_ok].y});
			}
		}
		else {
			let inter= segment_intersects_segment(this.checkpoints[last_x_ok], this.checkpoints[last_x_ok+ 1], {"x" : 1.0, "y" : 0.0}, {"x" : 1.0, "y" : 1.0});
			if (inter=== null) {
				this.checkpoints.splice(last_x_ok+ 1, this.checkpoints.length- last_x_ok, {"x" : 1.0, "y" : this.checkpoints[last_x_ok].y});
			}
			else {
				this.checkpoints.splice(last_x_ok+ 1, this.checkpoints.length- last_x_ok, {"x" : 1.0, "y" : inter.y});
			}
		}
	}


	crop_y() {
		for (let i=0; i<this.checkpoints.length; ++i) {
			if (this.checkpoints[i].y< 0.0) {
				this.checkpoints[i].y= 0.0;
			}
			if (this.checkpoints[i].y> 1.0) {
				this.checkpoints[i].y= 1.0;
			}
		}
	}


	functionalize() {
		let functionalized= functionalize_coords(this.checkpoints);
		this.checkpoints= functionalized;
		this.crop_x();
		this.crop_y();
	}
	

	add_point(coord) {
		this.checkpoints.push(coord);
		this.send_event();
		return this.checkpoints.length- 1;
	}
			

	add_point_on_line(idx_line, coord) {
		this.checkpoints.splice(idx_line+ 1, 0, coord);
		this.send_event();
	}
	
	
	remove_point(idx_point) {
		this.checkpoints.splice(idx_point, 1);
		this.send_event();
	}
	
	
	move_point(idx_point, coord) {
		this.checkpoints[idx_point]= coord;
		this.send_event();
	}

	
	set_editing_null() {
		this.editing_mode= null;
		this.editing_data= null;
	}


	update() {
		//console.log(this.checkpoints);
		this.svg_main_group.innerHTML= "";
		for (let i=0; i<this.checkpoints.length- 1; ++i) {
			this.svg_main_group.innerHTML+= '<line x1="'+ this.checkpoints[i].x+ '" y1="'+ this.checkpoints[i].y+ 
				'" x2="'+ this.checkpoints[i+ 1].x+
				'" y2="'+ this.checkpoints[i+ 1].y+
				'" class="lines" id="'+ this.get_svg_line_id(i)+ '" />\n';
		}
		for (let i=0; i<this.checkpoints.length; ++i) {
			this.svg_main_group.innerHTML+= '<circle cx="'+ this.checkpoints[i].x+ '" cy="'+ this.checkpoints[i].y+ 
				'" class="points" id="'+ this.get_svg_point_id(i)+ '" />\n';
		}
		
		let svg_lines= this.svg_main_group.getElementsByClassName("lines");
		for (let i=0; i<svg_lines.length; ++i) {
			svg_lines[i].addEventListener("mousedown", this.mouse_down_line.bind(this));
			svg_lines[i].addEventListener("mouseup", this.mouse_up_line.bind(this));
			svg_lines[i].addEventListener("mousemove", this.mouse_move.bind(this));
		}
		let svg_points= this.svg_main_group.getElementsByClassName("points");
		for (let i=0; i<svg_points.length; ++i) {
			svg_points[i].addEventListener("mousedown", this.mouse_down_point.bind(this));
			svg_points[i].addEventListener("mouseup", this.mouse_up_point.bind(this));
			svg_points[i].addEventListener("mousemove", this.mouse_move.bind(this));
		}
	}


	add_last_point_to_group() {
		let idx_point= this.checkpoints.length- 1;
		
		let line= document.createElementNS("http://www.w3.org/2000/svg", "line");
		this.svg_main_group.insertBefore(line, this.svg_main_group.getElementsByTagName("circle")[0]);
		line.setAttribute("x1", this.checkpoints[idx_point].x);
		line.setAttribute("y1", this.checkpoints[idx_point].y);
		line.setAttribute("x2", this.checkpoints[idx_point- 1].x);
		line.setAttribute("y2", this.checkpoints[idx_point- 1].y);
		line.setAttribute("id", this.get_svg_line_id(idx_point));
		line.classList.add("lines");
		line.addEventListener("mousedown", this.mouse_down_line.bind(this));
		line.addEventListener("mouseup", this.mouse_up_line.bind(this));
		line.addEventListener("mousemove", this.mouse_move.bind(this));
	
		let point= document.createElementNS("http://www.w3.org/2000/svg", "circle");
		this.svg_main_group.appendChild(point);
		point.setAttribute("cx", this.checkpoints[idx_point].x);
		point.setAttribute("cy", this.checkpoints[idx_point].y);
		point.setAttribute("id", this.get_svg_point_id(idx_point));
		point.classList.add("points");
		point.addEventListener("mousedown", this.mouse_down_point.bind(this));
		point.addEventListener("mouseup", this.mouse_up_point.bind(this));
		point.addEventListener("mousemove", this.mouse_move.bind(this));
	}


	update_point_position(idx_point) {
		let svg_point= document.getElementById(this.get_svg_point_id(idx_point));
		svg_point.setAttribute("cx", this.checkpoints[idx_point].x);
		svg_point.setAttribute("cy", this.checkpoints[idx_point].y);
	
		if (idx_point> 0) {
			let svg_line_before= document.getElementById(this.get_svg_line_id(idx_point- 1));
			svg_line_before.setAttribute("x2", this.checkpoints[idx_point].x);
			svg_line_before.setAttribute("y2", this.checkpoints[idx_point].y);
		}
		
		if (idx_point< this.checkpoints.length- 1) {
			let svg_line_after= document.getElementById(this.get_svg_line_id(idx_point));
			svg_line_after.setAttribute("x1", this.checkpoints[idx_point].x);
			svg_line_after.setAttribute("y1", this.checkpoints[idx_point].y);
		}
	}


	mouse_down(e) {
		if (DEBUG) { console.log("mouse_down") };
		
		let current_position= this.normalized_coords(e.offsetX, e.offsetY);

		if (this.keypresseds['x']) {
			this.init_checkpoints();
			this.update();
		}
		else if (this.keypresseds['t']) {
			this.editing_mode= "CREATE_TIMELINE";
			this.checkpoints= [];
			this.add_point(current_position);
			this.update();
		}
	}
	
	
	mouse_up(e) {
		if (DEBUG) { console.log("mouse_up") };

		//let current_position= this.normalized_coords(e.offsetX, e.offsetY);
		this.mouse_up_line(e);
		this.mouse_up_point(e);

	}


	mouse_move(e) {
		if (DEBUG) { console.log("mouse_move") };

		let current_position= this.normalized_coords(e.offsetX, e.offsetY);

		if (this.editing_mode== "CREATE_TIMELINE") {
			this.add_point(current_position);
			this.add_last_point_to_group();
		}
		else if (this.editing_mode== "MOVE_POINT") {
			this.move_point(this.editing_data["move_point_point_idx"], current_position);
			this.update_point_position(this.editing_data["move_point_point_idx"]);
		}
		else if (this.editing_mode== "MOVE_TIMELINE") {
			for (let i=0; i<this.checkpoints.length; ++i) {
				this.move_point(i, {
					"x" : this.editing_data["move_timeline_init"][i].x+ current_position.x- this.editing_data["move_timeline_start"].x,
					"y" : this.editing_data["move_timeline_init"][i].y+ current_position.y- this.editing_data["move_timeline_start"].y
				});
			}
			this.update();
		}
		else if (this.editing_mode== "MOVE_LINE") {
			this.move_point(this.editing_data["move_line_p0_idx"], {
				"x" : this.editing_data["move_line_p0"].x+ current_position.x- this.editing_data["move_line_start"].x,
				"y" : this.editing_data["move_line_p0"].y+ current_position.y- this.editing_data["move_line_start"].y
			});
			this.move_point(this.editing_data["move_line_p1_idx"], {
				"x" : this.editing_data["move_line_p1"].x+ current_position.x- this.editing_data["move_line_start"].x,
				"y" : this.editing_data["move_line_p1"].y+ current_position.y- this.editing_data["move_line_start"].y
			});
			this.update_point_position(this.editing_data["move_line_p0_idx"]);
			this.update_point_position(this.editing_data["move_line_p1_idx"]);
		}
	}


	mouse_out(e) {
		if (DEBUG) { console.log("mouse_out") };

		//let current_position= this.normalized_coords(e.offsetX, e.offsetY);
		if ((e.offsetX< 10) || (e.offsetX> this.svg_width- 10) || (e.offsetY< 10) || (e.offsetY> this.svg_height- 10)) {
			this.mouse_up_line(e);
			this.mouse_up_point(e);
		}
	}


	mouse_down_line(e) {
		if (DEBUG) { console.log("mouse_down_line") };

		let current_position= this.normalized_coords(e.offsetX, e.offsetY);

		let line_idx= this.get_line_idx(e.target.id);
	
		if (this.keypresseds['a']) {
			this.add_point_on_line(line_idx, current_position);
			this.update();
			this.set_editing_null();
		}
		else if (this.keypresseds['m']) {
			this.editing_mode= "MOVE_LINE";
			this.editing_data= {
				"move_line_p0_idx" : line_idx,
				"move_line_p1_idx" : line_idx+ 1,
				"move_line_start" : current_position,
				"move_line_p0" : deepcopy(this.checkpoints[line_idx]),
				"move_line_p1" : deepcopy(this.checkpoints[line_idx+ 1])
			};
		}
		else if (this.keypresseds['t']) {
			this.editing_mode= "MOVE_TIMELINE";
			this.editing_data= {
				"move_timeline_start" : current_position,
				"move_timeline_init" : deepcopy(this.checkpoints)
			};
		}
	}


	mouse_up_line(e) {
		if (DEBUG) { console.log("mouse_up_line"); };

		//let current_position= this.normalized_coords(e.offsetX, e.offsetY);

		if (this.editing_mode== "MOVE_LINE") {
			this.functionalize();
			this.update();
			this.set_editing_null();
		}
		else if (this.editing_mode== "MOVE_TIMELINE") {
			this.functionalize();
			this.update();
			this.set_editing_null();
		}

	}


	mouse_down_point(e) {
		if (DEBUG) { console.log("mouse_down_point") };

		let current_position= this.normalized_coords(e.offsetX, e.offsetY);

		let point_idx= this.get_point_idx(e.target.id);
	
		if (this.keypresseds['x']) {
			this.remove_point(point_idx);
			this.update();
			this.set_editing_null();
		}
		else if (this.keypresseds['t']) {
			this.editing_mode= "MOVE_TIMELINE";
			this.editing_data= {
				"move_timeline_start" : current_position,
				"move_timeline_init" : deepcopy(this.checkpoints)
			};
		}
		else if (this.keypresseds['m']) {
			this.editing_mode= "MOVE_POINT";
			this.editing_data= {
				"move_point_point_idx" : point_idx
			};
		}
	}


	mouse_up_point(e) {
		if (DEBUG) { console.log("mouse_up_point") };

		let current_position= this.normalized_coords(e.offsetX, e.offsetY);

		if (this.editing_mode== "CREATE_TIMELINE") {
			this.add_point(current_position);
			this.simplify_checkpoints(0.01, 0.01);
			this.functionalize();
			this.update();
			this.set_editing_null();
		}
		else if (this.editing_mode== "MOVE_POINT") {
			this.functionalize();
			this.update_point_position(this.editing_data["move_point_point_idx"]);
			this.set_editing_null();
		}
		else if (this.editing_mode== "MOVE_TIMELINE") {
			this.functionalize();
			this.update();
			this.set_editing_null();
		}
	
	}
}
