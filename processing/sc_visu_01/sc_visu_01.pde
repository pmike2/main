/*

*/

import oscP5.*;
import netP5.*;
import java.util.Map;

OscP5 oscP5;
HashMap<String, Instru> instrus;

void setup() {
  size(400,400);
  frameRate(60);
  oscP5 = new OscP5(this, 57111);
  instrus= new HashMap<String, Instru>();
  instrus.put("kick", new Instru("kick", color(255, 0, 0), 30, 30));
  instrus.put("snare", new Instru("snare", color(0, 255, 0), 200, 30));
  instrus.put("hihat", new Instru("hihat", color(0, 0, 255), 30, 200));
}

void draw() {
  background(0);
  for (Map.Entry me : instrus.entrySet()) {
    Instru inst= Instru.class.cast(me.getValue());
    inst.anim();
    inst.display();
  }
}

void keyPressed() {
  exit();  // Stops the program
}

void oscEvent(OscMessage msg) {
  /*println("--------");
  println(msg.arguments());
  println(msg.addrPattern());
  println(msg.typetag());*/
  if (msg.checkTypetag("i")) {
    String instru_name= msg.addrPattern();
    int value= (int)(msg.arguments()[0]);
    if (value== 1) {
      instrus.get(instru_name).trig();
    }
  }
}

class Instru {
  String _name;
  float _xpos;
  float _ypos;
  color _c;
  int _count;
  int _max_count;
  int _alpha;
  
  Instru(String name, color c, float xpos, float ypos) {
    _name= name;
    _c= c;
    _xpos= xpos;
    _ypos= ypos;
    _count= 0;
    _max_count= 20;
    _alpha= 255;
  }
  
  void display() {
    if (_count> _max_count) {
      return;
    }
    stroke(0);
    fill(rgba(red(_c), green(_c), blue(_c), _alpha));
    rectMode(CENTER);
    rect(_xpos, _ypos, 50, 50);
  }
  
  void anim() {
    _count++;
    _alpha= 255* _count/ _max_count;
  }
  
  void trig() {
    println("trig : "+ _name);
    _count= 0;
  }
}
