~verbose= false;

(
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

OSCdef.freeAll;

OSCdef(\listen_onsets, {
	arg msg, time;
	var idx_buffer;
	idx_buffer= msg[3].asInteger;
	time= time - ~initTime[idx_buffer];
	if(~verbose, {[msg, idx_buffer, time].postln});
	time= if(time< 0, 0, time);
	~checkpoints[idx_buffer]= ~checkpoints[idx_buffer].add(time);
}, '/onset', s.addr);

OSCdef(\listen_buf_start, {
	arg msg, time;
	var idx_buffer;
	idx_buffer= msg[3].asInteger;
	if(~verbose, {[msg, idx_buffer].postln});
	~initTime[idx_buffer]= time;
}, '/buf_start', s.addr);

OSCdef(\listen_buf_is_done, {
	arg msg, time;
	var idx_buffer, onset_path_snd, onset_path_txt;
	idx_buffer= msg[3].asInteger;
	if(~verbose, {[msg, idx_buffer].postln});
	onset_path_snd= ~filenames[idx_buffer].pathOnly +/+ ~filenames[idx_buffer].fileNameWithoutExtension ++ "_onsets.aiff";
	onset_path_txt= ~filenames[idx_buffer].pathOnly +/+ ~filenames[idx_buffer].fileNameWithoutExtension ++ "_onsets.txt";

	~checkpoints[idx_buffer].do({
		arg cp;
		var idx;
		idx= (~buffers[idx_buffer].sampleRate* cp).round.asInteger;
		~array_checkpoints[idx_buffer].put(idx, 1.0);
	});

	f= SoundFile.openWrite(onset_path_snd, "AIFF", "float", 1, ~buffers[idx_buffer].sampleRate);
	f.writeData(~array_checkpoints[idx_buffer]);
	f.close;

	File.use(onset_path_txt, "w", {
		arg f;
		~checkpoints[idx_buffer].do({
			arg cp;
			f.write(cp.asString);
			f.write("\n");
		});
	});

}, '/buf_is_done', s.addr);
)


(
Routine.run({
	var root_loops= "/Volumes/Data/perso/son/loops/amen";
	//var root_loops= "/Users/home/Desktop/pm/son/loops/test_onset";

	// liste des temps des transients détectés
	~array_checkpoints= [];

	~filenames= [];

	// lire tous les samples d'un dossier
	// buffers mono
	~buffers= (root_loops ++ "/*.wav").pathMatch.collect({
		arg loop;
		if(~verbose, {loop.postln});
		~filenames= ~filenames.add(PathName(loop));
		Buffer.readChannel(s, loop, channels:[0], action:{
			arg b;
			~array_checkpoints= ~array_checkpoints.add( FloatArray.fill(b.numFrames, 0.0) );
		});
	});

	~n_buffers= ~buffers.size;
	~n_buffers;

	s.sync;

	~initTime = Array.fill(~n_buffers, {nil});
	~checkpoints= Array.fill(~n_buffers, {[ ]});

	s.sync();

	~n_buffers.do({
		arg idx_buffer;
		Synth(\beat_detect, [
			\buffer, ~buffers[idx_buffer],
			\id, idx_buffer,
			\duration, ~buffers[idx_buffer].numFrames/ ~buffers[idx_buffer].sampleRate,
			\treshold, 0.2,
			\odftype, \rcomplex,
			\relaxtime, 1.0
		]);
	});
});
)

// ------------------------------------------------------------------------------------------
(
SynthDef(\beat_player, {
	arg out= 0, buffer= 0, trig= 1, start= 0, duration= 0.1;
    Out.ar(
		out,
		PlayBuf.ar(
			2, buffer,
			rate: BufRateScale.kr(buffer),
			trigger: trig,
			startPos: start* BufSampleRate.kr(buffer),
			loop: 0,
			doneAction: 0
		)* EnvGen.kr(Env([0, 1, 1, 0], [0.01, duration, 0.01], \lin), trig, doneAction: 2)
    );
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


(
~seq_task= Task({
	var idx_buffer, idx_start, duration;
    loop {
		idx_buffer= ~n_buffers.rand;
		idx_start= ~checkpoints[idx_buffer].size.rand;
		duration= if(idx_start== (~checkpoints[idx_buffer].size- 1), {(~buffers[idx_buffer].numFrames/ ~buffers[idx_buffer].sampleRate)- ~checkpoints[idx_buffer][idx_start]}, {~checkpoints[idx_buffer][idx_start+ 1]- ~checkpoints[idx_buffer][idx_start]});
		[idx_buffer, idx_start, ~checkpoints[idx_buffer][idx_start], duration].postln;
		Synth(\beat_player, [
			\buffer, ~buffers[idx_buffer],
			\start, ~checkpoints[idx_buffer][idx_start],
			\duration, duration,
			\out, 0
		]);
		duration.wait;
    }
});
~seq_task.play;
)

~seq_task.stop;
