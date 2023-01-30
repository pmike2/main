(
~load_onsets= {
	arg root_loops, verbose=false;

	var buffers, checkpoints;

	checkpoints= [];
	buffers= (root_loops ++ "/*.wav").pathMatch.collect({
		arg loop;
		var onset_txt, arr;
		if(verbose, {loop.postln});

		onset_txt= PathName(loop).pathOnly +/+ PathName(loop).fileNameWithoutExtension ++ "_onsets.txt";
		arr= File.readAllString(onset_txt).stripWhiteSpace.split($\n);
		checkpoints= checkpoints.add(arr.collect({arg x; x.asFloat}));

		Buffer.read(s, loop);
	});

	[buffers, checkpoints];
};
)


/*
// Utilisation
"globals.scd".loadRelative();
(
var root_loops= "/Volumes/Data/perso/son/loops/amen";
var buffers, checkpoints;
#buffers, checkpoints= ~load_onsets.value(root_loops);
)
*/
