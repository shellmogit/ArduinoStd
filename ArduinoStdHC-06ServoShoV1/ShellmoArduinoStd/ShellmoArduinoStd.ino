/* 2014 05 13 Version 1.00 */
/* For Rotation servo with HC-06 Bluetooth module */

#include <SoftwareSerial.h>
#include <MsTimer2.h>
#include <Servo.h>
#include <EEPROM.h>

//special pins ex:eyes
int PINA0 = 14;
int PINA1 = 15;
int PINA2 = 16;
int PINA3 = 17;
int PINA4 = 18;
int PINA5 = 19;

//Heart LED
int HRT = 9;
//Bluetooth pins
int bluetoothTx = 2;
int bluetoothRx = 3;
SoftwareSerial bluetooth(bluetoothTx, bluetoothRx);

char c=0;//Serial data
short BLUETOOTH_AVAILABLE = 0;
int DATA_TYPE=0;

int heart = 0,heartrate = 10,tcount=0,eyecount=0;
short PHASE_HEART = 0;

int spd=99,speednow=0,speedmax,accel;
boolean moveflag = false,eyeflag=false; 

//servo
int servostop = 1500;//set initial servo position 
int PINSR = 10, PINSL=11;//Servo pins on Arduino
Servo SERVO_R; // create servo object
Servo SERVO_L; // create servo object
short direction_r,direction_l;

void setup() {    
  pinMode(HRT, OUTPUT);analogWrite(HRT,128);
  
//read last value of speedmax and accel
speedmax= EEPROM.read(0);if(speedmax==0){speedmax=255;}
accel= EEPROM.read(1); if(accel==0){accel=128;}  
         
   delay(10);  

    bluetooth.begin(9600);  //Bluetooth Mate 115200bps
       bluetooth.println("Start");
          MsTimer2::set(100, timer); // 500ms period
  MsTimer2::start();

  
}

void timer(){

//accel
if(moveflag==true){
if(speednow<speedmax){
speednow += accel/3;   
  }
else{speednow = speedmax;}
if(speednow>255){speednow = 255;}

if(direction_r==1){SERVO_R.writeMicroseconds(servostop-speednow);}
else{SERVO_R.writeMicroseconds(servostop+speednow);}
if(direction_l==1){SERVO_L.writeMicroseconds(servostop+speednow);}
else{SERVO_L.writeMicroseconds(servostop-speednow);}

}

//heart rate  
if(PHASE_HEART==0){heart +=heartrate; if(heart>=255){heart=255; PHASE_HEART=1;}}
else if(PHASE_HEART==1){heart -=heartrate; if(heart<=0){heart=0; PHASE_HEART=0;}}
analogWrite(HRT,heart);
if(tcount>=7){
 if(moveflag==true){heartrate += speednow/100;}
 else{heartrate -= 1;  }
 if(heartrate<=10){heartrate=10;}
 else if(heartrate>=50){heartrate=50;}
 tcount=0;
  }
tcount++;

//eye safety
if(eyeflag==true){eyecount++;if(eyecount>20){eyeflag=false;digitalWrite(PINA0, 0);digitalWrite(PINA1, 0);digitalWrite(PINA2, 0);}}

}//timer


void loop() {

 BLUETOOTH_AVAILABLE = bluetooth.available();
     if(BLUETOOTH_AVAILABLE==1){
      c = bluetooth.read();
       bluetooth.println(c);
       switch(c){
       case 'w':move_forward();break;
       case 's':move_back();break;
       case 'a':move_left();break;
       case 'd':move_right();break;
       case 'x':move_stop();break;
       case 'z':move_stop();break;
       case 'c':move_brakes();break;
       case 'h':digitalWrite(PINA0, 1);eyeflag=true;eyecount=0;break;
       case 'j':digitalWrite(PINA1, 1);eyeflag=true;eyecount=0;break;
       case 'l':digitalWrite(PINA2, 1);eyeflag=true;eyecount=0;break;      
       case 'i':digitalWrite(PINA0, 0);eyeflag=false;break;
       case 'k':digitalWrite(PINA1, 0);eyeflag=false;break;
       case 'm':digitalWrite(PINA2, 0);eyeflag=false;break;       
       case 'v':DATA_TYPE=1;break;
       case 'b':DATA_TYPE=3;break;       
       default:
       if(c>=48&&c<=57){
         //read value of speedmax or accel        
      if(DATA_TYPE==1){
        spd = 10*(c -48);
        DATA_TYPE = 2;
      }
      else if(DATA_TYPE==2){
        spd =spd + c -48;
        speedmax=spd*2+spd/2;
        EEPROM.write(0,speedmax); 
        DATA_TYPE = 0;
      }
      else if(DATA_TYPE==3){
        accel = 10*(c -48);
        DATA_TYPE = 4;
      }
      else if(DATA_TYPE==4){
        accel =accel + c -48;
        accel=accel*2+accel/2;
        EEPROM.write(1,accel);
        DATA_TYPE = 0;        
      }
      }
      else{move_stop();}//other data = stop 
      break;
       
       }//switch data

}//bluetooth available
}//loop
void move_forward(){servo_attach(); direction_r = 1; direction_l = 1; moveflag = true;}
void move_back(){   servo_attach(); direction_r = 0; direction_l = 0; moveflag = true;}
void move_left(){   servo_attach(); direction_r = 1; direction_l = 0; moveflag = true;}
void move_right(){  servo_attach(); direction_r = 0; direction_l = 1; moveflag = true;}
void move_stop(){  SERVO_R.writeMicroseconds(servostop);SERVO_L.writeMicroseconds(servostop); SERVO_R.detach();SERVO_L.detach();moveflag=false;speednow=0;} //detach pins for reduce malfunction, but before detach, need to write servostop for next smoothly moving
void move_brakes(){SERVO_R.writeMicroseconds(servostop);SERVO_L.writeMicroseconds(servostop); SERVO_R.detach();SERVO_L.detach();moveflag=false;speednow=0;}

void servo_attach(){SERVO_R.attach(PINSR);SERVO_L.attach(PINSL);}


