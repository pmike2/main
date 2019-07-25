
public class Vertex {
    public float x;
    public float y;
    public float z;
    public int r;
    public int g;
    public int b;
    
}

int NBLOCKS= 64;

Vertex []vertices;

void setup() {
  size(600,600,P3D);
  

  
  String[] lines = loadStrings("/Volumes/Cezanne/Cezanne/perso_dev/ovh/www/codes/audio2video/pa_server_V2/pa_server.log");
  vertices= new Vertex[lines.length];
  for (int i = 0 ; i < lines.length; i++) {
      String[] list = split(lines[i], ' ');
      Vertex v= new Vertex();
      v.x= float(list[0])* 1.0;
      v.y= float(list[1])* 10.0;
      v.z= float(list[2])* 0.001;
      if (int(list[3])== 0) {
          v.r= 100; v.g= 200; v.b= 100;
      }
      else {
          v.r= 200; v.g= 100; v.b= 100;
      }
      vertices[i]= v;
  }
}

void draw() {
    background(255);
    beginShape();
    lights();
    translate(0, 400, -200);
    rotateX(PI/2);
    rotateZ(-PI/6);
    for (int i = 0 ; i < vertices.length; i++) {
        //print("%f %f %f %i %i %i\n", vertices[i].x, vertices[i].y, vertices[i].z, vertices[i].r, vertices[i].g, vertices[i].b);
        pushMatrix();
        fill(vertices[i].r, vertices[i].g, vertices[i].b);
        //vertex(vertices[i].x, vertices[i].y, vertices[i].z);
        translate(vertices[i].x, vertices[i].y, vertices[i].z);
        box(0.2);
        popMatrix();
    }
    endShape();
}