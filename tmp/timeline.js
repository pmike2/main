export class TimeLineContext {
	constructor(id, keypresseds) {
		this.id= id;
		
		this.checkpoints= [];
		this.keypresseds= keypresseds;
		//this.editing_mode= null;
		//this.editing_data= null;
		
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
	}


	normalized_coords(x, y) {
		return {"x" : x/ this.svg_width, "y" : 1.0- y/ this.svg_height};
	}


	mouse_down_emprise(e) {
		let current_position= this.normalized_coords(e.offsetX, e.offsetY);
	}
	
	
	mouse_up_emprise(e) {
		let current_position= this.normalized_coords(e.offsetX, e.offsetY);
	}
}


