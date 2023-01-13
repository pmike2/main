(
// lire tous les samples d'un dossier
//var root_loops= "/Volumes/Data/perso/son/loops";
var root_loops= "/Users/home/Desktop/pm/son/loops";

~transient_buffers= [ ];

~buffers= (root_loops ++ "/*").pathMatch.collect({
	arg loop;
	loop.postln;
	Buffer.read(s, loop, action:{
		arg b;
		b.numFrames.postln;
		~transient_buffers= ~transient_buffers.add(Buffer.alloc(s, b.numFrames* s.sampleRate/ b.sampleRate, 1));
	});
});

~idx_buffer= 0;

~fft_n_frames= 1024;
// FFT stereo
~fft_buffers= Buffer.alloc(s, ~fft_n_frames, 1) ! 2;
)

// on vérifie qu'il y a bien qqchose
(
Post << ~buffers;
~buffers[~idx_buffer].plot;
Post << ~transient_buffers;
~transient_buffers[~idx_buffer].plot;
Post << ~fft_buffers;
~fft_buffers[0].plot;
)

(
/*SynthDef(\buf_play, {
	var pl;
	pl= PlayBuf.ar(2, ~buffers[~idx_buffer], BufRateScale.kr(~buffers[~idx_buffer]), trigger:1, loop:0);
	Out.ar(0, pl);
}).add;
*/

// detection de beat avec PV_JensenAndersen; voir aussi PV_HainsworthFoote
SynthDef(\beat_detect, {
	//arg out;
	arg bufnum;
	var in, chain, detect, env, rec, onsets ;
	in= PlayBuf.ar(2, ~buffers[~idx_buffer], BufRateScale.kr(~buffers[~idx_buffer]), trigger:1, loop:0, doneAction:0);
	chain= FFT(~fft_buffers, in);
	chain= chain[0];
	//detect= PV_JensenAndersen.ar(chain, threshold:0.5);
	onsets= Onsets.kr(chain);

	//Out.ar(out, in* Decay.ar(0.1 * detect, 0.1));
	//env= Env([0.0, 1.0, 0.0], [0.001, 0.1], \lin, nil, nil, 0);
	//Out.ar(out, in* EnvGen.ar(env, detect, doneAction:0));
	//Out.ar(out, SinOsc.ar(440, 0, Decay.ar(0.5 * detect, 0.1)));
	//Impulse.ar(0)
	//Out.ar(out, detect);
	//Out.ar(out, SinOsc.ar(440));
	//rec= SinOsc.ar(440, 0, Decay.ar(0.5 * detect, 0.1));
	//rec= SinOsc.ar(440);
	//rec= detect;
	//rec= Line.ar(0.0, 1.0, 1.5, doneAction:2);
	//rec= in[0];
	//rec= chain;
	//RecordBuf.ar(rec, bufnum, recLevel:1.0, preLevel:0.0, loop:0, doneAction:0);
	SendTrig.kr(onsets);
}).add;

SynthDef(\rec_buf, {
	arg input, bufnum;
	RecordBuf.ar(In.ar(input), bufnum, recLevel:1.0, preLevel:0.0, loop:0, doneAction:2);
	//RecordBuf.ar(SinOsc.ar(400), bufnum, recLevel:1.0, preLevel:0.0, loop:0, doneAction:2);
}).add;
)

(
o= OSCFunc({
	arg msg, time;
    [time, msg].postln;
},"/tr", s.addr);
)

(
~bus= Bus.audio(s, 1);
~buf= Buffer.alloc(s, s.sampleRate* 5, 1);
)

//y= Synth(\beat_detect, [\out, ~bus]);
y= Synth(\beat_detect, [\bufnum, ~transient_buffers[~idx_buffer]]);
y.free;

(
~transient_buffers[~idx_buffer].plot;
~transient_buffers[~idx_buffer].write("/Users/home/Desktop/pm/son/loops/test.aiff");
)


~bus.scope;
~bus.numChannels;

z= Synth.tail(s, \rec_buf, [\input, ~bus, \bufnum, ~buf]);
z.free;

~buf.plot;
~buf.numChannels;
//~buf.write("/Volumes/Data/perso/son/loops/test.aiff");
~buf.write("/Users/home/Desktop/pm/son/loops/test.aiff");

(
~buf.close;
~buf.free;
)
