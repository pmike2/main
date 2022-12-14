/*
a - z  : variables globales
sinon faire débuter nom variable par ~ pour avoir du global

cmd enter -> execute bloc
shift enter -> execute line or selection
cmd b -> boot ; s vaut server localhost
cmd . -> stop sound (free all synths)
cmd shift p -> clear log
cmd d sur un mot-clef -> help
*/

// evaluate function
f= {arg x=0; x* 2};
f.value; // ou f.value() ou value(f);
f.value(3); // si arguments

// scope de tout le son du server
s.scope();

// redémarre serveur
s.reboot;

// libère un synth
x.free;

// reinit environment variables
currentEnvironment.clear;
