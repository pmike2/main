// routines et tasks ------------------------------------------------------------------------
// 1 beat / sec = 60 BPM
c= TempoClock.new(1);
c.schedAbs(t.beats.ceil, { arg beat, sec; [beat, sec].postln; 3 });
c.stop
c.free

(
var duration= Routine({
	[1, 2, 3].do({arg x; x.yield});
});
var msg= Routine({
	["hello", "old", "friend"].do({arg x; x.yield});
});

// alternative Pseq
//var duration= Pseq([1, 2, 3]).asStream;
//var msg= Pseq(["hello", "old", "friend"]).asStream;

t= Task({
	var delta;
	while {
		delta= duration.next;
		delta.notNil
	} {
		msg.next.postln;
		delta.yield;
	}
}, c); // on donne c en argument ce qui permet si on utilise 'quant' en param de play de synchroniser la task avec la clock
)

t.play(quant:4);
t.stop;


// Pseq -----------------------------------------------
p = Pseq([60, 72, 71, 67, 69, 71, 72, 60, 69, 67], 1);
r = p.asStream;
while { (m = r.next).notNil } { m.postln };

p= Pseries(10, 3, 4).asStream;
p.next;

(
var a;
a = Pgauss(0.0, 100, inf);
c = a.asStream.nextN(500);
c.plot(bounds: Rect(10, 10, 520, 380), discrete: true);
c.histo(500).plot(bounds: Rect(10, 410, 520, 380));
)

