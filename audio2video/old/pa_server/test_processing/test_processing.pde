import oscP5.*;
import netP5.*;

int WIDTH= 800;
int HEIGHT= 800;
int N= 64;
OscP5 oscP5;
float []alphas= new float[N];
int w= 90;
int h= 90;
int eps= 3;
int n_w;


void settings() {
  size(WIDTH, HEIGHT);
}

void setup() {
  oscP5 = new OscP5(this, 12001);
  
  for (int i=0; i<N; ++i) {
      alphas[i]= 0.0;
  }
  noStroke();
  
  n_w= WIDTH/ (w+ eps);
}

void draw() {
  int x, y;
  background(255);
  for (int i=0; i<N; ++i) {
      fill(204, 102, 0, alphas[i]* 255);
      
      x= (i % n_w)* (w+ eps);
      y= (i / n_w)* (h+ eps);
      rect(x, y, w, h);
      
      alphas[i]-= 0.05;
  }
}

void oscEvent(OscMessage theOscMessage) {
  if (theOscMessage.addrPattern().equals("/test1")) {
    int i = theOscMessage.get(0).intValue();
    alphas[i]= 1.0;
    //println(i);
  }
}