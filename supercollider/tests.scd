"globals.scd".loadRelative();

(
var root_loops= "/Volumes/Data/perso/son/loops/amen";
var buffers, checkpoints;

#buffers, checkpoints= ~load_onsets.value(root_loops);
checkpoints.postln;
)


a= ~load_buffers.value("/Users/home/Desktop/pm/son/SnareDrums");
a.keysValuesDo { |key, value| [key, value].postln };

b= Synth(\tmp, [\freq, 200]);
b.free;

k= Synth(\kick);
k.set(\t_trig, 1);

h= Synth(\hihat);
h.set(\t_trig, 1);

i= Synth(\snare);
i.set(\t_trig, 1);
