/*
port supercollider par défaut = 57110
on ne peut pas prendre ce port si j'ai bien compris car sclang y écoute déjà
il faut donc dans sc émettre des OSC sur un nouveau port à chaque fois que l'on veut
que qqchose se passe dans processing
*/

import oscP5.*;
import netP5.*;

OscP5 oscP5;

void setup() {
  size(400,400);
  frameRate(25);
  oscP5 = new OscP5(this, 57111);
}

void draw() {
  background(0);
}

void keyPressed() {
  exit();
}

void oscEvent(OscMessage msg) {
  println("---------------");
  println("addrpattern : "+ msg.addrPattern());
  println("typetag : "+ msg.typetag());
  print("args :");
  for (int i = 0; i < msg.arguments().length; i++) {
    print(msg.arguments()[i]+ " , ");
  }
  println("");
  //msg.print();
  //msg.printData();
}
