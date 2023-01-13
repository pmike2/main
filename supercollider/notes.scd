/*
a - z  : variables globales
sinon faire débuter nom variable par ~ pour avoir du global

cmd enter -> execute bloc
shift enter -> execute line or selection
cmd b -> boot ; s vaut server localhost
cmd . -> stop sound (free all synths)
cmd shift p -> clear log
cmd d sur un mot-clef -> help

Les extensions sont ici : /Volumes/Vol-Users/Users/pmbeau2/Library/Application Support/SuperCollider/Extensions/SC3plugins
telechargées ici : https://github.com/supercollider/sc3-plugins
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

// concaténation de strings
"aa" ++ "bb";

// -----------------------------------------
// concaténation de 2 Arrays
[1, 2, 3] ++ [7, 8];

// map Array
[1, 2, 3].collect({arg x, i; x* 2});

// choix aléatoire
[1, 2, 3].choose;

// taille Array
Array.fill(4, {arg i; i}).postln.size.postln;

// plot Array
Array.fill(4, {arg i; i}).plot;

// ajout d'un element
(
a= [1,2,3];
a= a.add(4);
a.postln;
)

// -----------------------------------------

// scope de tout le son du server
s.scope();

// redémarre serveur
s.reboot;

// affiche tous les noeuds
s.queryAllNodes;

// permet d'attendre que tous les SynthDef soient faits avant de les instancier avec des Synth
s.sync;

// libère un synth
x.free;

// reinit environment variables
currentEnvironment.clear;

// alternative à postln : Post
Post << "/Volumes/Data/perso/son/loops/*".pathMatch;

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

// permet d'ajouter un comportement a cmd + .
CmdPeriod.doOnce({"cmd + . pressed".postln;});

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


// choisir un fichier, écrire un fichier ---------------------------------
Dialog.openPanel({arg x; x.postln;});
Dialog.savePanel({arg x; x.postln;});

// remplir un buffer avec des valeurs
(
a= Buffer.alloc(s, 128, 1, completionMessage: {
	arg buf;
	buf.setnMsg(0, Array.fill(buf.numFrames, { 1.0.rand }))
});
)
a.plot;

// on peut ajouter un tableau à une fonction; le résultat est une fonction qu'on peut évaluer avec .value
(
x= [1, 2, 3];
y= x+ {1.0.rand};
10.do({y.value.postln;});
)

// les arguments qui commencent par a_ peuvent lire un bus à audio rate
(
SynthDef(\test_modulated, {
	arg a_fm= 0.0;
	Out.ar(0, SinOsc.ar(400.0+ (a_fm* 100.0), mul:0.2));
}).add;
SynthDef(\test_modulator, {
	arg out, freq= 100.0, amp=0.5;
	Out.ar(out, SinOsc.ar(freq, mul:amp));
}).add;
)
(
~audio_bus= Bus.audio(s, 1);
~audio_bus.scope();
~modulator= Synth(\test_modulator, [\out, ~audio_bus]);
~test_audio_rate= Synth(\test_modulated);
~test_audio_rate.map(\a_fm, ~audio_bus);
~modulator.set(\freq, 100.0);
~modulator.set(\amp, 1.8);
)


// \ir dans SynthDef rend constant certains arguments ; ici \ir ! 3 == [\ir, \ir, \ir] donc tous les args.
// -> code plus performant en terme de CPU
(
SynthDef(\const_args, {
	arg x, y, z;
	Out.ar(0, SinOsc(x* y* z, mul:0.3));
}, \ir ! 3).add;
)
