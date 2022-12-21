(
// lire tous les samples d'un dossier
//var root_loops= "/Volumes/Data/perso/son/loops";
var root_loops= "/Users/home/Desktop/pm/son/loops";

~buffers= (root_loops ++ "/*").pathMatch.collect({
	arg loop;
	loop.postln;
	Buffer.read(s, loop);
});
~idx_buffer= 0;

~fft_n_frames= 1024;
// FFT stereo
~fft_buffers= Buffer.alloc(s, ~fft_n_frames, 1) ! 2;

)

// on vérifie qu'il y a bien qqchose dans ~buffers
Post << ~buffers[~idx_buffer];
~buffers[~idx_buffer].plot;

(
SynthDef("buf_play", {
	var pl;
	pl= PlayBuf.ar(2, ~buffers[~idx_buffer], BufRateScale.kr(~buffers[~idx_buffer]), trigger:1, loop:0);
	Out.ar(0, pl);
}).add;

// detection de beat avec PV_JensenAndersen; voir aussi PV_HainsworthFoote
SynthDef("beat_detect", {
	var in, chain, detect, env;
	in= PlayBuf.ar(2, ~buffers[~idx_buffer], BufRateScale.kr(~buffers[~idx_buffer]), trigger:1, loop:1);
	chain= FFT(~fft_buffers, in);
	chain= chain[0];
	detect= PV_JensenAndersen.ar(chain, threshold:0.1);
	//Out.ar(0, in* Decay.ar(0.1 * detect, 0.1));
	env= Env([1.0, 0.0], [0.1], \step, nil, nil, 0);
	Out.ar(0, in* EnvGen.ar(env, detect, doneAction:0));
}).add;
)


x= Synth("buf_play");
x.free;
y= Synth("beat_detect");
y.free;

plot({ [Decay.ar(Impulse.ar(1), 0.01), Impulse.ar(1)] });
