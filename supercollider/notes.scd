/*
a - z  : variables globales
sinon faire débuter nom variable par ~ pour avoir du global

cmd enter -> execute bloc
shift enter -> execute line or selection
cmd b -> boot ; s vaut server localhost
cmd . -> stop sound (free all synths)
cmd shift p -> clear log
cmd d sur un mot-clef -> help
*/

// evaluate function
f= {arg x=0; x* 2};
f.value; // ou f.value() ou value(f);
f.value(3); // si arguments

// # permet d'affecter une liste de valeurs
(
var x1, x2, x3;
#x1, x2, x3= Array.rand(3, 0.0, 1.0);
[x1, x2, x3].do({arg x; x.postln;});
)

// concaténation de 2 Arrays
[1, 2, 3] ++ [7, 8];

// map Array
[1, 2, 3].collect({arg x, i; x* 2});

// taille Array
Array.fill(4, {arg i; i}).postln.size.postln;

// plot Array
Array.fill(4, {arg i; i}).plot;

// scope de tout le son du server
s.scope();

// redémarre serveur
s.reboot;

// affiche tous les noeuds
s.queryAllNodes;

// libère un synth
x.free;

// reinit environment variables
currentEnvironment.clear;

// nombre de frames du buffer, mais .ir -> ce UGen ne calcule cette valeur qu'une fois
BufFrames.ir(buf)

// remplacent mul et add
SinOsc().range(2.0, 12.0)
SinOsc().exprange(2.0, 12.0)

// 3 sines avec différentes amplitudes sommées avec .sum
// Peak calcule la valeur max d'un signal, échantillonné par Impulse et poll l'affiche dans la console
(
SynthDef("test_peak", {
	arg out= 0;
	var sound;
	sound= SinOsc.ar([100, 200, 500], mul:[0.7, 0.5, 0.4]).sum;
	Peak.ar(sound, Impulse.kr(1)).poll(1);
	Out.ar(out, sound);
}).add;
)
t= Synth("test_peak");
t.free;

// serveur vers client -------------------------------------------------------
// On receiving a trigger, sends a trigger message from the server back to the client.
// "tr" est le message lié au Trig
// 0.9 est la valeur envoyée
(
SynthDef("test_trig", {
	arg id;
	SendTrig.kr(Dust.kr(1.0), id, 0.9);
}).add;

// dis que faire lorsque l'on recoit le message "/tr"
o= OSCFunc({
	arg msg, time;
    [time, msg].postln;
},"/tr", s.addr);

// création d'un id unique
~trig_id= UniqueID.next;
Synth("test_trig", ["id", ~trig_id]);
)
o.free;

// permet d'ajouter un comportement a cmd + .
CmdPeriod.doOnce({o.remove; "removed the responder".postln;});

// Trig -> trig to gate; ignore les trigs si gate high
{ Trig.ar(Dust.ar(4), 1.0) }.play;

// groupes ------------------------------------------------------------------
(
g= Group.new;
h = Group.after(g);

SynthDef("tmp", {
	arg freq;
	Out.ar(0, SinOsc.ar(freq, mul:0.2));
}).add;

Array.fill(4, {Synth("tmp", ["freq", rand(200, 1000)], g, \addToHead)});
)
// tous les UGen du groupe auront freq = 2000 s'ils ont un arg freq
g.set("freq", 2000);
g.free; h.free;
