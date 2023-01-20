
MIDIIn.connectAll;

// Attention : cmd + . fait un free sur tous les MIDIFunc

a= MIDIFunc.noteOn({arg ...args; args.postln});
b= MIDIFunc.cc({arg ...args; args.postln});
c= MIDIFunc.program({arg ...args; args.postln});

a.free;
b.free;
c.free;
