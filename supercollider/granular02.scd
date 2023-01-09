b= Buffer.read(s, "/Users/home/Desktop/pm/son/loops/loop01.wav");

(
SynthDef(\granbuf, {
	arg out=0, amp=0.6, buf;
	Out.ar(out, GrainBuf.ar(
		numChannels:2,
		trigger:Impulse.kr(10.0),
		dur:0.1,
		sndbuf:buf,
		rate:1.0,
		pos:LFNoise2.kr(0.1).range(0, 1),
		interp:4,
		pan:1,
		envbufnum:-1,
		maxGrains:512,
		mul:amp));
}).add;
)

x= Synth(\granbuf, [\buf, b]);
