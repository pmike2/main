"globals.scd".loadRelative();

(
var root_loops= "/Volumes/Data/perso/son/loops/amen";
var buffers, checkpoints;

#buffers, checkpoints= ~load_onsets.value(root_loops);
checkpoints.postln;
)
