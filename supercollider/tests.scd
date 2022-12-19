
(
// lire tous les samples d'un dossier
var root_loops= "/Volumes/Data/perso/son/loops";
~buffers= (root_loops ++ "/*").pathMatch.collect({
	arg loop;
	loop.postln;
	Buffer.read(s, loop);
});
~idx_buffer= 1;

~fft_n_frames= 1024;
// FFT stereo
~fft_buffers= Buffer.alloc(s, ~fft_n_frames, 1) ! 2;

// pour test PV_BinDelay
~delayBuf= Buffer.alloc(s, ~fft_n_frames/ 2, 1, completionMessage: {arg buff;
	buff.numFrames.postln;
	//buff.setnMsg(0, Array.fill(buff.numFrames, { arg i; rrand(0.0, 1.0) }));
	buff.setnMsg(0, Array.fill(buff.numFrames, { arg i; if(i< 10, 0.0, 0.1) }));
});
~fdbBuf= Buffer.alloc(s, ~fft_n_frames/ 2, 1, completionMessage: {arg buff;
	buff.numFrames.postln;
	//buff.setnMsg(0, Array.fill(buff.numFrames, { arg i; rrand(0.0, 100.0) }));
	buff.setnMsg(0, Array.fill(buff.numFrames, { arg i; 10.0 }));
});
)

// on vérifie qu'il y a bien qqchose dans ~buffers
Post << ~buffers[~idx_buffer];
~buffers[~idx_buffer].plot;
~fft_buffers[0].plot;
~delayBuf.plot;
~fdbBuf.plot;

(
SynthDef("test1", {
	var in, chain, ifft, delayBuf, fbBuf, hop;
	hop= 0.5; // défaut = 0.5
	in= PlayBuf.ar(2, ~buffers[~idx_buffer], BufRateScale.kr(~buffers[~idx_buffer]), trigger:1, loop:1);
	// on peut utiliser des buffers temporaires LocalBuf
	//chain= FFT(Array.fill(2, { LocalBuf(2048, 1) }), in);
	chain= FFT(~fft_buffers, in, hop:hop);
	chain= chain[0];

	// il existe pas mal de PV_...
	//chain= PV_MagFreeze(chain, SinOsc.kr(1.0));
	//chain = PV_MagSmear(chain, MouseX.kr(0, 100));
	//chain = PV_SpectralEnhance(chain, 8, 2, MouseX.kr(0, 0.99));

	chain = PV_BinDelay(chain, 2.0, ~delayBuf, ~fbBuf, hop:hop);

	ifft= IFFT(chain);
	Out.ar(0, ifft);
}).add;

// detection de beat avec PV_JensenAndersen; voir aussi PV_HainsworthFoote
SynthDef("test2", {
	var in, chain, detect;
	in= PlayBuf.ar(2, ~buffers[~idx_buffer], BufRateScale.kr(~buffers[~idx_buffer]), trigger:1, loop:1);
	chain= FFT(~fft_buffers, in);
	chain= chain[0];
	detect= PV_JensenAndersen.ar(chain, threshold:0.1);
	Out.ar(0, in* Decay.ar(0.1 * detect, 0.1));
}).add;

// ici on applique une fonction pour chaque bin de la FFT
SynthDef("test3", {
	var in, chain, ifft, lfo;

	in= PlayBuf.ar(2, ~buffers[~idx_buffer], BufRateScale.kr(~buffers[~idx_buffer]), trigger:1, loop:1);

	chain= FFT(~fft_buffers, in);

	chain= chain[0];

	lfo = LFPar.kr(0.5).range(0.1, 1);

	//this function gets called once for every bin
	// pas bien compris diff entre pvcollect et pvcalc
	chain= chain.pvcollect(~fft_buffers[0].numFrames, { |mag, phase, index|
	//chain= chain.pvcalc(~fft_buffers[0].numFrames, { |mag, phase|
		// doit renvoyer (amplitude, phase) ou juste amplitude

		// phase aléatoire
		//[mag, pi.rand]

		// spectral delay
		if(index< 100, mag, mag + DelayN.kr(mag, 1, 0.1));
	},frombin:0, tobin:250, zeroothers:1);

	ifft= IFFT(chain);
	Out.ar(0, ifft);
}).add;
)


x= Synth("test1");
x.free;
y= Synth("test2");
y.free;
z= Synth("test3");
z.free;


