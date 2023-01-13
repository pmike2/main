// serveur vers client -------------------------------------------------------
// On receiving a trigger, sends a trigger message from the server back to the client.
// "tr" est le message lié au Trig
// 0.9 est la valeur envoyée
(
SynthDef("test_trig", {
	arg id;
	SendTrig.kr(Dust.kr(1.0), id, 0.9);
}).add;

// dis que faire lorsque l'on recoit le message "/tr"
o= OSCFunc({
	arg msg, time;
    [time, msg].postln;
},"/tr", s.addr);

// création d'un id unique
~trig_id= UniqueID.next;
Synth("test_trig", ["id", ~trig_id]);
)

o.free; o.remove;


// -----------------------------------------------------------------------------
// SendReply est plus complet que SendTrig
(
a = {
    var trig = Dust.kr(8),
    decay = Decay2.kr(trig, 0.01, 0.1),
    sig = SinOsc.ar(TExpRand.kr(200, 600, trig), 0, 0.1) * decay;
    SendReply.kr(trig, '/bleep', trig);
    sig ! 2
}.play;
)

(
o = OSCdef(\bleepResponder, { |msg|
    msg.postln;
}, '/bleep', s.addr, argTemplate: [a.nodeID]);
)

a.free; o.remove;


