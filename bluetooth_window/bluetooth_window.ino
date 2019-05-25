#include <DHT11.h>

#include <Time.h>
#include <TimeLib.h>

#include <SoftwareSerial.h> //시리얼 통신 라이브러리 호출
#include <Stepper.h>

int blueTx=2;   //Tx (보내는핀 설정)
int blueRx=3;   //Rx (받는핀 설정)
SoftwareSerial BTSerial(blueTx, blueRx);  //시리얼 통신을 위한 객체선언
int res=2038;
Stepper stepper(res, );//IN4, IN2, IN3, IN1
int window=0;
int windowState=0;//창문상태
int night=0;
int out=0;
int spinner=0;//창문열리는 정도를 받아올 변수

unsigned long pulse = 0;
//미세먼지센서
float inDust=0;//ugm3
float outDust=0;
float hum=0;
//온습도센서
int pin=7;
DHT11 dht11(pin);
//초음파센서
int trigPin = 11;//trigger
int echoPin = 12;//Echo
long duration, cm, inches;

void setup() {
  BTSerial.begin(9600);//블루투스 시리얼
  Serial.begin(9600);   //시리얼모니터 
  pinMode(8,INPUT);//내부미세먼지센서
  pinMode(,INPUT);//외부미세먼지센서
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  stepper.setSpeed(15);
}
 
void loop() {
  //내부미세먼지센서
  pulse=pulseIn(8, LOW, 20000);//pulse에 pin8에서 LOW신호를 받을 때까지 걸리는 시간
  inDust=pulse2ugm3(pulse);
  //온습도센서
  float temp, humi;
  dht11.read(humi,temp);
  //초음파센서
  digitalWrite(trigPin,LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin,HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin,LOW);
  pinMode(echoPin,INPUT);
  duration = pulseIn(echoPin,HIGH);
  cm = (duration/2)/29.1;
  
  //내부미세먼지센서값 어플에 출력
  if(ugm3>1){//잡음 방지
    BTSerial.print("내부 미세먼지 : ");
    BTSerial.print(inDust,4);//미세먼지를 소숫점 4자리수까지 출력
    BTSerial.print("ug/m3");
    if(inDust<=15){
    BTSerial.print("\t좋음");
    }
  else if(inDust>15 or inDust<=35){
    BTSerial.print("\t보통");
    }
  else if(inDust>35){
    BTSerial.print("\t나쁨");
    }
  }
  delay(600000);
  
  //외부 미세먼지센서값 어플에 출력
  if(ugm3>1){//잡음 방지
    BTSerial.print("외부 미세먼지 : ");
    BTSerial.print(outDust,4);//미세먼지를 소숫점 4자리수까지 출력
    BTSerial.print("ug/m3");
    if(inDust<=15){
    BTSerial.print("\t좋음");
    }
  else if(outDust>15 or outDust<=35){
    BTSerial.print("\t보통");
    }
  else if(outDust>35){
    BTSerial.print("\t나쁨");
    }
  }
  delay(600000);
  
  //온습도 센서
  BTSerial.print("온도 : "+temp+"℃");
  BTSerial.print("습도 : "+humi+"%");
  delay(600000);
  
  //창문 자동 작동
  if (inDust>outDust or hum<=60){//내부미세먼지가 높고 습도가 낮으면
    if(windowState==0){//창문이 닫혀있을때
      stepper.step(res);//창문열기 
      windowState=1;
    }
  }
  else if(inDust<outDust and hum>60){//외부미세먼지가 높거나 습도가 높으면
    if(windowState==1){//창문이 열려있을때
      stepper.step(-res);//창문닫기
      windowState=0;
    }
  }

  //방범
  if(cm<=){//초음파센서에 걸림
    BTSerial.print("침입!!!");
  }

  //설정
  if(BTSerial.available())  //BTSerial에 전송된 값이 있으면
  {
    byte data = BTSerial.read();//수신 받은 데이터 저장
    else if(data=="open"){//창문열기
      window=BTSerial.read();
      if(window==0){
        stepper.step(-res);//창문닫기
        windowState=0;
      }
      else if(window==1){
        stepper.step(res);//창문열기
        windowState=1;
      }
    }
    else if(data=="night"){
      if(night==0){//밤에 사용x
        if(hour()>=0 or hour<=7){
          if(windowState==1) stepper.step(-res);
        }
      }
    }
    else if(data=="out"){
      if(out==1){//외출
        if(distance<500) BTSerial.Write("침입!");//어플알림
      }
    }
    else if(data=="1"){
      res=2038;
    }
    else if(data=="2"){
      res=4076;
    }
    else if(data=="3"){
      res=6114;
    }
  }
}
float pulse2ugm3(unsigned long pulse){//pulse2ugm3함수 설정
  float value = (pulse - 1400) / 14.0;
  if(value > 300){
    value = 0;
  }
  return value; 
}
