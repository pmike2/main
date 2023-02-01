"globals.scd".loadRelative();

(
// bus des effets, vers lequel on va diriger les instrus
~bus_fx= Bus.audio(s, 2);
// bus sidechain
~bus_sidechain= Bus.audio(s, 2);
// groupe des effets
~group_fx= Group.new;
// groupe des instrus
~group_instrus= Group(~group_fx, \addBefore);
~compressor= Synth(\compressor, [\in, ~bus_fx, \ctrl, ~bus_sidechain], ~group_fx);
// on peut imposer l'ordre des effets au sein de leur groupe
~delay= Synth.after(~compressor, \simple_delay, [\in, ~bus_fx]);
//~delay= Synth.before(~compressor, \delay, [\in, ~bus_fx]);
~kick= Synth(\kick, [\out, ~bus_fx], ~group_instrus);
// peut-êre pas la meilleure facon de faire
~kick_sidechain= Synth(\kick, [\out, ~bus_sidechain]);
~hihat= Synth(\hihat, [\out, ~bus_fx], ~group_instrus);
~snare= Synth(\snare, [\out, ~bus_fx], ~group_instrus);
)

(
// config instrus et effets
~kick.set(\freq_env_amount, 1.0);
~kick.set(\freq_env_dur, 0.05);
~kick.set(\freq_min, 20);
~kick.set(\freq_max, 200);
~kick.set(\amp_dur, 0.9);
~kick.set(\amp, 0.7);

~hihat.set(\freq1, 1000);
~hihat.set(\freq2, 9000);
~hihat.set(\feedback, 12.0);
~hihat.set(\amp_dur, 0.4);
~hihat.set(\amp, 0.03);

~snare.set(\mix, 0.4);
~snare.set(\room, 2.0);
~snare.set(\damp, 0.3);
~snare.set(\amp_dur, 0.01);
~snare.set(\amp_dur_2, 0.8);
~snare.set(\amp, 0.8);

~compressor.set(\tresh, 0.1);
~compressor.set(\slope_below, 1.0);
~compressor.set(\slope_above, 0.1);
~compressor.set(\clamp_time, 0.01);
~compressor.set(\relax_time, 0.01);
~compressor.set(\mul, 2.0);

~delay.set(\delaytime, 0.2);
~delay.set(\dry, 0.8);
~delay.set(\wet, 0.1);
)

// on envoie en plus un message OSC avec ~net_addr pour synchro visuel Processing
(
~clock= TempoClock(1.5);
~net_addr= NetAddr.new("127.0.0.1", 57111);

~task_kick= Task({
	var p_kick = Pseq([1, 0, 0, Prand([0, 1], 1), 0], inf).asStream;
	//var p_kick = Pseq([1], inf).asStream;
	var n_kick;
	loop({
		n_kick= p_kick.next;
		~kick.set(\t_trig, n_kick);
		~kick_sidechain.set(\t_trig, n_kick);
		~net_addr.sendMsg("kick", n_kick);
		0.5.wait;
	});
}, ~clock);

~task_hihat= Task({
	var p_hihat= Pseq([1, Prand([0, 1], 1)], inf).asStream;
	//var p_hihat = Pseq([1], inf).asStream;
	var n_hihat;
	loop({
		n_hihat= p_hihat.next;
		~hihat.set(\t_trig, n_hihat);
		~net_addr.sendMsg("hihat", n_hihat);
		0.25.wait;
	});
}, ~clock);

~task_snare= Task({
	var p_snare= Pseq([0, 0, 1, 0], inf).asStream;
	//var p_snare = Pseq([1], inf).asStream;
	var n_snare;
	loop({
		n_snare= p_snare.next;
		~snare.set(\t_trig, n_snare);
		~net_addr.sendMsg("snare", n_snare);
		0.5.wait;
	});
}, ~clock);
)


(
// on quantize au beat et on reset pour être sur d'être synchros
~task_kick.play(doReset:true, quant:1);
~task_hihat.play(doReset:true, quant:1);
~task_snare.play(doReset:true, quant:1);
)

// changement tempo
~clock.tempo= 1.5;

// commande commune à tous les instrus
~group_instrus.set(\amp, 0.01);

(
~task_kick.stop;
~task_hihat.stop;
~task_snare.stop;
)


(
CmdPeriod.run;
currentEnvironment.clear;
)
