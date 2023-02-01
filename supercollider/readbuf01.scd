// reinit ----------------------------------------
(
//s.freeAll;
CmdPeriod.run; // plus complet que s.freeAll, car arrête aussi toutes les routines et autres threads
currentEnvironment.clear;
)

"globals.scd".loadRelative();

// global vars------------------------------------
(
~buf1= Buffer.read(s, "/Volumes/Data/perso/son/loops/2022_10_10_001.wav");
~buf2= Buffer.read(s, "/Volumes/Data/perso/son/loops/2022_11_25_bis_001.wav");
~buf_array= [~buf1, ~buf2];
~bus_ctrl= Bus.control(s, 1);
~bus_ctrl2= Bus.control(s, 1);
~bus_ctrl3= Bus.control(s, 1);
~bus_ctrl4= Bus.control(s, 1);
~bus_reverb= Bus.audio(s, 2);
)

// SynthDefs -------------------------------------
(
SynthDef("buf_player", {
	arg out= 0, bufidx= 0, trig= 0, start= 0, release= 0.1;
	var bufnum= Select.kr(bufidx, ~buf_array);
    Out.ar(
		out,
		PlayBuf.ar(
			2, bufnum,
			rate: BufRateScale.kr(bufnum),
			trigger: trig,
			startPos: start,
			loop: 0,
			doneAction: 0
		)* Linen.kr(trig, 0.0, 1.0, release, doneAction: Done.none)
    );
}).add;

SynthDef("imp", {
	arg out, freq;
	Out.kr(out, Impulse.kr(freq));
}).add;

SynthDef("rand_frame", {
	arg out, trig= 0;
	var drand, rand_float;
	drand= Dxrand([0.1, 0.4, 0.8], inf);
	rand_float= Demand.kr(trig, 0, drand).poll(label:"frame");
	Out.kr(out, rand_float* ~buf1.numFrames);
}).add;

SynthDef("rand_release", {
	arg out, trig= 0;
	var drand, rand_float;
	drand= Dxrand([0.01, 0.08, 0.2], inf);
	rand_float= Demand.kr(trig, 0, drand).poll(label:"release");
	Out.kr(out, rand_float);
}).add;

SynthDef("rand_bufidx", {
	arg out, trig= 0;
	var drand, rand_int;
	drand= Dxrand([0, 1], inf);
	rand_int= Demand.kr(trig, 0, drand).poll(label:"bufidx");
	Out.kr(out, rand_int);
}).add;

)

// Synths -------------------------------------------
(
var rand_frame_synth, rand_release_synth, rand_bufidx_synth, buf_player_synth;

Synth("imp", ["out", ~bus_ctrl, "freq", 5.0]);

rand_frame_synth= Synth("rand_frame", ["out", ~bus_ctrl2]);
rand_frame_synth.map("trig", ~bus_ctrl);

rand_release_synth= Synth("rand_release", ["out", ~bus_ctrl3]);
rand_release_synth.map("trig", ~bus_ctrl);

rand_bufidx_synth= Synth("rand_bufidx", ["out", ~bus_ctrl4]);
rand_bufidx_synth.map("trig", ~bus_ctrl);

Synth("freeverb", ["in", ~bus_reverb]);

buf_player_synth = Synth("buf_player", ["out", ~bus_reverb]);
buf_player_synth.map("trig", ~bus_ctrl);
buf_player_synth.map("start", ~bus_ctrl2);
buf_player_synth.map("release", ~bus_ctrl3);
buf_player_synth.map("bufidx", ~bus_ctrl4);
)

