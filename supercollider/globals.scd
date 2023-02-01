(
// fonctions -------------------------------------------------------------------------------
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


~load_buffers= {
	arg root_loops, verbose=false;

	var result= ();

	(root_loops ++ "/*.wav").pathMatch.collect({
		arg loop;
		var name;
		if(verbose, {loop.postln});

		name= PathName(loop).fileNameWithoutExtension;
		result[name]= Buffer.read(s, loop);
	});

	result;
};


// SynthDef -------------------------------------------------------------------------------
SynthDef(\beat_detect, {
	arg buffer, id, duration, treshold, odftype, relaxtime;
	var in, chain, onsets, done;

	SendReply.ar(Impulse.ar(0), '/buf_start', id);

	in= PlayBuf.ar(1, buffer, BufRateScale.kr(buffer), trigger:1, loop:0, doneAction:2);
	chain= FFT(LocalBuf(512, 1), in);
	onsets= Onsets.kr(chain, treshold, odftype, relaxtime:relaxtime);
	SendReply.kr(onsets, '/onset', id);

	done = Done.kr(Line.kr(dur:duration));
	SendReply.kr(done, '/buf_is_done', id);
}).add;

SynthDef(\beat_player, {
	arg out= 0, buffer= 0, trig=1, start= 0, duration= 0.1, amp= 0.5;
    Out.ar(
		out,
		PlayBuf.ar(
			2, buffer,
			rate: BufRateScale.kr(buffer),
			trigger: trig,
			startPos: start* BufSampleRate.kr(buffer),
			loop: 0,
			doneAction: 0
		)* EnvGen.kr(Env([0, 1, 1, 0], [0.001, duration, 0.001], \lin), 1, doneAction: 2, levelScale: amp)
    );
}).add;

SynthDef(\tick, {
	arg out= 0, amp= 0.4, duration=0.02, freq=800;
	Out.ar(out, Blip.ar(freq)* EnvGen.kr(Env([0, 1, 1, 0], [0.001, duration, 0.001], \lin), 1, doneAction: 2, levelScale: amp));
}).add;

SynthDef(\gran_player, {
	arg out=0, amp=0.6, buf, phase_rate, trigger_rate, dur, pos;
	Out.ar(out, GrainBuf.ar(
		numChannels:2,
		trigger:Impulse.kr(trigger_rate),
		dur:dur,
		sndbuf:buf,
		rate:1.0,
		pos:pos,
		interp:4,
		pan:0,
		envbufnum:-1,
		maxGrains:512,
		mul:amp));
}).add;

// Arguments that begin with "t_" (e.g. t_trig), or that are specified as \tr in the def's rates argument, will be made as a TrigControl
SynthDef(\kick, {
	arg t_trig= 0, out= 0, freq_env_amount=0.5, freq_env_dur=0.03, freq_min=20, freq_max=200, amp_dur=1.2, amp=0.7;
	var amp_env, freq_env;
	amp_env= Env.asr(0.001, amp, amp_dur, -4);
	freq_env= Env.asr(0.001, freq_env_amount, freq_env_dur, -4);
	//amp_env.plot;
	Out.ar(out, Pan2.ar(EnvGen.ar(amp_env, t_trig)* SinOsc.ar(EnvGen.ar(freq_env, t_trig).exprange(freq_min, freq_max))));
}).add;

SynthDef(\hihat, {
	arg t_trig= 0, out= 0, freq1= 1000, freq2= 9000, feedback=12.0, amp_dur=0.4, amp=0.2;
	var amp_env;
	amp_env= Env.asr(0.001, amp, amp_dur, -10);
	Out.ar(out, EnvGen.ar(amp_env, t_trig)* SinOscFB.ar([freq1, freq2], feedback));
}).add;

SynthDef(\snare, {
	arg t_trig= 0, out= 0, amp_dur=0.1, amp_dur_2=0.8, amp=1.0, mix=0.7, room=5.0, damp=0.3;
	var amp_env, amp_env_2, dry, rev;
	amp_env= Env.asr(0.001, 1.0, amp_dur, -4);
	amp_env_2= Env.asr(0.001, amp, amp_dur_2, -4);
	dry= EnvGen.ar(amp_env, t_trig)* PinkNoise.ar();
	//rev = Normalizer.ar(FreeVerb2.ar(dry, dry, mix, room, damp), dur:0.03);
	rev = FreeVerb2.ar(dry, dry, mix, room, damp);
	Out.ar(out, EnvGen.ar(amp_env_2, t_trig)* Compander.ar(rev, rev));
}).add;

SynthDef.new(\freeverb, {
	arg in, out=0, room=0.5, mix=0.5;
	var sig = In.ar(in, 2);
	var rev = FreeVerb2.ar(sig[0], sig[1], mix: mix, room: room);
	Out.ar(out, rev);
}).add;

SynthDef(\allpass_delay, {
	arg in, out=0, delaytime=0.1;
	var sig = In.ar(in, 2);
	sig = Mix.ar([sig, AllpassC.ar(sig, 0.5, delaytime, 0.01)]);
	Out.ar(out, sig);
}).add;

SynthDef(\simple_delay, {
	arg in, out=0, delaytime=0.5, dry=0.5, wet=0.5;
	var sig = In.ar(in, 2);
	var fx= DelayC.ar(sig, 2.0, delaytime);
	Out.ar(out, Pan2.ar(Mix([sig* dry, fx* wet])));
}).add;

SynthDef(\compressor, {
	arg in, ctrl=0, out=0, tresh=0.5, slope_below=2.0, slope_above=0.4, clamp_time=0.05, relax_time=0.1, mul=1.0;
	var sig= In.ar(in, 2);
	// par défaut pas de sidechain, ctrl == in
	var sig_ctrl= if(ctrl == 0, {sig}, {In.ar(ctrl, 2)});
	var fx= Compander.ar(sig, sig, tresh, slope_below, slope_above, clamp_time, relax_time, mul);
	Out.ar(out, Pan2.ar(fx));
}).add;
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
