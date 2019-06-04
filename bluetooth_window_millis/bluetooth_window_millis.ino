#include <DHT11.h>
#include <Time.h>
#include <TimeLib.h>
#include <SoftwareSerial.h> //시리얼 통신 라이브러리 호출
#include <Stepper.h>

const long delayTime_app = 5000;
const long delayTime_window = 8000;
int blueTx=2;   //Tx (보내는핀 설정)
int blueRx=3;   //Rx (받는핀 설정)
SoftwareSerial BTSerial(blueTx, blueRx);  //시리얼 통신을 위한 객체선언
int res=2038;
Stepper stepper(res, 10, 5, 9, 4);//IN4, IN2, IN3, IN1
int windowState=0;//창문상태
int outState=0;//외출상태

//미세먼지센서
unsigned long pulse1 = 0;
unsigned long pulse2 = 0;
float inDust=0;//ugm3
float outDust=0;
//온습도센서
float hum=0;
int pin=7;
DHT11 dht11(pin);
//초음파센서
int trigPin = 11;//trigger
int echoPin = 12;//Echo
long duration, cm;
String data = "";//입력받은 command
unsigned long prev_time_app = 0;
unsigned long prev_time_window = 0;

void setup() {
  BTSerial.begin(9600);//블루투스 시리얼
  Serial.begin(9600);   //시리얼모니터 
  pinMode(8,INPUT);//내부미세먼지센서
  pinMode(6,INPUT);//외부미세먼지센서
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  stepper.setSpeed(12);
}
void loop() {

    //초음파센서
    digitalWrite(trigPin,LOW);
    delayMicroseconds(5);
    digitalWrite(trigPin,HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin,LOW);
    pinMode(echoPin,INPUT);
    duration = pulseIn(echoPin,HIGH);
    cm = (duration/2)/29.1;
    
    if(millis() - prev_time_app >= delayTime_app){

      //내부 미세먼지 센서
      pulse1=pulseIn(8, LOW, 20000);//pulse에 pin8에서 LOW신호를 받을 때까지 걸리는 시간
      inDust=pulse1ugm3(pulse1);
      //외부미세먼지센서
      pulse2=pulseIn(6, LOW, 20000);
      outDust=pulse2ugm3(pulse2);
      //온습도센서
      float temp, humi;
      dht11.read(humi,temp);      
      //내부미세먼지센서값 어플에 출력
      BTSerial.print("\nIn dust : ");
      BTSerial.print(inDust,4);//미세먼지를 소숫점 4자리수까지 출력
      BTSerial.print("ug/m3");
      Serial.print("\nIn dust : ");
      Serial.print(inDust,4);
      if(inDust<=15){
      BTSerial.print("\tgood\n");
      }
      else if(inDust>15 and inDust<=35){
      BTSerial.print("\tnormal\n");
      }
      else if(inDust>35){
      BTSerial.print("\tbad\n");
      }
    
      //외부 미세먼지센서값 어플에 출력
      BTSerial.print("Out dust : ");
      BTSerial.print(outDust,4);//미세먼지를 소숫점 4자리수까지 출력
      Serial.print("\nOut dust : ");
      Serial.print(outDust,4);
      BTSerial.print("ug/m3");
      if(outDust<=15){
        BTSerial.print("\tgood\n");
      }
      else if(outDust>15 and outDust<=35){
        BTSerial.print("\tnormal\n");
      }
      else if(outDust>35){
        BTSerial.print("\tbad\n");
      }
    
      //온습도 센서
      BTSerial.print("temperature : ");
      BTSerial.print(temp);
      BTSerial.print("Cel\n");
      BTSerial.print("humidity : ");
      BTSerial.print(humi);
      BTSerial.print("%\n");
      prev_time_app = millis();
    }//delay

    if(millis() - prev_time_window >= delayTime_window){
      //창문 자동 작동
      if (inDust>outDust and hum<=60){//내부미세먼지가 높고 습도가 낮으면
        if(windowState==0){//창문이 닫혀있을때
          stepper.step(res);//창문열기 
          windowState=1;
        }
      }
      else if(inDust<outDust or hum>60){//외부미세먼지가 높거나 습도가 높으면
        if(windowState==1){//창문이 열려있을때
          stepper.step(-res);//창문닫기
          windowState=0;
        }
      }
      prev_time_window = millis();
    }

    //방범
    if(outState==1){
      if(cm<=5){//초음파센서에 걸림
        BTSerial.print("Invade!!!");
      }
    }

  
  
  //설정
  while(BTSerial.available())  //BTSerial에 전송된 값이 있으면
  {
    char myChar = (char)BTSerial.read();//수신 받은 데이터 저장
    data += myChar;//설정 command값
    delay(5);//수신 문자열 끊김 방지
  }
  if(!data.equals(""))//data값이 있다면
  {
    Serial.print(data);
    if(data=="open"){//창문열기
      Serial.print("open");
      if(windowState == 0){//창문이 닫혀있으면
        stepper.step(res);
        windowState=1;
      }
      data="";
    }
    else if(data=="close"){//창문닫기
      if(windowState == 1){//창문이 열려있으면
        stepper.step(-res);
        windowState=0;
      }
      data="";
    }
    else if(data=="nightOff"){//밤에 사용X
      if(hour()>=0 and hour()<=7){//밤에
        if(windowState==1){//창문에 열려있으면
          stepper.step(-res);//창문닫기
          windowState=0;
        }
      }
      data="";
    }
    else if(data=="out"){//외출
      outState=1;
      data="";
    }
    else if(data="in"){
      outState=0;
      data="";
    }
    else if(data=="one"){//창문 1단계
      res=2038;
      data="";
    }
    else if(data=="two"){//창문 2단계
      Serial.print(res);
      res=2038*2;
      Serial.print(res);
      data="";
    }
    else if(data=="three"){//창문 3단계
      res=2038*3;
      data="";
    }
  }
  
  
  
}

float pulse1ugm3(unsigned long pulse1){//pulse1ugm3함수 설정
  float value = (pulse1 - 1400) / 14.0;
  if(value > 300){
    value = 0;
  }
  return value; 
}

float pulse2ugm3(unsigned long pulse2){//pulse2ugm3함수 설정
  float value = (pulse2 - 1400) / 14.0;
  if(value > 300){
    value = 0;
  }
  return value; 
}
