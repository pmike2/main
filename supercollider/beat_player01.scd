~verbose= false;
"globals.scd".loadRelative();

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
