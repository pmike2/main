(
var root_loops= "/Volumes/Data/perso/son/loops";
//var root_loops= "/Users/home/Desktop/pm/son/loops";

// liste des temps des transients détectés
~array_checkpoints= [];
// buffer des transients détectés
~buf_checkpoints= [];

// lire tous les samples d'un dossier
// buffers mono
~buffers= (root_loops ++ "/*").pathMatch.collect({
	arg loop;
	loop.postln;
	Buffer.readChannel(s, loop, channels:[0], action:{
		arg b;
		//b.numFrames.postln;
		~array_checkpoints.add( Array.fill(b.numFrames, 0.0) );
		~buf_checkpoints.add( Buffer.alloc(s, b.numFrames, 1) );
	});
});

~idx_buffer= 0;

~fft_n_frames= 1024;
// FFT mono
~fft_buffers= Buffer.alloc(s, ~fft_n_frames, 1);
)

(
SynthDef(\beat_detect, {
	var in, chain, onsets, done;
	in= PlayBuf.ar(1, ~buffers[~idx_buffer], BufRateScale.kr(~buffers[~idx_buffer]), trigger:1, loop:0, doneAction:2);
	chain= FFT(~fft_buffers, in);
	onsets= Onsets.kr(chain, 0.1, \power, relaxtime:1.0);
	//onsets= Onsets.kr(chain, 0.1);
	SendTrig.kr(onsets);

	done = Done.kr(Line.kr(dur:~buffers[~idx_buffer].numFrames/ ~buffers[~idx_buffer].sampleRate));
	SendReply.kr(done, '/buf_is_done');
}).add;
)

(
~initTime = nil;
~checkpoints= [ ];

OSCdef(\listen_buf_is_done, {
	arg msg, time;
    msg.postln;

	OSCdef(\listen_onsets).free;
	OSCdef(\listen_buf_is_done).free;

	~checkpoints.do({
		arg cp;
		var idx;
		idx= (s.sampleRate* cp).round.asInteger;
		idx= if(idx< 0, 0, idx);
		idx.postln;
		~array_checkpoints[~idx_buffer].put(idx, 1.0);
	});

	~buf_checkpoints[~idx_buffer].loadCollection(~array_checkpoints[~idx_buffer]);
	~buf_checkpoints[~idx_buffer].write("/Volumes/Data/perso/son/loops/test.aiff");

}, '/buf_is_done', s.addr);

OSCdef(\listen_onsets, {
	arg msg, time;
    msg.postln;
	~initTime = ~initTime ?? { Main.elapsedTime };
	time = time - ~initTime;
	time.postln;
	~checkpoints= ~checkpoints.add(time);
},'/tr', s.addr);

Synth(\beat_detect);
)

