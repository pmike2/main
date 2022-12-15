s.reboot;

// reinit ----------------------------------------

s.freeAll;
currentEnvironment.clear;


// print nodes -------------------------------------
s.queryAllNodes;


// global vars --------------------------------------
(
~n_configs= 6;
~buf1= Buffer.read(s, "/Volumes/Data/perso/son/loops/2022_10_10_001.wav",
	//action: { arg buffer; ~buf1_n_frames= ~buf1.numFrames}
);
~buf2= Buffer.read(s, "/Volumes/Data/perso/son/loops/2022_11_25_bis_001.wav",
	//action: { arg buffer; ~buf2_n_frames= ~buf1.numFrames}
);
~buf_array= [~buf1, ~buf2];
~bus_delay= Bus.audio(s, 2);
~bus_ctrl= Bus.control(s, 1);
)

// a faire quand les buffers sont chargés ...
(
~config= Array.fill(~n_configs, {[~buf_array.size.rand, ~buf1.numFrames.rand, 0.1+ 1.0.rand, 0.1+ 0.5.rand]});
)


// synthdefs -----------------------------------------
(
SynthDef("buf_player", {
	arg out= 0, bufidx= 0, trig= 1, start= 0, release= 0.1;
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
		)* EnvGen.kr(Env.perc(0.0, release), trig, doneAction: Done.freeSelf)
    );
}).add;

SynthDef.new("delay", {
	arg in, out=0, delaytime= 0.1;
	var sig = In.ar(in, 2);
	sig = Mix.ar([sig, AllpassC.ar(sig, 0.5, delaytime.poll, 0.01)]);
	Out.ar(out, sig);
}).add;

SynthDef("rand_delaytime", {
	arg out, trig= 1;
	var drand, rand_float;
	drand= Dxrand([0.01, 0.05, 0.1], inf);
	rand_float= Demand.kr(trig, 0, drand);
	Out.kr(out, rand_float);
}).add;
)


// synths et task ------------------------------------
(
var delay_synth;
delay_synth= Synth("delay", ["in", ~bus_delay]);
delay_synth.map("delaytime", ~bus_ctrl);

~seq_task= Task({
    loop {
        ~config.do({
			arg cfg;
			cfg.postln;
			Synth("rand_delaytime", ["out", ~bus_ctrl]);
			Synth("buf_player", ["bufidx", cfg[0], "start", cfg[1], "release", cfg[2], "out", ~bus_delay]);
			cfg[3].wait;
        });
    }
});

~seq_task.play;
)

~seq_task.stop;

