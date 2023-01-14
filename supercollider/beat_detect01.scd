(
//var root_loops= "/Volumes/Data/perso/son/loops";
var root_loops= "/Users/home/Desktop/pm/son/loops";

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

~fft_n_frames= 1024;
// FFT mono
~fft_buffers= Array.fill(~n_buffers, {Buffer.alloc(s, ~fft_n_frames, 1)});

~initTime = Array.fill(~n_buffers, {nil});
~checkpoints= Array.fill(~n_buffers, {[]});
)

~array_checkpoints.size.postln;

(
SynthDef(\beat_detect, {
	arg buffer, fft_buffer, id, duration;
	var in, chain, onsets, done;
	in= PlayBuf.ar(1, buffer, BufRateScale.kr(buffer), trigger:1, loop:0, doneAction:2);
	chain= FFT(fft_buffer, in);
	onsets= Onsets.kr(chain, 0.1, \power, relaxtime:1.0);
	//SendTrig.kr(onsets);
	SendReply.kr(onsets, '/onset', id);

	done = Done.kr(Line.kr(dur:duration));
	SendReply.kr(done, '/buf_is_done', id);
}).add;
)

(
OSCdef(\listen_onsets, {
	arg msg, time;
	var idx_buffer;
    //msg.postln;
	idx_buffer= msg[3].asInteger;
	//idx.postln;
	~initTime[idx_buffer] = ~initTime[idx_buffer] ?? { Main.elapsedTime };
	time= time - ~initTime[idx_buffer];
	//[idx, time].postln;
	~checkpoints[idx_buffer]= ~checkpoints[idx_buffer].add(time);
},'/onset', s.addr);


OSCdef(\listen_buf_is_done, {
	arg msg, time;
	var idx_buffer, onset_path;
    //msg.postln;
	idx_buffer= msg[3].asInteger;
	//idx_buffer.postln;
	onset_path= ~filenames[idx_buffer].pathOnly +/+ ~filenames[idx_buffer].fileNameWithoutExtension ++ "_onsets.aiff";

	~checkpoints[idx_buffer].do({
		arg cp;
		var idx;
		idx= (~buffers[idx_buffer].sampleRate* cp).round.asInteger;
		idx= if(idx< 0, 0, idx);
		//[idx_buffer, idx].postln;
		~array_checkpoints[idx_buffer].put(idx, 1.0);
	});

	f= SoundFile.openWrite(onset_path, "AIFF", "float", 1, ~buffers[idx_buffer].sampleRate);
	f.writeData(~array_checkpoints[idx_buffer]);
	f.close;

}, '/buf_is_done', s.addr);
)


(
~n_buffers.do({
	arg idx_buffer;
	Synth(\beat_detect, [\buffer, ~buffers[idx_buffer], \fft_buffer, ~fft_buffers[idx_buffer], \id, idx_buffer, \duration, ~buffers[idx_buffer].numFrames/ ~buffers[idx_buffer].sampleRate]);
});
)


(
OSCdef(\listen_onsets).free;
OSCdef(\listen_buf_is_done).free;
)
