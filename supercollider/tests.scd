"globals.scd".loadRelative();



k= Synth(\kick);
k.set(\t_trig, 1);

h= Synth(\hihat);
h.set(\t_trig, 1);

i= Synth(\snare);
i.set(\t_trig, 1);


~clock= TempoClock(1.5);
~net_addr= NetAddr.new("127.0.0.1", 57111);


(
~task_snare= Task({
	var p_snare= Pseq([
		1, 0, 1, 0,
		1, 0, 1, 0,
		1, 0, 0, 1,
		0, 0, 0, 0

	], inf).asStream;
	//var p_snare = Pseq([1], inf).asStream;
	var n_snare;
	loop({
		n_snare= p_snare.next;
		h.set(\t_trig, n_snare);
		//~net_addr.sendMsg("snare", n_snare);
		0.5.wait;
	});
}, ~clock);
)

~task_snare.play(doReset:true, quant:1);
~task_snare.stop;

~clock.tempo= 3.5;
