#include <SoftwareSerial.h>
#include <MsTimer2.h>
#include <EEPROM.h>

//Motor A
int PWMA = 5; //Speed control 
int AIN1 = 10; //Direction
int AIN2 = 8; //Direction
//Motor B
int PWMB = 6; //Speed control
int BIN1 = 12; //Direction
int BIN2 = 13; //Direction
int STBY = 11; //standby  LOW = stop motors
//special pins ex:eyes
int PINA0 = 14;
int PINA1 = 15;
int PINA2 = 16;
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

void setup() {    
  pinMode(STBY, OUTPUT);
  pinMode(PWMA, OUTPUT);pinMode(AIN1, OUTPUT);pinMode(AIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);pinMode(BIN1, OUTPUT);pinMode(BIN2, OUTPUT);
  pinMode(PINA0, OUTPUT);pinMode(PINA1, OUTPUT);pinMode(PINA2, OUTPUT);
  pinMode(HRT, OUTPUT);analogWrite(HRT,128);
  
//read last value of speedmax and accel
speedmax= EEPROM.read(0);if(speedmax==0){speedmax=255;}
accel= EEPROM.read(1); if(accel==0){accel=128;}  
         
   delay(10);  

    bluetooth.begin(115200);  //Bluetooth Mate 115200bps
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
analogWrite(PWMA, speednow);
analogWrite(PWMB, speednow);
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

void move_forward(){motor(1,1);motor(2,1);moveflag=true;}
void move_back(){motor(1,0);motor(2,0);moveflag=true;}
void move_left(){motor(1,1);motor(2,0);moveflag=true;}
void move_right(){motor(1,0);motor(2,1);moveflag=true;}
void move_stop(){digitalWrite(STBY, 0);digitalWrite(AIN1, 0);digitalWrite(AIN2, 0);digitalWrite(BIN1, 0);digitalWrite(BIN2, 0);moveflag=false;speednow=0;}
void move_brakes(){digitalWrite(AIN1, 1);digitalWrite(AIN2, 1);digitalWrite(BIN1, 1);digitalWrite(BIN2, 1);moveflag=false;speednow=0;}

void motor(int motor,  int direction){
  digitalWrite(STBY, 1); //disable standby
  int inPin1 = 1;
  int inPin2 = 0;
  if(direction == 1){
    inPin1 = 0;
    inPin2 = 1;
  }
  if(motor == 1){
    digitalWrite(AIN1, inPin1);
    digitalWrite(AIN2, inPin2);
    analogWrite(PWMA, speednow);
  }else{
    digitalWrite(BIN1, inPin1);
    digitalWrite(BIN2, inPin2);
    analogWrite(PWMB, speednow);
  }
}//motor



