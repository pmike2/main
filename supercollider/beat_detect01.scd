(
~load_loops= {
	arg root_loops;
//var root_loops= "/Volumes/Data/perso/son/loops";
//var root_loops= "/Users/home/Desktop/pm/son/loops/test_onset";

// liste des temps des transients détectés
~array_checkpoints= [];

~filenames= [];

// lire tous les samples d'un dossier
// buffers mono
~buffers= (root_loops ++ "/*.wav").pathMatch.collect({
	arg loop;
	loop.postln;
	~filenames= ~filenames.add(PathName(loop));
	Buffer.readChannel(s, loop, channels:[0], action:{
		arg b;
		~array_checkpoints= ~array_checkpoints.add( FloatArray.fill(b.numFrames, 0.0) );
	});
});

~n_buffers= ~buffers.size;
~n_buffers.postln;

//~fft_n_frames= 1024;
// FFT mono
//~fft_buffers= Array.fill(~n_buffers, {Buffer.alloc(s, ~fft_n_frames, 1)});
}
)

~load_loops.value("/Users/home/Desktop/pm/son/loops/test_onset");


(
SynthDef(\beat_detect, {
	arg buffer, fft_buffer, id, duration, treshold, odftype, relaxtime;
	var in, chain, onsets, done;
	in= PlayBuf.ar(1, buffer, BufRateScale.kr(buffer), trigger:1, loop:0, doneAction:2);
	//chain= FFT(fft_buffer, in);
	chain= FFT(LocalBuf(1024, 1));
	onsets= Onsets.kr(chain, treshold, odftype, relaxtime:relaxtime);
	SendReply.kr(onsets, '/onset', id);

	done = Done.kr(Line.kr(dur:duration));
	SendReply.kr(done, '/buf_is_done', id);
}).add;
)

(
OSCdef(\listen_onsets, {
	arg msg, time;
	var idx_buffer;
	idx_buffer= msg[3].asInteger;
	~initTime[idx_buffer] = ~initTime[idx_buffer] ?? { Main.elapsedTime };
	time= time - ~initTime[idx_buffer];
	[msg, idx_buffer, time].postln;
	time= if(time< 0, 0, time);
	~checkpoints[idx_buffer]= ~checkpoints[idx_buffer].add(time);
},'/onset', s.addr);


OSCdef(\listen_buf_is_done, {
	arg msg, time;
	var idx_buffer, onset_path_snd, onset_path_txt;
    //msg.postln;
	idx_buffer= msg[3].asInteger;
	//idx_buffer.postln;
	onset_path_snd= ~filenames[idx_buffer].pathOnly +/+ ~filenames[idx_buffer].fileNameWithoutExtension ++ "_onsets.aiff";
	onset_path_txt= ~filenames[idx_buffer].pathOnly +/+ ~filenames[idx_buffer].fileNameWithoutExtension ++ "_onsets.txt";

	~checkpoints[idx_buffer].do({
		arg cp;
		var idx;
		idx= (~buffers[idx_buffer].sampleRate* cp).round.asInteger;
		//[idx_buffer, idx].postln;
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
~initTime = Array.fill(~n_buffers, {nil});
~checkpoints= Array.fill(~n_buffers, {[ ]});

~n_buffers.do({
	arg idx_buffer;
	Synth(\beat_detect, [\buffer, ~buffers[idx_buffer], \fft_buffer, ~fft_buffers[idx_buffer], \id, idx_buffer, \duration, ~buffers[idx_buffer].numFrames/ ~buffers[idx_buffer].sampleRate, \treshold, 0.5, \odftype, \rcomplex, \relaxtime, 1.0]);
});
)


(
OSCdef(\listen_onsets).free;
OSCdef(\listen_buf_is_done).free;
)
