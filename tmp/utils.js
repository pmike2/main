function deepcopy(data) {
	return JSON.parse(JSON.stringify(data));
}


export {deepcopy};