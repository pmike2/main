~verbose= false;

(
SynthDef(\beat_player, {
	arg out= 0, buffer= 0, start= 0, duration= 0.1, amp= 0.5;
    Out.ar(
		out,
		PlayBuf.ar(
			2, buffer,
			rate: BufRateScale.kr(buffer),
			trigger: 1,
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
)

(
Routine.run({
	var root_loops= "/Volumes/Data/perso/son/loops/amen";
	//var root_loops= "/Users/home/Desktop/pm/son/loops/test_onset";

	~checkpoints= [];

	~buffers= (root_loops ++ "/*.wav").pathMatch.collect({
		arg loop;
		var onset_txt, arr;
		if(~verbose, {loop.postln});

		onset_txt= PathName(loop).pathOnly +/+ PathName(loop).fileNameWithoutExtension ++ "_onsets.txt";
		arr= File.readAllString(onset_txt).stripWhiteSpace.split($\n);
		//arr.postln;
		~checkpoints= ~checkpoints.add(arr.collect({arg x; x.asFloat}));

		Buffer.read(s, loop);
	});

	~n_buffers= ~buffers.size;
})
)

MIDIIn.connectAll;
(
var main_seconds= Main.elapsedTime.ceil;

~midi_noteon.free;
~midi_noteoff.free;
~midi_cc.free;
~midi_program.free;

~cc_idx_buffer= 0;
~cc_duration= 0.1;

~tempo= 150.0;

~clock= TempoClock(~tempo/ 60, seconds:main_seconds, queueSize:1024);
~clock_tick= TempoClock(~tempo/ 60, seconds:main_seconds);

~loop_nbeats= 4;
~quant= 0.5;

~clock_tick.play({
	arg beats, time, clock;
	//[beats, time, clock].postln;
	Synth(\tick, [\out, 0, \amp, 0.2]);
	1;
}, quant:1);

~clock_tick.playNextBar({
	arg beats, time, clock;
	//[beats, time, clock].postln;
	Synth(\tick, [\out, 0, \amp, 0.6]);
	~clock_tick.beatsPerBar;
});

~midi_noteon= MIDIFunc.noteOn({
	arg ...args;
	var idx_buffer, idx_start, duration, amp, quant_t;

	//args.postln;

	idx_buffer= ~cc_idx_buffer;
	idx_start= (args[1]- 36) % ~checkpoints[idx_buffer].size;
	duration= ~cc_duration;
	// alternatives mapping : https://doc.sccode.org/Classes/SimpleNumber.html
	amp= args[0].linlin(0, 127, 0.0, 1.0);

	if(~verbose, {[idx_buffer, idx_start, duration, amp].postln;});

	Synth(\beat_player, [
		\buffer, ~buffers[idx_buffer],
		\start, ~checkpoints[idx_buffer][idx_start],
		\duration, duration,
		\amp, amp,
		\out, 0
	]);

	//quant_t= ~clock.timeToNextBeat(quant:~quant)+ (~loop_nbeats* ~clock.beatDur);
	//now= ~clock.secs2beats(~clock.seconds);
	quant_t= ~clock.beatInBar.round;
	quant_t= if(quant_t>= ~clock.beatsPerBar, {0.0}, {quant_t});
	quant_t= quant_t+ ~loop_nbeats;
	quant_t.postln;

	~clock.sched(quant_t, {
		Synth(\beat_player, [
			\buffer, ~buffers[idx_buffer],
			\start, ~checkpoints[idx_buffer][idx_start],
			\duration, duration,
			\amp, amp,
			\out, 0
		]);
		~loop_nbeats; // delta in beats
	});
});

//~midi_noteoff= MIDIFunc.noteOff({arg ...args; args.postln});

~midi_cc= MIDIFunc.cc({
	arg ...args;
	var idx_cc;
	//args.postln;

	idx_cc= args[1];

	case
	{idx_cc == 70} {~cc_idx_buffer= args[0].linlin(0, 127, 0, ~n_buffers- 1);}
	{idx_cc == 71} {~cc_duration= args[0].linlin(0, 127, 0.005, 0.7);}
	;
});

~midi_program= MIDIFunc.program({arg ...args; args.postln});
)

(
~midi_noteon.free;
~midi_noteoff.free;
~midi_cc.free;
~midi_program.free;
)

~clock.tempo= 4;
~clock.clear;
~clock.stop;

~clock_tick.tempo= 4;
~clock_tick.stop;

