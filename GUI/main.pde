import processing.serial.*;

Serial myPort;
String angle = "";
String distance = "";
String data = "";
int iAngle, iDistance;

void setup() {
  size(1200, 700); 
  smooth();
  myPort = new Serial(this, "COM5", 9600); 
  myPort.bufferUntil('.');
}

void draw() {
  fill(98, 245, 31);
  noStroke();
  fill(0, 4); 
  rect(0, 0, width, height - height * 0.065); 

  drawRadar();
  drawLine();
  drawObject();
  drawText();
}

void serialEvent(Serial myPort) {
  data = myPort.readStringUntil('.');
  if (data != null) {
    data = data.trim(); 
    if (data.length() > 0 && data.endsWith(".")) {
      data = data.substring(0, data.length() - 1);
      int index1 = data.indexOf(",");
      if (index1 > 0) {
        angle = data.substring(0, index1);
        distance = data.substring(index1 + 1);
        iAngle = int(angle);
        iDistance = int(distance);
      }
    }
  }
}

void drawRadar() {
  pushMatrix();
  translate(width/2, height - height*0.074);
  noFill();
  strokeWeight(2);
  stroke(98, 245, 31);
  // Koncentricni krugovi
  arc(0, 0, (width - width*0.0625), (width - width*0.0625), PI, TWO_PI);
  arc(0, 0, (width - width*0.27), (width - width*0.27), PI, TWO_PI);
  arc(0, 0, (width - width*0.479), (width - width*0.479), PI, TWO_PI);
  arc(0, 0, (width - width*0.687), (width - width*0.687), PI, TWO_PI);
  // Linije uglova
  line(-width/2, 0, width/2, 0);
  line(0, 0, (-width/2)*cos(radians(30)), (-width/2)*sin(radians(30)));
  line(0, 0, (-width/2)*cos(radians(60)), (-width/2)*sin(radians(60)));
  line(0, 0, (-width/2)*cos(radians(90)), (-width/2)*sin(radians(90)));
  line(0, 0, (-width/2)*cos(radians(120)), (-width/2)*sin(radians(120)));
  line(0, 0, (-width/2)*cos(radians(150)), (-width/2)*sin(radians(150)));
  popMatrix();
}

void drawLine() {
  pushMatrix();
  strokeWeight(9);
  stroke(30, 250, 60);
  translate(width/2, height - height*0.074);
  line(0, 0, (height*0.87)*cos(radians(iAngle)), -(height*0.87)*sin(radians(iAngle)));
  popMatrix();
}

void drawObject() {
  pushMatrix();
  translate(width/2, height - height*0.074);
  strokeWeight(9);
  stroke(255, 10, 10); 
  float pixsDistance = iDistance * ((height - height*0.1666) * 0.025); 
  if (iDistance < 40 && iDistance > 0) {
    line(pixsDistance*cos(radians(iAngle)), -pixsDistance*sin(radians(iAngle)), 
         (height*0.87)*cos(radians(iAngle)), -(height*0.87)*sin(radians(iAngle)));
  }
  popMatrix();
}

void drawText() {
 
  noStroke();
  fill(0); // Cista crna boja
  rect(0, height - height * 0.08, width, height * 0.08);


  fill(98, 245, 31);
  textSize(22);
  
  
  text("Ugao: " + iAngle + "°", width * 0.08, height * 0.95);
  
  
  if (iDistance < 40 && iDistance > 0) {
    text("Distanca: " + iDistance + " cm", width * 0.55, height * 0.95);
  } else {
    text("Distanca: Izvan opsega", width * 0.55, height * 0.95);
  }
}
