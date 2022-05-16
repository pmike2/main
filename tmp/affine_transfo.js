import {deepcopy} from "./utils.js";
import {norm, add, diff, angle, rad2deg} from "./geom.js";

const DEBUG= false;
const SIZE= 3.0;
const MARGIN_X= 0.3;
const MARGIN_Y= 0.3;
const INSTRUCTIONS= {
	"fond" : {
		"x" : "reinit"
	}
};



export class AffineTransfoContext {
	constructor(id, keypresseds) {
		this.id= id;
		
		this.center= {"x" : 0.0, "y" : 0.0};
		this.angle= 0.0;
		this.scale_x= 1.0;
		this.scale_y= 1.0;
		this.compute_u();
		this.compute_v();
		this.compute_mult_add();

		this.keypresseds= keypresseds;
		this.editing_mode= null;
		this.editing_data= null;
		
		this.svg_main= document.getElementById(this.id);
		this.svg_main.classList.add("affine_transfo_root");
		this.svg_width= this.svg_main.getAttribute("width");
		this.svg_height= this.svg_main.getAttribute("height");
		let xmin= -MARGIN_X- SIZE;
		let width= 2.0* (MARGIN_X+ SIZE);
		let ymin= -MARGIN_Y- SIZE;
		let height= 2.0* (MARGIN_Y+ SIZE);
		this.svg_main.setAttribute("viewBox", xmin+ " "+ ymin+ " "+ width+ " "+ height);
		
		this.svg_emprise_with_margin= document.createElementNS("http://www.w3.org/2000/svg", "rect");
		this.svg_emprise_with_margin.classList.add("affine_transfo_emprise_with_margin");
		this.svg_emprise_with_margin.setAttribute("x", xmin);
		this.svg_emprise_with_margin.setAttribute("y", ymin);
		this.svg_emprise_with_margin.setAttribute("width", width);
		this.svg_emprise_with_margin.setAttribute("height", height);
		this.svg_main.appendChild(this.svg_emprise_with_margin);

		this.svg_emprise= document.createElementNS("http://www.w3.org/2000/svg", "rect");
		this.svg_emprise.classList.add("affine_transfo_emprise");
		this.svg_emprise.setAttribute("x", -SIZE);
		this.svg_emprise.setAttribute("y", -SIZE);
		this.svg_emprise.setAttribute("width", 2.0* SIZE);
		this.svg_emprise.setAttribute("height", 2.0* SIZE);
		this.svg_main.appendChild(this.svg_emprise);
		
		this.svg_main_group= document.createElementNS("http://www.w3.org/2000/svg", "g");
		this.svg_main_group.classList.add("affine_transfo_main_group");
		// transform sert à avoir une origine en bas à gauche
		this.svg_main_group.setAttribute("transform", "matrix(1.0 0.0 0.0 -1.0 0.0 0.0)");
		this.svg_main.appendChild(this.svg_main_group);

		this.init_svg();
		
		this.svg_emprise_with_margin.addEventListener("mousedown", this.mouse_down.bind(this));
		this.svg_emprise_with_margin.addEventListener("mouseup", this.mouse_up.bind(this));
		this.svg_emprise_with_margin.addEventListener("mousemove", this.mouse_move.bind(this));
		this.svg_emprise_with_margin.addEventListener("mouseout", this.mouse_out.bind(this));

		let text_base_x= 1.8;
		let text_base2_x= 1.9;
		let text_base_y= -2.0;
		let text_step_y= 0.3;
		let compt= 0;
		for (let key in INSTRUCTIONS) {
			let t= document.createElementNS("http://www.w3.org/2000/svg", "text");
			t.classList.add("affine_transfo_text");
			t.setAttribute("x", text_base_x);
			t.setAttribute("y", text_base_y+ compt* text_step_y);
			compt+= 1;
			t.innerHTML= key;
			this.svg_main.appendChild(t);
	
			for (let key2 in INSTRUCTIONS[key]) {
				let t= document.createElementNS("http://www.w3.org/2000/svg", "text");
				t.classList.add("affine_transfo_text");
				t.setAttribute("x", text_base2_x);
				t.setAttribute("y", text_base_y+ compt* text_step_y);
				compt+= 1;
				t.innerHTML= key2+ " : "+ INSTRUCTIONS[key][key2];
				this.svg_main.appendChild(t);
			}
		}
	}


	debug() {
		if (this.id!= "svg_main_0") {
			return;
		}
		console.log("center= "+ JSON.stringify(this.center));
		console.log("u= "+ JSON.stringify(this.u));
		console.log("v= "+ JSON.stringify(this.v));
		console.log("angle= "+ JSON.stringify(this.angle));
		console.log("scale_x= "+ JSON.stringify(this.scale_x));
		console.log("scale_y= "+ JSON.stringify(this.scale_y));
		console.log("mult_matrix= "+ JSON.stringify(this.mult_matrix));
		console.log("add_vector= "+ JSON.stringify(this.add_vector));
	}


	send_event() {
		document.dispatchEvent(new CustomEvent("svg_affine_transfo_changed", {detail : {"svg_id" : this.id}}));
	}

	
	get_svg_center_id() {
		return this.id+ "_center";
	}


	get_svg_u_id() {
		return this.id+ "_u";
	}


	get_svg_v_id() {
		return this.id+ "_v";
	}


	normalized_coords(x, y) {
		return {"x" : 2* (SIZE+ MARGIN_X)* x/ this.svg_width- SIZE- MARGIN_X, "y" : SIZE- 2* (SIZE+ MARGIN_Y)* y/ this.svg_height+ MARGIN_Y};
	}


	compute_u() {
		this.u= {"x" : this.center.x+ this.scale_x* Math.cos(this.angle), "y" : this.center.y+ this.scale_x* Math.sin(this.angle)};
	}


	compute_v() {
		this.v= {"x" : this.center.x- this.scale_y* Math.sin(this.angle), "y" : this.center.y+ this.scale_y* Math.cos(this.angle)};
	}


	compute_angle_from_u() {
		this.angle= angle(this.center, add(this.center, {"x" : 1.0, "y" : 0.0}), this.u);
	}


	compute_angle_from_v() {
		this.angle= angle(this.center, add(this.center, {"x" : 0.0, "y" : 1.0}), this.v);
	}


	compute_scale_x() {
		this.scale_x= norm(diff(this.center, this.u));
	}


	compute_scale_y() {
		this.scale_y= norm(diff(this.center, this.v));
	}


	compute_mult_add() {
		this.mult_matrix= [this.scale_x* Math.cos(this.angle), this.scale_x* Math.sin(this.angle), -this.scale_y* Math.sin(this.angle), this.scale_y* Math.cos(this.angle)];
		this.add_vector= deepcopy(this.center);
	}


	init_transfo() {
		this.center= {"x" : 0.0, "y" : 0.0};
		this.angle= 0.0;
		this.scale_x= 1.0;
		this.scale_y= 1.0;
		this.compute_u();
		this.compute_v();
		this.compute_mult_add();

		this.send_event();
	}


	set_transfo(mult_matrix, add_vector) {
		this.center= deepcopy(add_vector);
		this.scale_x= norm({"x" : mult_matrix[0], "y" : mult_matrix[1]});
		this.scale_y= norm({"x" : mult_matrix[2], "y" : mult_matrix[3]});
		this.angle= Math.acos(mult_matrix[0]/ this.scale_x);
		this.compute_u();
		this.compute_v();
		this.compute_mult_add();
	}


	move_center(coord) {
		this.center= coord;
		this.compute_u();
		this.compute_v();
		this.compute_mult_add();

		this.send_event();
	}
		
	
	move_u(coord) {
		this.u= coord;
		this.compute_angle_from_u();
		this.compute_scale_x();
		this.compute_v();
		this.compute_mult_add();

		this.send_event();
	}

	
	move_v(coord) {
		this.v= coord;
		this.compute_angle_from_v();
		this.compute_scale_y();
		this.compute_u();
		this.compute_mult_add();

		this.send_event();
	}

	
	set_editing_null() {
		this.editing_mode= null;
		this.editing_data= null;
	}


	init_svg() {
		this.svg_main_group.innerHTML= "";
		this.svg_main_group.innerHTML+= '<circle cx="0.0" cy="0.0" r="1.0" class="affine_transfo_circle_init" />\n';
		this.svg_main_group.innerHTML+= '<line x1="0.0" y1="0.0" x2="1.0" y2="0.0" class="affine_transfo_line_u_init" />\n';
		this.svg_main_group.innerHTML+= '<line x1="0.0" y1="0.0" x2="0.0" y2="1.0" class="affine_transfo_line_v_init" />\n';

		this.svg_main_group.innerHTML+= '<g class="affine_transfo_elliptic_group" transform="translate('+ this.center.x+ ' '+ this.center.y+ ') rotate('+ rad2deg(this.angle)+ ') scale('+ this.scale_x+ ' '+ this.scale_y+ ')"><circle cx="0.0" cy="0.0" r="1.0" class="affine_transfo_circle_init"></g>';

		this.svg_main_group.innerHTML+= '<line x1="'+ this.center.x+ '" y1="'+ this.center.y+ 
			'" x2="'+ this.u.x+
			'" y2="'+ this.u.y+
			'" class="affine_transfo_line_u" />\n';

		this.svg_main_group.innerHTML+= '<line x1="'+ this.center.x+ '" y1="'+ this.center.y+ 
		'" x2="'+ this.v.x+
		'" y2="'+ this.v.y+
		'" class="affine_transfo_line_v" />\n';

		this.svg_main_group.innerHTML+= '<circle cx="'+ this.center.x+ '" cy="'+ this.center.y+ 
			'" class="affine_transfo_circle" id="'+ this.get_svg_center_id()+ '" />\n';
		
		this.svg_main_group.innerHTML+= '<circle cx="'+ this.u.x+ '" cy="'+ this.u.y+ 
		'" class="affine_transfo_circle" id="'+ this.get_svg_u_id()+ '" />\n';

		this.svg_main_group.innerHTML+= '<circle cx="'+ this.v.x+ '" cy="'+ this.v.y+ 
		'" class="affine_transfo_circle" id="'+ this.get_svg_v_id()+ '" />\n';

		let svg_center= document.getElementById(this.get_svg_center_id());
		let svg_u= document.getElementById(this.get_svg_u_id());
		let svg_v= document.getElementById(this.get_svg_v_id());
		[svg_center, svg_u, svg_v].forEach(svg_pt => {
			svg_pt.addEventListener("mousedown", this.mouse_down_point.bind(this));
			svg_pt.addEventListener("mouseup", this.mouse_up_point.bind(this));
			svg_pt.addEventListener("mousemove", this.mouse_move.bind(this));
		});
	}


	update() {
		let elliptic_group= this.svg_main_group.getElementsByClassName("affine_transfo_elliptic_group")[0];
		elliptic_group.setAttribute("transform", 'translate('+ this.center.x+ ' '+ this.center.y+ ') rotate('+ rad2deg(this.angle)+ ') scale('+ this.scale_x+ ' '+ this.scale_y+ ')');

		let line_u= this.svg_main_group.getElementsByClassName("affine_transfo_line_u")[0];
		let line_v= this.svg_main_group.getElementsByClassName("affine_transfo_line_v")[0];
		line_u.setAttribute("x1", this.center.x);
		line_u.setAttribute("y1", this.center.y);
		line_u.setAttribute("x2", this.u.x);
		line_u.setAttribute("y2", this.u.y);
		line_v.setAttribute("x1", this.center.x);
		line_v.setAttribute("y1", this.center.y);
		line_v.setAttribute("x2", this.v.x);
		line_v.setAttribute("y2", this.v.y);

		let svg_center= document.getElementById(this.get_svg_center_id());
		let svg_u= document.getElementById(this.get_svg_u_id());
		let svg_v= document.getElementById(this.get_svg_v_id());
		svg_center.setAttribute("cx", this.center.x);
		svg_center.setAttribute("cy", this.center.y);
		svg_u.setAttribute("cx", this.u.x);
		svg_u.setAttribute("cy", this.u.y);
		svg_v.setAttribute("cx", this.v.x);
		svg_v.setAttribute("cy", this.v.y);
	}


	mouse_down(e) {
		if (DEBUG) { console.log("mouse_down") };
		
		let current_position= this.normalized_coords(e.offsetX, e.offsetY);

		if (this.keypresseds['x']) {
			this.init_transfo();
			this.update();
		}
	}
	
	
	mouse_up(e) {
		if (DEBUG) { console.log("mouse_up") };

		//let current_position= this.normalized_coords(e.offsetX, e.offsetY);
		this.mouse_up_point(e);
	}


	mouse_move(e) {
		if (DEBUG) { console.log("mouse_move") };

		let current_position= this.normalized_coords(e.offsetX, e.offsetY);

		if (this.editing_mode== "MOVE_CENTER") {
			this.move_center(current_position);
			this.update();
		}
		else if (this.editing_mode== "MOVE_U") {
			this.move_u(current_position);
			this.update();
		}
		else if (this.editing_mode== "MOVE_V") {
			this.move_v(current_position);
			this.update();
		}
	}


	mouse_out(e) {
		if (DEBUG) { console.log("mouse_out") };

		//let current_position= this.normalized_coords(e.offsetX, e.offsetY);
		if ((e.offsetX< 10) || (e.offsetX> this.svg_width- 10) || (e.offsetY< 10) || (e.offsetY> this.svg_height- 10)) {
			this.mouse_up_point(e);
		}
	}


	mouse_down_point(e) {
		if (DEBUG) { console.log("mouse_down_point") };

		//let current_position= this.normalized_coords(e.offsetX, e.offsetY);

		if (e.target.id== this.get_svg_center_id()) {
			this.editing_mode= "MOVE_CENTER";
		}
		else if (e.target.id== this.get_svg_u_id()) {
			this.editing_mode= "MOVE_U";
		}
		else if (e.target.id== this.get_svg_v_id()) {
			this.editing_mode= "MOVE_V";
		}
	}


	mouse_up_point(e) {
		if (DEBUG) { console.log("mouse_up_point") };

		//let current_position= this.normalized_coords(e.offsetX, e.offsetY);

		if (this.editing_mode== "MOVE_CENTER") {
			this.set_editing_null();
		}
		else if (this.editing_mode== "MOVE_U") {
			this.set_editing_null();
		}
		else if (this.editing_mode== "MOVE_V") {
			this.set_editing_null();
		}
	
	}
}
