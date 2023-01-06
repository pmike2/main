import processing.video.*;
Movie myMovie;

void setup() {
  size(200, 200);
  background(0);
  myMovie = new Movie(this, "/Users/home/Desktop/pm/son/records/rec_solo_01_2021/00021.MP4");
  myMovie.loop();
}

void draw() {
  image(myMovie, 0, 0);
}

// Called every time a new frame is available to read
void movieEvent(Movie m) {
  m.read();
}
