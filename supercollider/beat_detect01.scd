~verbose= false;
"globals.scd".loadRelative();

(
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

