s.reboot;

// si le sample est stereo il faut utiliser readChannel et pas read, afin de spécifier un seul canal
//b= Buffer.readChannel(s, "/Users/home/Desktop/pm/son/loops/loop01.wav", channels: [0]);
b= Buffer.readChannel(s, "/Volumes/Data/perso/son/loops/2022_11_25_bis_001.wav", channels: [0]);

b.plot;

(
SynthDef(\granbuf, {
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
)

x= Synth(\granbuf, [\buf, b, \phaseRate, 1.0, \trigger_rate, 1.0, \dur, 1.0, \pos, 0.0]);
x.set(\phaseRate, 0.1);
x.set(\trigger_rate, 10.0);
x.set(\dur, 0.1);
x.free;

~clock= TempoClock(1);
(
~task= Task({
	var stream= Pseq([0.1, 0.2, 0.7], inf).asStream;
	var pos;
	loop({
		pos= stream.next;
		x.set(\pos, pos);
		0.2.wait;
	});
}, ~clock);
)
~task.play();
~task.stop;
