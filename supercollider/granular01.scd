/* un grain
FSinOsc est une sinusoide CPU efficient
OffsetOut permet de conserver l'offset au sein d'un bloc de controle
\ir rend non-modifiable les arguments ce qui plus CPU efficient
*/
(
SynthDef(\gabor, {
	arg out=0, freq=400, duration=1, pan=0, amp=0.1, width=0.25;
	var env= LFGauss.ar(duration, width, loop:0, doneAction:2);
	var sound= FSinOsc.ar(freq, 0.5pi, mul:env);
	OffsetOut.ar(out, Pan2.ar(sound, pan, amp));
}, \ir ! 6).add;
)

// visus
{LFGauss.ar(1, 0.25, loop:0, doneAction:2)}.plot(1.0);
{FSinOsc.ar(400, 0.5pi, mul:LFGauss.ar(1, 0.25, loop:0, doneAction:2))}.plot(1.0);

/*
on définit le pattern \grain
\dur dans le contexte PBind est le temps à attendre avant le prochain événement
*/
(
Pbindef(\grain,
	\instrument, \gabor,
	\freq, 1000,
	\dur, 0.2,
	\duration, 20/1000,
	\amp, 0.2
).play;
)

/*
modifs du pattern \grain
*/
Pbindef(\grain, \duration, 10/ Pkey(\freq));
Pbindef(\grain, \duration, 5/ Pkey(\freq));
Pbindef(\grain, \duration, 3/ Pkey(\freq));
Pbindef(\grain, \duration, 2/ Pkey(\freq));
Pbindef(\grain, \duration, 1/ Pkey(\freq));
Pbindef(\grain, \duration, Pseq((10 .. 1), inf)/ Pkey(\freq));

Pbindef(\grain, \dur, 0.1);
Pbindef(\grain, \dur, 0.05);
Pbindef(\grain, \dur, 0.03);

/*
on repète le burst de grains avec 1sec d'attente entre chaque
*/
(
Tdef(\grain_task, {
	loop {
		Pbindef(\grain, \duration, Pseq((10 .. 1), 1)/ Pkey(\freq)).play;
		1.wait;
	}
}).play;
)
