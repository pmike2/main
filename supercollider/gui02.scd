(
~cursor= 0.0;
~tempo= 150.0;
~clock= TempoClock(~tempo/ 60);

SynthDef(\beat_pulse, {
	SendReply.ar(Impulse.ar(0), '/beat');
}).add;

OSCdef(\listen_beat, {
	arg msg, time;
	//[msg, time].postln;
	~cursor= (~cursor+ 0.05).wrap(0.0, 1.0);
	{v.refresh}.defer;
}, '/beat', s.addr);
)

(
w = Window.new.front;
v = UserView(w,Rect(50,50,200,20));
v.drawFunc = {
	var value= ~cursor;
    // Draw the frame
    Pen.strokeColor = Color.black;
    Pen.addRect(Rect(0, 0, v.bounds.width, v.bounds.height));
    Pen.stroke;
    // Draw the triangle
    Pen.fillColor = Color.red;
    Pen.moveTo(((v.bounds.width*value)-5) @ v.bounds.height);
    Pen.lineTo(((v.bounds.width*value)+5) @ v.bounds.height);
    Pen.lineTo(((v.bounds.width*value)) @ (v.bounds.height/2));
    Pen.lineTo(((v.bounds.width*value)-5) @ v.bounds.height);
    Pen.fill;
};
/*v.action = {~cursor.postln; v.refresh};
v.keyDownAction = {
	arg view, char, modifiers, unicode,keycode;
	~cursor= 1.0.rand;
    v.doAction;
};*/
)

(
~clock.sched(0.0, {
	Synth(\beat_pulse);
	0.1;
});
)

~clock.stop;
