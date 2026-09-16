#include <Keypad.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>
#include <SPI.h>
#include <mcp2515.h>

#define OLED_SDA PB7
#define OLED_SCL PB6
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define  OLED_RESET -1
#define PIN_BUZZ PA3

#define TRIG_PIN PB0
#define ECHO_PIN PB1

#define PIN_SERVO PA0
#define PIN_MOSFET PA1

#define CAN_CS_PIN PA4

Adafruit_SSD1306 display(SCREEN_WIDTH,SCREEN_HEIGHT,&Wire,OLED_RESET);

String input="";

MCP2515 mcp2515(CAN_CS_PIN);
struct can_frame canMsg;


const byte ROWS=4;
const byte COLS=4;
char keys[ROWS][COLS]={
{'1','2','3','A'},
{'4','5','6','B'},
{'7','8','9','C'},
{'*','0','#','D'}
};

byte rowPins[ROWS]={PB12,PB13,PB14,PB15};
byte colPins[COLS]={PB8,PB9,PB3,PB4};

Keypad input_keypad=Keypad(makeKeymap(keys),rowPins,colPins,ROWS,COLS);

Servo my_servo;

unsigned long last_measurement=0;
unsigned long last_buzz=0;
int sensor_distance=0;
int current_angle=90;
int scan_direction=1; 
char current_mode='B';
unsigned long last_can_send=0;
String input_angle_str = ""; 
int target_angle = 90; 

//Servo pomeranje
void Move_Servo(int target_angle,int speed_delay=8){
  target_angle=constrain(target_angle,0,180);
  digitalWrite(PIN_MOSFET,HIGH);
  delay(20);
  if(target_angle>current_angle){
    for(int angle=current_angle;angle<=target_angle;angle++){
      my_servo.write(angle);
      delay(speed_delay);
    }
  }
  else{
    for(int angle=current_angle;angle>=target_angle;angle--){
      my_servo.write(angle);
      delay(speed_delay);
    }
  }
current_angle=target_angle;
delay(speed_delay);
delay(150);
digitalWrite(PIN_MOSFET,LOW);
}
//Ultrazvucni senzor
int Get_Distance(){
  digitalWrite(TRIG_PIN,LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN,HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN,LOW);

  long duration=pulseIn(ECHO_PIN,HIGH,15000);
  if (duration==0) return 99;

  return duration*0.034/2;
}
//C mod kretanje
void Mode_C_Radar_Serial() {
  int next_angle=current_angle+(scan_direction*2);
  if(next_angle>=180){
  next_angle=180;
  scan_direction=-1;
  }
  else if(next_angle<=0){ 
  next_angle=0; 
  scan_direction=1; 
  }
  Move_Servo(next_angle, 15);
  sensor_distance=Get_Distance();

  Serial.print(current_angle);
  Serial.print(",");
  Serial.print(sensor_distance);
  Serial.println("."); 

  Serial1.print(current_angle);
  Serial1.print(",");
  Serial1.print(sensor_distance);
  Serial1.println(".");
}
//BUZZER
void Bip(int buzz_time=30){
  digitalWrite(PIN_BUZZ,HIGH);
  delay(buzz_time);
  digitalWrite(PIN_BUZZ,LOW);
}
//OLED DISPLEJ
void Refresh_Display(String msg_display=""){
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  if(current_mode=='A'){
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("Target Tracker (AUTO)");
  }
  else if(current_mode=='B'){
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("Target Tracker (MAN)");
  }
  else if(current_mode=='C'){
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("Target Tracker (C)");
  }
  else if(current_mode=='D'){
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("Target Tracker (D)");
  }

  display.drawLine(0,10,128,10,SSD1306_WHITE);
 if(current_mode=='D'){
    display.setCursor(0,14);
    display.print("Unos: ");
    display.print(input_angle_str);
    display.print("_");
    display.setCursor(0,24);
    display.print("Cilj: ");
    display.print(target_angle);
    display.print((char)247);
    display.print(" | ");

    if (sensor_distance>=99 || sensor_distance<0){
      display.print("--cm");
    }else{
      display.print(sensor_distance);
      display.print("cm");
    }
 }
 if(current_mode!='D'){
  display.setCursor(0,14);
  display.print("Mod: ");
  display.print(current_mode);

  display.setCursor(64,14);
  display.print("Ugao: ");
  display.print(current_angle);
  display.print((char)247);

  display.setCursor(0,24);
  display.print("Dist: ");
  if (sensor_distance>=99 || sensor_distance<0){
    display.print("-- cm");
  } else {
    display.print(sensor_distance);
    display.print(" cm");
  }
 }
  display.display();
}

void setup(){
  pinMode(PIN_BUZZ,OUTPUT);
  digitalWrite(PIN_BUZZ,LOW);

  Wire.setSDA(OLED_SDA);
  Wire.setSCL(OLED_SCL);
  Wire.begin();
  
  pinMode(TRIG_PIN,OUTPUT);
  pinMode(ECHO_PIN,INPUT);

  pinMode(PIN_MOSFET,OUTPUT);
  digitalWrite(PIN_MOSFET,LOW);

  my_servo.attach(PIN_SERVO);
  Move_Servo(90);
  Serial.begin(9600);
  Serial1.begin(9600);
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS,MCP_8MHZ);
  mcp2515.setLoopbackMode();

  Serial1.println("MCP2515 Pokrenut.");

  if (display.begin(SSD1306_SWITCHCAPVCC,0x3C)){
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10,10);
    display.println("System Ready...");
    display.display();
    delay(1000);
  }
  input_keypad.setDebounceTime(100);
  Refresh_Display();
}

void loop(){
char pressedKey=input_keypad.getKey();
unsigned long timestamp=millis();

//CAN BUS
if (millis()-last_can_send>=200) {
  last_can_send=millis();

  canMsg.can_id=0x036;
  canMsg.can_dlc=4; 

  canMsg.data[0]=current_angle;                     
  canMsg.data[1]=(sensor_distance >> 8) & 0xFF;     
  canMsg.data[2]=sensor_distance & 0xFF;            
  canMsg.data[3]=(byte)current_mode;     

if (mcp2515.sendMessage(&canMsg) == MCP2515::ERROR_OK) {
  Serial1.print("[CAN TX OK] Mod: ");
  Serial1.print(current_mode);
  Serial1.print(" | Ugao: ");
  Serial1.print(current_angle);
  Serial1.print(" deg");
  Serial1.print(" | Dist: ");
  Serial1.print(sensor_distance);
  Serial1.println(" cm");
} 
else{
  Serial1.println("[CAN TX ERROR] MCP2515 nije poslao poruku!");
}
}

//TASTATURA
if(pressedKey){
  if(pressedKey=='A'){
    current_mode='A';
    input_angle_str = "";
    Refresh_Display();
    }
  else if(pressedKey=='B'){
    current_mode='B';
    input_angle_str = "";
    Refresh_Display();
    }
  else if(pressedKey=='C'){
    current_mode='C';
    input_angle_str = "";
    Refresh_Display();
    }
  else if(pressedKey=='D'){
    current_mode='D';
    input_angle_str = "";
    Refresh_Display();
  }
  else if(current_mode=='D'){
  if (pressedKey>='0' && pressedKey<='9'){
    if (input_angle_str.length()<3){
      input_angle_str+=pressedKey;
      Refresh_Display();
    }
  } 
  else if(pressedKey=='*'){
    input_angle_str="";
    Refresh_Display();
  } 
  else if(pressedKey=='#'){
    if(input_angle_str.length()>0){
      int parsed_angle=input_angle_str.toInt();
      if (parsed_angle>=0 && parsed_angle<=180) {
        target_angle=parsed_angle;
        current_angle=target_angle;
        Move_Servo(current_angle,15);
        sensor_distance=Get_Distance();
      }
      input_angle_str="";
      Refresh_Display();
    }
  }
  }
  else if(current_mode=='B'){
    if(pressedKey=='9'){
      Move_Servo(current_angle-15);
      sensor_distance = Get_Distance();
      Refresh_Display();
    }
  else if(pressedKey=='7'){
    Move_Servo(current_angle+15);
    sensor_distance = Get_Distance();
    Refresh_Display();
  }
  else if(pressedKey=='5'){
    Move_Servo(90);
    sensor_distance = Get_Distance();
    Refresh_Display();
  }
  }
  else if(current_mode=='C'){
  Mode_C_Radar_Serial();
  Refresh_Display();
  }
  }

//ULTRAZVUCNI SENZOR
if(millis()-last_measurement>150){
  last_measurement=millis();
  sensor_distance=Get_Distance();
  if(current_mode=='A'){
    if(sensor_distance>=2 && sensor_distance<=20){
      
    }
    else{
      int next_angle=current_angle+(scan_direction*5);
      if(next_angle>=180){
        next_angle=180;
        scan_direction=-1; 
      }
      else if(next_angle<=0){
        next_angle=0;
        scan_direction=1; 
      }
      Move_Servo(next_angle,6);
      sensor_distance=Get_Distance();
      Refresh_Display();
    }
  }
  else if(current_mode=='C'){
    Mode_C_Radar_Serial();
    Refresh_Display();
  }
  if(sensor_distance>0 && sensor_distance<=20){
    int time_between_buzz=sensor_distance*35;
    if(millis()-last_buzz>time_between_buzz){
      last_buzz=millis();
      Bip(15);
    }
  }
}

}
