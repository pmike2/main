
export function norm(p) {
	return Math.sqrt(p.x* p.x+ p.y* p.y);
}


export function normalized(p) {
	let n= norm(p);
	if (n< 1e-8) {
		return null;
	}
	return {"x" : p.x/ n, "y" : p.y/ n};
}


export function add(p1, p2) {
	return {"x" : p1.x+ p2.x, "y" : p1.y+ p2.y};
}


export function diff(p1, p2) {
	return {"x" : p1.x- p2.x, "y" : p1.y- p2.y};
}


export function scalar_product(p1, p2) {
	return p1.x* p2.x+ p1.y* p2.y;
}


export function cross_product(p1, p2) {
	return  p1.x* p2.y- p1.y* p2.x;
}


// est-ce que situé en pt_ref, regardant vers dir_ref, pt_test est à gauche
export function is_left(pt_ref, dir_ref, pt_test) {
	return cross_product(diff(pt_test, pt_ref), dir_ref)<= 0.0;
}


export function orthogonal(p) {
	return {"x" : p.y, "y" : -1.0* p.x};
}


export function inertia_center(coords) {
	if (coords.length== 0) {
		return null;
	}

	let result= {"x" : 0.0, "y" : 0.0};
	for (let i=0; i<coords.length; ++i) {
		result.x+= coords[i].x;
		result.y+= coords[i].y;
	}
	result.x/= coords.length;
	result.y/= coords.length;

	return result;
}


export function bbox(coords) {
	if (coords.length== 0) {
		return null;
	}

	let result= {"xmin" : 1e7, "ymin" : 1e7, "xmax" : -1e7, "ymax" : -1e7};
	for (let i=0; i<coords.length; ++i) {
		if (coords[i].x< result.xmin) {
			result.xmin= coords[i].x;
		}
		if (coords[i].x> result.xmax) {
			result.xmax= coords[i].x;
		}
		if (coords[i].y< result.ymin) {
			result.ymin= coords[i].y;
		}
		if (coords[i].y> result.ymax) {
			result.ymax= coords[i].y;
		}
	}

	return result;
}


export function cos_angle(base, p1, p2) {
	let n1= norm(diff(base, p1));
	let n2= norm(diff(base, p2));
	if ((n1< 1e-8) || (n2< 1e-8)) {
		return null;
	}
	return scalar_product(diff(p1, base), diff(p2, base))/ (n1* n2);
}


export function angle(base, p1, p2) {
	if (is_left(base, diff(p1, base), p2)) {
		return Math.acos(cos_angle(base, p1, p2));
	}
	else {
		return Math.PI* 2.0- Math.acos(cos_angle(base, p1, p2));
	}
}


export function rotate(pt, center, a) {
	return {
		"x" : center.x+ (pt.x- center.x)* Math.cos(a)- (pt.y- center.y)* Math.sin(a),
		"y" : center.y+ (pt.x- center.x)* Math.sin(a)+ (pt.y- center.y)* Math.cos(a)
	}
}


export function simplify_coords(coords, treshold, min_dist) {
	if (coords.length< 3) {
		return coords;
	}

	let result= [];
	result.push(coords[0]);
	let base= 0;
	let base2= 1;
	let tested= 2;
	let done= false;
	
	while (true) {
		while (norm(diff(coords[base], coords[base2]))< min_dist) {
			base2+= 1;
			if (base2== coords.length- 1) {
				done= true;
				result.push(coords[coords.length- 1]);
				break;
			}
		}
		if (done) {
			break;
		}
		tested= base2+ 1;
		
		while (cos_angle(coords[base2], coords[base], coords[tested])< -1.0+ treshold) {
			tested+= 1;
			if (tested== coords.length) {
				done= true;
				result.push(coords[coords.length- 1]);
				break;
			}
		}
		if (done) {
			break;
		}
		
		result.push(coords[tested- 1]);
		base= tested- 1;
		base2= tested;
		if (base2== coords.length- 1) {
			break;
		}
	}
	return result;
}
