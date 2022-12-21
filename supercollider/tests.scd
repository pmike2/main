(
SynthDef(\kick, {
	arg t_trig= 0, out= 0, freq_env_amount=0.5, freq_env_dur=0.1, freq_min=40, freq_max=200, amp_dur=2.0, amp=0.8;
	var amp_env, freq_env;
	amp_env= Env.asr(0.001, amp, amp_dur, -4);
	freq_env= Env.asr(0.001, freq_env_amount, freq_env_dur, -4);
	//amp_env.plot;
	Out.ar(out, Pan2.ar(EnvGen.ar(amp_env, t_trig)* SinOsc.ar(EnvGen.ar(freq_env, t_trig).exprange(freq_min, freq_max))));
}).add;

SynthDef(\hihat, {
	arg t_trig= 0, out= 0, freq1= 600, freq2= 610, feedback=3.0, amp_dur=0.1, amp=0.5;
	var amp_env;
	amp_env= Env.asr(0.001, amp, amp_dur, -10);
	Out.ar(out, EnvGen.ar(amp_env, t_trig)* SinOscFB.ar([freq1, freq2], feedback));
}).add;

SynthDef(\snare, {
	arg t_trig= 0, out= 0, amp_dur=0.1, amp_dur_2=0.1, amp=0.8, mix=0.4, room=0.5, damp=0.5;
	var amp_env, amp_env_2, dry, rev;
	amp_env= Env.asr(0.001, 1.0, amp_dur, -10);
	amp_env_2= Env.asr(0.001, amp, amp_dur_2, -10);
	dry= EnvGen.ar(amp_env, t_trig)* PinkNoise.ar();
	rev = FreeVerb2.ar(dry, dry, mix, room, damp);
	Out.ar(out, EnvGen.ar(amp_env_2, t_trig)* rev);
}).add;

SynthDef(\compressor, {
	arg in, out=0, tresh=0.5, slope_below=2.0, slope_above=0.4, clamp_time=0.05, relax_time=0.1, mul=1.0;
	var sig = In.ar(in, 2);
	var fx= Compander.ar(sig, sig, tresh, slope_below, slope_above, clamp_time, relax_time, mul);
	Out.ar(out, Pan2.ar(fx));
}).add;

SynthDef(\delay, {
	arg in, out=0, delaytime=0.5, dry=0.5, wet=0.5;
	var sig = In.ar(in, 2);
	var fx= DelayC.ar(sig, 2.0, delaytime);
	Out.ar(out, Pan2.ar(Mix([sig* dry, fx* wet])));
}).add;
)


(
~bus_fx= Bus.audio(s, 2);
~group_fx= Group.new;
~group_instrus= Group(~group_fx, \addBefore);
~compressor= Synth(\compressor, [\in, ~bus_fx], ~group_fx);
~delay= Synth.after(~compressor, \delay, [\in, ~bus_fx]);
//~delay= Synth.before(~compressor, \delay, [\in, ~bus_fx]);
~kick= Synth(\kick, [\out, ~bus_fx], ~group_instrus);
~hihat= Synth(\hihat, [\out, ~bus_fx], ~group_instrus);
~snare= Synth(\snare, [\out, ~bus_fx], ~group_instrus);

~clock= TempoClock(2);

~task= Task({
	var p_kick = Pseq([1, 0, 0, 1, 0, 0, Prand([0, 1], 1), 0], inf).asStream;
	var p_hihat= Pseq([0, 1, 1, 0, 1, 1, 0, 1], inf).asStream;
	var p_snare= Pseq([0, 0, 1, 0, 0, 1, 0, 0], inf).asStream;
	loop({
		~kick.set(\t_trig, p_kick.next);
		~hihat.set(\t_trig, p_hihat.next);
		~snare.set(\t_trig, p_snare.next);
		0.25.wait;
	});
}, ~clock);

~task.play;
)

~task.stop;

(
~kick.set(\freq_env_amount, 1.0);
~kick.set(\freq_env_dur, 0.05);
~kick.set(\freq_min, 20);
~kick.set(\freq_max, 200);
~kick.set(\amp_dur, 0.4);
~kick.set(\amp, 0.5);

~hihat.set(\freq1, 1000);
~hihat.set(\freq2, 9000);
~hihat.set(\feedback, 12.0);
~hihat.set(\amp_dur, 0.4);
~hihat.set(\amp, 0.03);

~snare.set(\mix, 0.7);
~snare.set(\room, 5.0);
~snare.set(\damp, 0.3);
~snare.set(\amp_dur, 0.1);
~snare.set(\amp_dur_2, 0.8);
~snare.set(\amp, 1.5);

~compressor.set(\tresh, 0.1);
~compressor.set(\slope_below, 1.0);
~compressor.set(\slope_above, 0.1);
~compressor.set(\clamp_time, 0.01);
~compressor.set(\relax_time, 0.01);
~compressor.set(\mul, 2.0);

~delay.set(\delaytime, 0.2);
~delay.set(\dry, 0.8);
~delay.set(\wet, 0.5);
)

