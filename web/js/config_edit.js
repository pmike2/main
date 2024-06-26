import {deepcopy} from "./utils.js";

export class ConfigEdit {
	constructor(js_model, js_configs) {
		this.js_model= js_model;
		this.js_configs= js_configs;
		const first_config_key= Object.keys(this.js_configs)[0];
		this.js_config= this.js_configs[first_config_key];

		this.gen_utilities();
		this.gen_load(first_config_key);

		this.expand_const();
		this.complexify_keys();

		this.div_main= document.body;

		this.gen_dom("/", this.div_main);
	}


	send_event(node_id) {
		document.dispatchEvent(new CustomEvent("js_config_changed", {detail : {"node_id" : node_id}}));
	}

	
	concat_ids(parent_id, child_id) {
		if (parent_id== "/") {
			return "/"+ child_id;
		}
		return parent_id + "/"+ child_id;
	}


	get_parent_id(node_id) {
		if (node_id== "/") {
			//return "/";
			return null;
		}
		let result= node_id.substring(0, node_id.lastIndexOf("/"));
		if (result== "") {
			result= "/";
		}
		return result;
	}


	get_children_ids(node_id) {
		let result= [];
		let [js_parent_node, key]= this.get_config_node(node_id);
		for (let i in js_parent_node[key]) {
			result.push(this.concat_ids(node_id, i));
		}
		return result;
	}


	get_all_config_ids() {
		let result= [];
	
		let get_node_id= (js_node, id) => {
			result.push(id);
			if (typeof(js_node)== "object") {
				for (let key in js_node) {
					if (id!= "") {
						get_node_id(js_node[key], this.concat_ids(id, key));
					}
					else {
						get_node_id(js_node[key], key);
					}
				}
			}
		}
	
		get_node_id(this.js_config, "/");
	
		return result;
	}


	get_config_node(node_id, js_config) {
		js_config= js_config || this.js_config;

		if (node_id[0]== "/") {
			node_id= node_id.substring(1);
		}
		if (node_id=== "") {
			return [js_config, null]
		}
		let keys= node_id.split("/");
		let js_parent_node= js_config;
		for (let i=0; i<keys.length- 1; ++i) {
			if (keys[i] in js_parent_node) {
				js_parent_node= js_parent_node[keys[i]];
			}
			else {
				return [null, null];
			}
		}
		let key= keys[keys.length- 1];
		if (key in js_parent_node) {
			return [js_parent_node, key];
		}
		return [null, null];
	}

	
	get_model_node(node_id) {
		if (node_id[0]== "/") {
			node_id= node_id.substring(1);
		}
		if (node_id=== "") {
			return this.js_model["__root__"];
		}
		let keys= node_id.split("/");
		let model_node= this.js_model["__root__"];
		for (let i=0; i<keys.length; ++i) {
			if (model_node.type== "dict") {
				if ("keys" in model_node) {
					let found= false;
					for (let j=0; j<model_node.keys.length; ++j) {
						if (model_node.keys[j]== keys[i]) {
							model_node= this.js_model[model_node.keys[j]];
							found= true;
							break;
						}
					}
					if (!found) {
						return null;
					}
				}
				else if ("values" in model_node) {
					model_node= this.js_model[model_node.values];
				}
			}
			else if (model_node.type== "list") {
				model_node= this.js_model[model_node.values];
			}
		}
		return model_node;
	}

	


	get_default(node_id) {
	
		let get_default_node = (js, node_id) => {
			let model_node= this.get_model_node(node_id);
			let key= node_id.split("/").pop();
	
			if ((model_node.type== "int") || (model_node.type== "float") || (model_node.type== "string")) {
				if (model_node.default== "random") {
					if (model_node.type== "int") {
						js[key]= model_node.min+ Math.floor(Math.random()* (model_node.max- model_node.min));
					}
					else if (model_node.type== "float") {
						js[key]= model_node.min+ Math.random()* (model_node.max- model_node.min);
					}
				}
				else {
					js[key]= model_node.default;
				}
			}
			else if (model_node.type== "dict") {
				if ("keys" in model_node) {
					js[key]= {};
					for (let i=0; i<model_node.keys.length; ++i) {
						get_default_node(js[key], this.concat_ids(node_id, model_node.keys[i]));
					}
				}
				else if ("possible_keys" in model_node) {
					js[key]= {};
					get_default_node(js[key], this.concat_ids(node_id, model_node.possible_keys[0]));
				}
			}
			else if (model_node.type== "list") {
				let n= 1;
				if ("number" in model_node) {
					n= model_node.number;
				}
				else if ("min_number" in model_node) {
					n= model_node.min_number;
				}
				js[key]= [];
				for (let i=0; i<n; ++i) {
					get_default_node(js[key], this.concat_ids(node_id, i));
				}
			}
		}
	
		let result= {};
		let key= node_id.split("/").pop();
		get_default_node(result, node_id);
		//console.log("get_default : "+ JSON.stringify(result[key], null, 4));
		return result[key];
	}
	
		
	reset2default() {
		this.js_config= this.get_default("/");
		this.gen_dom("/", this.div_main);
		this.send_event("/");
	}
	

	add_default_to_list(node_id, i) {
		let [js_parent_node, key]= this.get_config_node(node_id);
		let model_node= this.get_model_node(node_id);

		if (key!== null) {
			if (("max_number" in model_node) && (model_node.max_number== js_parent_node[key].length)) {
				return;
			}
			js_parent_node[key].push(this.get_default(this.concat_ids(node_id, i)));
		}
		// root
		else {
			if (("max_number" in model_node) && (model_node.max_number== js_parent_node.length)) {
				return;
			}
			js_parent_node.push(this.get_default(this.concat_ids(node_id, i)));
		}
	}
	

	add_default_to_dict(node_id, k) {
		let [js_parent_node, key]= this.get_config_node(node_id);
		if (key!== null) {
			js_parent_node[key][k]= this.get_default(this.concat_ids(node_id, k));
		}
		// root
		else {
			js_parent_node[k]= this.get_default(this.concat_ids(node_id, k));
		}
	}

	
	delete_id(node_id) {
		if (node_id== "/") {
			this.js_config= {};
			return;
		}
		let [js_parent_node, key]= this.get_config_node(node_id);
		let parent_model_node= this.get_model_node(this.get_parent_id(node_id));
		if (parent_model_node.type== "dict") {
			delete js_parent_node[key];
		}
		else if (parent_model_node.type== "list") {
			if ("number" in parent_model_node) {
				return;
			}
			if (("min_number" in parent_model_node) && (parent_model_node.min_number== js_parent_node.length)) {
				return;
			}
			js_parent_node.splice(key, 1);
		}
	}

	
	randomize(node_id) {
		let [js_parent_node, key]= this.get_config_node(node_id);
		let model_node= this.get_model_node(node_id);
		
		/*console.log("------");
		console.log(node_id);
		console.log(js_parent_node);
		console.log(key);
		console.log(model_node);*/
		

		if (model_node.type== "list") {
			if (key!== null) {
				js_parent_node[key]= [];
			}
			// root
			else {
				this.js_config= [];
			}
			let min_number= 0;
			let max_number= 10; // TODO : forcer l'existence de model_node.max_number ?
			if ("min_number" in model_node) {
				min_number= model_node.min_number;
			}
			if ("max_number" in model_node) {
				max_number= model_node.max_number;
			}
			let n= min_number+ Math.floor(Math.random()* (max_number- min_number));
			if ("number" in model_node) {
				n= model_node.number;
			}
			for (let i=0; i<n; ++i) {
				this.add_default_to_list(node_id, i);
				this.randomize(this.concat_ids(node_id, i));
			}
		}
		else if (model_node.type== "dict") {
			if ("keys" in model_node) {
				for (let i=0; i<model_node.keys.length; ++i) {
					this.randomize(this.concat_ids(node_id, model_node.keys[i]));
				}
			}
			else if ("possible_keys" in model_node) {
				if (key!== null) {
					js_parent_node[key]= {};
				}
				// root
				else {
					this.js_config= {};
				}
				let n= Math.floor(Math.random()* model_node.possible_keys.length);
				for (let i=0; i<n; ++i) {
					this.add_default_to_dict(node_id, model_node.possible_keys[i]);
					this.randomize(this.concat_ids(node_id, model_node.possible_keys[i]));
				}
			}
		}
		else {
			if ("possible_values" in model_node) {
				let i= Math.floor(Math.random()* model_node.possible_values.length);
				js_parent_node[key]= model_node.possible_values[i];
			}
			else {
				if (model_node.type== "int") {
					js_parent_node[key]= model_node.min+ Math.floor(Math.random()* (model_node.max+ 1- model_node.min));
				}
				else if (model_node.type== "float") {
					js_parent_node[key]= model_node.min+ Math.random()* (model_node.max- model_node.min);
				}
			}
		}
	}
	

	expand_const() {
		for (let key in this.js_model) {
			if (key!= "__const__") {
				for (let k in this.js_model[key]) {
					for (let key_const in this.js_model["__const__"]) {
						if (this.js_model[key][k]== key_const) {
							this.js_model[key][k]= this.js_model["__const__"][key_const];
						}
					}
				}
			}
		}
	}
	

	get_complexified_id(node_id) {
		let keys= node_id.split("/");
		for (let i=keys.length- 1; i>=0; --i) {
			let found= false;
			for (let k in this.js_model) {
				if ((k.includes(".")) && (k.split(".")[1]== keys[i])) {
					for (let j=i- 1; j>=0; --j){
						if (keys[j]== k.split(".")[0]) {
							keys[i]= k;
							found= true;
							break;
						}
					}
				}
				if (found) {
					break;
				}
			}
		}
		return keys.join("/");
	}


	get_complexified_key(node_id) {
		return this.get_complexified_id(node_id).split("/").pop();
	}


	complexify_keys() {
		let all_config_ids= this.get_all_config_ids();
		all_config_ids.sort(function(a, b) {
			return b.length- a.length;
		});
		for (let i=0; i<all_config_ids.length; ++i) {
			let [js_parent_node, key]= this.get_config_node(all_config_ids[i]);
			if (key=== null) {
				continue;
			}
			let complexified_key= this.get_complexified_key(all_config_ids[i]);
			if (complexified_key!= key) {
				js_parent_node[complexified_key]= deepcopy(js_parent_node[key]);
				delete js_parent_node[key];
			}
		}
	}


	get_simplified_key(key) {
		if (key.includes(".")) {
			return key.split(".").pop();
		}
		return key;
	}


	get_simplified_config() {
		let all_config_ids= this.get_all_config_ids();
		all_config_ids.sort(function(a, b) {
			return b.length- a.length;
		});

		let result= deepcopy(this.js_config);
	
		for (let i=0; i<all_config_ids.length; ++i) {
			let [js_parent_node, key]= this.get_config_node(all_config_ids[i], result);
			if ((key!== null) && (key.includes("."))) {
				js_parent_node[this.get_simplified_key(key)]= deepcopy(js_parent_node[key]);
				delete js_parent_node[key];
			}
		}

		return result;
	}

	
	enforce_rules() {
		for (let i=0; i<this.js_model["__rules__"].length; ++i) {
			let f = new Function("js_config", this.js_model["__rules__"][i]);
			f(this.js_config);
		}
	}


	get_div_node(node_id) {
		return document.querySelector('div[node_id="'+ node_id+ '"]');
	}


	get_fold_button_node(node_id) {
		return document.querySelector('button[node_id="'+ node_id+ '"].fold-button');
	}


	is_fold(node_id) {
		let div_node= this.get_div_node(node_id);
		return div_node.classList.contains("folded");
	}



	toggle_fold(node_id) {
		if (this.is_fold(node_id)) {
			this.unfold(node_id);
		}
		else {
			this.fold(node_id);
		}
	}


	fold(node_id) {
		let div_node= this.get_div_node(node_id);
		let button_node= this.get_fold_button_node(node_id);
		if ((div_node=== null) || (button_node=== null)) {
			return;
		}
		if (!this.is_fold(node_id)) {
			div_node.classList.add("folded");
		}
		button_node.innerHTML= ">";
	}


	unfold(node_id) {
		let div_node= this.get_div_node(node_id);
		let button_node= this.get_fold_button_node(node_id);
		if ((div_node=== null) || (button_node=== null)) {
			return;
		}
		if (this.is_fold(node_id)) {
			div_node.classList.remove("folded");
		}
		button_node.innerHTML= "v";
	}


	fold_all() {
		let node_ids= this.get_all_config_ids();
		for (let i=0; i<node_ids.length; ++i) {
			this.fold(node_ids[i]);
		}
	}


	unfold_all() {
		let node_ids= this.get_all_config_ids();
		for (let i=0; i<node_ids.length; ++i) {
			this.unfold(node_ids[i]);
		}
	}

	
	gen_dom(node_id, parent_html_node) {
		let [js_parent_node, key]= this.get_config_node(node_id);
		let model_node= this.get_model_node(node_id);

		/*console.log("------");
		console.log(node_id);
		console.log(parent_html_node);
		console.log(JSON.stringify(js_parent_node));
		console.log(key);
		console.log(model_node);*/

		if ("nodom" in model_node) {
			return;
		}
	
		let div_elmt= this.get_div_node(node_id);
		let was_fold= null;
		if (div_elmt!== null) {
			div_elmt.innerHTML= "";
			was_fold= this.is_fold(node_id);
		}
		else {
			div_elmt= document.createElement("div");
			div_elmt.setAttribute("node_id", node_id);
			// on insere cette div après la dernière des div déjà présentes et notamment avant tout ce qui n'est pas div (svg...)
			let siblings= document.querySelectorAll('div[node_id="'+ this.get_parent_id(node_id)+ '"] > div');
			if (siblings.length> 0) {
				parent_html_node.insertBefore(div_elmt, siblings[siblings.length- 1].nextSibling);
			}
			else {
				parent_html_node.appendChild(div_elmt);
			}
		}
		
		if (key!== null) {
			//div_elmt.innerHTML= "<span>"+ this.get_simplified_key(key)+ "</span>";
			let button_info_elmt= document.createElement("button");
			button_info_elmt.innerHTML= this.get_simplified_key(key);
			button_info_elmt.classList.add("info-button");
			div_elmt.insertBefore(button_info_elmt, div_elmt.firstChild);
		}

		let button_randomize_elmt= document.createElement("button");
		button_randomize_elmt.setAttribute("node_id", node_id);
		button_randomize_elmt.innerHTML= "R";
		button_randomize_elmt.classList.add("randomize-button");
		button_randomize_elmt.addEventListener("click", () => {
			this.randomize(node_id);
			this.enforce_rules();
			this.gen_dom(node_id, parent_html_node);
			this.send_event(node_id);
		});
		div_elmt.insertBefore(button_randomize_elmt, div_elmt.firstChild);

		if (model_node.type== "list") {
			let button_add_elmt= document.createElement("button");
			button_add_elmt.setAttribute("node_id", node_id);
			button_add_elmt.innerHTML= "+";
			button_add_elmt.classList.add("add-button");
			button_add_elmt.addEventListener("click", () => {
				if (key!== null) {
					this.add_default_to_list(node_id, js_parent_node[key].length);
				}
				// root
				else {
					this.add_default_to_list(node_id, js_parent_node.length);
				}
				this.enforce_rules();
				this.gen_dom(node_id, parent_html_node);
				this.send_event(node_id);
			});
			div_elmt.insertBefore(button_add_elmt, div_elmt.firstChild);
		}
		else if ((model_node.type== "dict") && ("possible_keys" in model_node)) {
			let button_add_elmt= document.createElement("button");
			button_add_elmt.setAttribute("node_id", node_id);
			button_add_elmt.innerHTML= "+";
			button_add_elmt.classList.add("add-button");
			button_add_elmt.addEventListener("click", () => {
				let idx_key= 0;
				if ((key!== null) && (Object.keys(js_parent_node[key]).length> 0)) {
					idx_key= Math.max(...Object.keys(js_parent_node[key]).map(x => model_node.possible_keys.indexOf(x)))+ 1;
				}
				// root
				else if ((key=== null) && (Object.keys(js_parent_node).length> 0)) {
					idx_key= Math.max(...Object.keys(js_parent_node).map(x => model_node.possible_keys.indexOf(x)))+ 1;
				}
				this.add_default_to_dict(node_id, model_node.possible_keys[idx_key]);
				this.enforce_rules();
				this.gen_dom(node_id, parent_html_node);
				this.send_event(node_id);
			});
			div_elmt.insertBefore(button_add_elmt, div_elmt.firstChild);
		}

		let parent_node_id= this.get_parent_id(node_id);
		if (parent_node_id!== null) {
			let parent_model_node= this.get_model_node(parent_node_id);
			if ((parent_model_node.type== "list") || ((parent_model_node.type== "dict") && ("possible_keys" in parent_model_node))) {
				let button_remove_elmt= document.createElement("button");
				button_remove_elmt.setAttribute("node_id", node_id);
				button_remove_elmt.innerHTML= "-";
				button_remove_elmt.classList.add("remove-button");
				button_remove_elmt.addEventListener("click", () => {
					this.delete_id(node_id);
					this.gen_dom(parent_node_id, parent_html_node.parentNode);
					this.send_event(parent_node_id);
				});
				div_elmt.insertBefore(button_remove_elmt, div_elmt.firstChild);
			}
		}

		if ((model_node.type== "int") || (model_node.type== "float") || (model_node.type== "string")) {
			
			if ("possible_values" in model_node) {
				let select_elmt= document.createElement("select");

				for (let i=0; i<model_node.possible_values.length; ++i) {
					let possible_value= model_node.possible_values[i];
					let option_elmt= document.createElement("option");
					option_elmt.setAttribute("value", possible_value);
					if ((typeof possible_value=== 'string') && (possible_value.includes("/"))) {
						option_elmt.innerHTML= possible_value.split("/").pop();
					}
					else {
						option_elmt.innerHTML= possible_value;
					}
					select_elmt.appendChild(option_elmt);
				}

				if (model_node.type== "string") {
					select_elmt.classList.add("long-select");
				}
				
				select_elmt.addEventListener('change', (e) => {
					if (model_node.type== "int") {
						js_parent_node[key]= parseInt(select_elmt.value);
					}
					else if (model_node.type== "float") {
						js_parent_node[key]= parseFloat(select_elmt.value);
					}
					else if (model_node.type== "string") {
						js_parent_node[key]= select_elmt.value;
					}
					this.send_event(node_id);
				});

				// si le json contient une valeur en accord avec ce qui est possible on la sélectionne sinon on choisit la 1ere
				if (model_node.possible_values.includes(js_parent_node[key])) {
					select_elmt.value= js_parent_node[key];
				}
				else {
					select_elmt.selectedIndex= 0;
				}

				div_elmt.appendChild(select_elmt);
			}
			else {
				let input_elmt= document.createElement("input");

				input_elmt.addEventListener('input', (e) => {
					if (model_node.type== "int") {
						js_parent_node[key]= parseInt(input_elmt.value);
					}
					else if (model_node.type== "float") {
						js_parent_node[key]= parseFloat(input_elmt.value);
					}
					else if (model_node.type== "string") {
						js_parent_node[key]= input_elmt.value;
					}
					this.send_event(node_id);
				});
				div_elmt.appendChild(input_elmt);
	
				if ((model_node.type== "int") || (model_node.type== "float")) {
					//input_elmt.setAttribute("type", "number");
					input_elmt.setAttribute("type", "range"); // + sympa
					input_elmt.classList.add("long-input");
					
					if ("min" in model_node) {
						input_elmt.setAttribute("min", model_node.min);
						input_elmt.addEventListener("change", function() {
							if (parseFloat(this.value)< parseFloat(model_node.min)) {
								this.value= model_node.min;
							}
						});
					}
					if ("max" in model_node) {
						input_elmt.setAttribute("max", model_node.max);
						input_elmt.addEventListener("change", function() {
							if (parseFloat(this.value)> parseFloat(model_node.max)) {
								this.value= model_node.max;
							}
						});
					}
					if (model_node.type== "float") {
						input_elmt.setAttribute("step", 0.001);
					}

					let slider_value_elmt= document.createElement("input");
					slider_value_elmt.value= input_elmt.value;
					slider_value_elmt.setAttribute("readonly", true);
					div_elmt.appendChild(slider_value_elmt);

					input_elmt.addEventListener("change", function () {
						slider_value_elmt.value= this.value;
					});

					input_elmt.value= js_parent_node[key];

					// pour que slider_value_elmt suive le changement
					input_elmt.dispatchEvent(new Event('change'));
				}
				else if (model_node.type== "string") {
					input_elmt.classList.add("long-input");
				}
			}
		}
		else if ((model_node.type== "dict") || (model_node.type== "list")) {
			let button_fold_elmt= document.createElement("button");
			button_fold_elmt.setAttribute("node_id", node_id);
			button_fold_elmt.classList.add("fold-button");
			button_fold_elmt.addEventListener("click", () => {
				this.toggle_fold(node_id);
			});
			
			div_elmt.insertBefore(button_fold_elmt, div_elmt.firstChild);
			if (was_fold=== true) {
				this.fold(node_id);
			}
			else if (was_fold=== false) {
				this.unfold(node_id);
			}
			else if (was_fold=== null) {
				this.fold(node_id);
			}

			if (node_id== "/") {
				for (let k in js_parent_node) {
					this.gen_dom(this.concat_ids(node_id, k), div_elmt);
				}
			}
			else {
				for (let k in js_parent_node[key]) {
					this.gen_dom(this.concat_ids(node_id, k), div_elmt);
				}
			}
		}
	}


	gen_utilities() {
		let div_utilities= document.getElementById("div_utilities");
		if (div_utilities!== null) {
			div_utilities.remove();
		}
		div_utilities= document.createElement("div");
		div_utilities.setAttribute("id", "div_utilities");
		document.body.appendChild(div_utilities);

		let save_button= document.createElement("button");
		save_button.classList.add("button_utilities");
		save_button.addEventListener("click", () => {
			document.dispatchEvent(new CustomEvent("js_config_save"));
		});
		save_button.innerHTML= "save";
		div_utilities.appendChild(save_button);

		let fold_button= document.createElement("button");
		fold_button.classList.add("button_utilities");
		fold_button.addEventListener("click", this.fold_all.bind(this));
		fold_button.innerHTML= "fold";
		div_utilities.appendChild(fold_button);

		let unfold_button= document.createElement("button");
		unfold_button.classList.add("button_utilities");
		unfold_button.addEventListener("click", this.unfold_all.bind(this));
		unfold_button.innerHTML= "unfold";
		div_utilities.appendChild(unfold_button);

		let reset_button= document.createElement("button");
		reset_button.classList.add("button_utilities");
		reset_button.addEventListener("click", this.reset2default.bind(this));
		reset_button.innerHTML= "reset";
		div_utilities.appendChild(reset_button);

	}


	gen_load(config_key) {
		let load_select= document.getElementById("load_select");
		if (load_select=== null) {
			load_select= document.createElement("select");
			
			load_select.value= config_key;

			for (let key in this.js_configs) {
				let option_elmt= document.createElement("option");
				option_elmt.setAttribute("value", key);
				option_elmt.innerHTML= key.split("/").pop();
				load_select.appendChild(option_elmt);
			}
			
			load_select.classList.add("long-select");
			load_select.addEventListener("change", () => {
				this.js_config= this.js_configs[load_select.value];
				this.complexify_keys();
				this.gen_dom("/", this.div_main);
				this.send_event("/");
			});
			div_utilities.appendChild(load_select);
		}
	}


	test() {
		//let node_id= "/readers";
		//console.log(this.get_children_ids(node_id));
		//console.log(this.get_parent_id(node_id));
		//console.log(this.get_all_config_ids());
		//console.log(this.get_config_node(node_id));
		//console.log(this.get_model_node(node_id));
		//console.log(this.get_default(node_id));
		//let [js_parent_node, key]= this.get_config_node(node_id);
		//js_parent_node[key]= [];
		//this.add_default_to_list(node_id, 0);
		//this.add_default_to_dict(node_id, "d");
		//this.delete_id(node_id);
		//this.randomize(node_id);
		//this.enforce_rules(js_config);
		//this.simplify_keys();
		//console.log(this.js_config);
		//this.fold_all();
	}

}

