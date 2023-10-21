#include <SoftwareSerial.h>
SoftwareSerial BTSerial(53, 52);  //TX:52, RX:52

#include <ezButton.h>
ezButton toggleSwitch(2);

int bottomUsoundEchoPin = 49;  //바닥 초음파센서 Echo
int bottomUsoundTrigPin = 48;  //바닥 초음파센서 Trig
int leftUsoundEchoPin = 45;    //왼쪽 초음파센서 Echo
int leftUsoundTrigPin = 44;    //왼쪽 초음파센서 Trig
int midUsoundEchoPin = 47;    //가운데 초음파센서 Echo
int midUsoundTrigPin = 46;    //가운데 초음파센서 Trig
int rightUsoundEchoPin = 51;  //오른쪽 초음파센서 Echo
int rightUsoundTrigPin = 50;  //오른쪽 초음파센서 Trig

int ledPin = 12;              //고휘도 LED
int vibrationMotorPin = 3;    //진동 모터
int speakerPin = 13;          //스피커
int leftMoterIn1Pin = 4;      //왼쪽 모터 1
int leftMoterIn2Pin = 6;      //왼쪽 모터 2
int rightMoterIn1Pin = 8;     //오른쪽 모터 1
int rightMoterIn2Pin = 9;     //오른쪽 모터 2

float bottomDuration, bottomDistance; //바닥 초음파센서 측정 값 저장
float leftDuration, leftDistance;   //왼쪽 초음파센서 측정 값 저장
float midDuration, midDistance;     //가운데 초음파센서 측정 값 저장
float rightDuration, rightDistance;   //오른쪽 초음파센서 측정 값 저장

float min;  //왼쪽, 가운데, 오른쪽 중 장애물과 가장 가까운 거리 저장
int flag; //가장 가까운 거리가 몇 단계에 해당하는지 판별
char directionFlag;  //장애물 방향 저장
boolean stairFlag = false;  //계단을 인식한 상태인지 저장

int forceValue = 0; //압력센서 값 저장

int cdsValue = 0; //조도센서 값 저장

boolean grab = false; //사용자가 손잡이를 잡았었는지 저장
boolean miss = false; //사용자가 손잡이를 놓쳤는지 저장

unsigned long prev_time = 0;  //이전 시간 저장(조건에 따른 이벤트)
unsigned long prevPowerSaving_time = 0;  //이전 시간 저장(절전모드)

void setup()
{
  Serial.begin(9600);
  BTSerial.begin(9600);
  toggleSwitch.setDebounceTime(50);
  pinMode(bottomUsoundTrigPin, OUTPUT);
  pinMode(bottomUsoundEchoPin, INPUT);
  pinMode(leftUsoundTrigPin, OUTPUT);
  pinMode(leftUsoundEchoPin, INPUT);
  pinMode(midUsoundTrigPin, OUTPUT);
  pinMode(midUsoundEchoPin, INPUT);
  pinMode(rightUsoundTrigPin, OUTPUT);
  pinMode(rightUsoundEchoPin, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(leftMoterIn1Pin, OUTPUT);
  pinMode(leftMoterIn2Pin, OUTPUT);
  pinMode(rightMoterIn1Pin, OUTPUT);
  pinMode(rightMoterIn2Pin, OUTPUT);

  digitalWrite(leftMoterIn1Pin, LOW);
  digitalWrite(leftMoterIn2Pin, LOW);
  digitalWrite(rightMoterIn1Pin, LOW);
  digitalWrite(rightMoterIn2Pin, LOW);
}

void loop()
{

  toggleSwitch.loop();
  int state = toggleSwitch.getState();


  if (state == LOW) {
    if (BTSerial.available()) {
      Serial.write(BTSerial.read());
    }

    if (Serial.available()) {
      BTSerial.write(Serial.read());
    }

    unsigned long current_time = millis();

    forceValueIn();  //압력센서 값 받아오기

    //사용자가 손잡이를 잡고 있는지 검사
    if (forceValue >= 100) {

      if (grab) noTone(speakerPin); //스피커 작동 종료

      grab = true;
      miss = false;

      ultrasound(); //주변 장애물 감지
      stairInspection(); //계단 감지

      Serial.println(flag);

      //장애물 위치에 따른 이벤트 처리
      if (flag == 1) { //계단

        Serial.println("계단감지");
        stairFlag = true;
        analogWrite(vibrationMotorPin, 255);  //진동모터 작동

        //역방향 브레이크 작동
        digitalWrite(leftMoterIn1Pin, HIGH);
        digitalWrite(leftMoterIn2Pin, LOW);
        digitalWrite(rightMoterIn1Pin, HIGH);
        digitalWrite(rightMoterIn2Pin, LOW);

        bluttothOut(); //스마트폰으로 데이터 전달
      } else if (flag == 2) { //10cm 이하
        analogWrite(vibrationMotorPin, 255);  //진동모터 작동
        bluttothOut(); //스마트폰으로 데이터 전달
      } else if (flag == 3 && current_time - prev_time > 500) { //20cm 이하
        prev_time = current_time;
        analogWrite(vibrationMotorPin, 255);  //진동모터 작동
        if ((current_time % 1000) == 0) analogWrite(vibrationMotorPin, 0); //진동모터 작동 종료
        bluttothOut(); //스마트폰으로 데이터 전달
      } else if (flag == 4 && current_time - prev_time > 1000) { //30cm 이하
        prev_time = current_time;
        analogWrite(vibrationMotorPin, 255);  //진동모터 작동
        if ((current_time % 1000) == 0) analogWrite(vibrationMotorPin, 0); //진동모터 작동 종료
        bluttothOut(); //스마트폰으로 데이터 전달
      } else if (flag == 5 && current_time - prev_time > 1500) { //50cm 이하
        prev_time = current_time;
        analogWrite(vibrationMotorPin, 255);  //진동모터 작동
        if ((current_time % 1000) == 0) analogWrite(vibrationMotorPin, 0); //진동모터 작동 종료
        bluttothOut(); //스마트폰으로 데이터 전달
      } else if (flag == 6 && current_time - prev_time > 2000) { //100cm 이하
        prev_time = current_time;
        analogWrite(vibrationMotorPin, 255);  //진동모터 작동
        if ((current_time % 1000) == 0) analogWrite(vibrationMotorPin, 0); //진동모터 작동 종료
      } else if (flag == 7 && current_time - prev_time > 3000) { //200cm 이하
        prev_time = current_time;
        analogWrite(vibrationMotorPin, 255);  //진동모터 작동
        if ((current_time % 1000) == 0) analogWrite(vibrationMotorPin, 0); //진동모터 작동 종료
      } else if (flag == 8 && current_time - prev_time > 4000) { //300cm 이하
        prev_time = current_time;
        analogWrite(vibrationMotorPin, 255);  //진동모터 작동
        if ((current_time % 1000) == 0) analogWrite(vibrationMotorPin, 0); //진동모터 작동 종료
      } else if (flag == 9 && current_time - prev_time > 5000) { //400cm 이하
        prev_time = current_time;
        analogWrite(vibrationMotorPin, 255);  //진동모터 작동
        if ((current_time % 1000) == 0) analogWrite(vibrationMotorPin, 0); //진동모터 작동 종료
      } else {
        stopDC(); //DC모터 작동 종료
      }

      cdsValueIn(); //조도센서 값 받아오기

      //밝기에 따른 이벤트 처리
      if (cdsValue >= 800) {
        analogWrite(ledPin, 255); //고휘도 LED 점등
      } else {
        analogWrite(ledPin, 0); //고휘도 LED 소등
      }
    } else {
      //손잡이를 놓쳤는지 검사
      if (grab && !miss) {
        miss = true;
        tone(speakerPin, 500);  //스피커 작동
        prevPowerSaving_time = current_time;
      }
      analogWrite(vibrationMotorPin, 0);  //진동모터 작동 종료
      stopDC(); //DC모터 작동 종료
      analogWrite(ledPin, 0); //고휘도 LED 소등

      if (current_time - prevPowerSaving_time > 180000) {
        noTone(speakerPin);  //스피커 작동 종료
      }
    }
  }

  if (toggleSwitch.isReleased()) {
    Serial.println("The switch: ON -> OFF");

    analogWrite(vibrationMotorPin, 0);  //진동모터 작동 종료
    stopDC(); //DC모터 작동 종료
    analogWrite(ledPin, 0); //고휘도 LED 소등
    noTone(speakerPin);  //스피커 작동 종료

    grab = false;
  }

}

void ultrasound()
{
  //왼쪽 초음파 센서 값 받아오기
  digitalWrite(leftUsoundTrigPin, HIGH);
  digitalWrite(leftUsoundTrigPin, LOW);
  leftDuration = pulseIn(leftUsoundEchoPin, HIGH);
  leftDistance = ((float)(340 * leftDuration) / 10000) / 2;
  Serial.print("leftDuration:");
  Serial.print(leftDuration);
  Serial.print("\nleftDIstance:");
  Serial.print(leftDistance);
  Serial.println("cm\n");


  //가운데 초음파 센서 값 받아오기
  digitalWrite(midUsoundTrigPin, HIGH);
  digitalWrite(midUsoundTrigPin, LOW);
  midDuration = pulseIn(midUsoundEchoPin, HIGH);
  midDistance = ((float)(340 * midDuration) / 10000) / 2;
  Serial.print("midDuration:");
  Serial.print(midDuration);
  Serial.print("\nmidDIstance:");
  Serial.print(midDistance);
  Serial.println("cm\n");


  //오른쪽 초음파 센서 값 받아오기
  digitalWrite(rightUsoundTrigPin, HIGH);
  digitalWrite(rightUsoundTrigPin, LOW);
  rightDuration = pulseIn(rightUsoundEchoPin, HIGH);
  rightDistance = ((float)(340 * rightDuration) / 10000) / 2;
  Serial.print("rightDuration:");
  Serial.print(rightDuration);
  Serial.print("\nrightDIstance:");
  Serial.print(rightDistance);
  Serial.println("cm\n");


  //가장 가까운 거리 판별
  if (leftDistance <= midDistance) {
    min = leftDistance;
    directionFlag = 'l';
  } else {
    min = midDistance;
    directionFlag = 'm';
  }
  if (rightDistance < min) {
    min = rightDistance;
    directionFlag = 'r';
  }
  Serial.println(directionFlag);

  //가장 가까운 거리의 해당 단계 판별
  if (min <= 10) flag = 2;
  else if (min <= 20) flag = 3;
  else if (min <= 30) flag = 4;
  else if (min <= 50) flag = 5;
  else if (min <= 100) flag = 6;
  else if (min <= 200) flag = 7;
  else if (min <= 300) flag = 8;
  else if (min <= 400) flag = 9;
  else flag = 0;
}

//계단 존재 유무를 확인하는 함수
void stairInspection()
{
  //바닥 초음파 센서 값 받아오기
  digitalWrite(bottomUsoundTrigPin, HIGH);
  digitalWrite(bottomUsoundTrigPin, LOW);
  bottomDuration = pulseIn(bottomUsoundEchoPin, HIGH);
  bottomDistance = ((float)(340 * bottomDuration) / 10000) / 2;
  Serial.print("bottomDuration:");
  Serial.print(bottomDuration);
  Serial.print("\nbottomDistance:");
  Serial.print(bottomDistance);
  Serial.println("cm\n");

  if (bottomDistance >= 10) {
    flag = 1;
    directionFlag = 's';
  } else {
    stopDC(); //DC모터 작동 종료
  }
}

//압력 센서 값 받아오는 함수
void forceValueIn()
{
  forceValue = analogRead(A5);

  Serial.print("Force= ");
  Serial.println(forceValue);
}


//조도 센서 값 받아오는 함수
void cdsValueIn()
{
  cdsValue = analogRead(A4);

  Serial.print("cds= ");
  Serial.println(cdsValue);
}

//DC모터 작동 종료
void stopDC()
{
  Serial.print("DC모터 작동 종료");
  digitalWrite(leftMoterIn1Pin, LOW);
  digitalWrite(leftMoterIn2Pin, LOW);
  digitalWrite(rightMoterIn1Pin, LOW);
  digitalWrite(rightMoterIn2Pin, LOW);
}

//스마트폰으로 데이터 전달하는 함수
void bluttothOut()
{
  BTSerial.print(flag); //위험 단계
  BTSerial.print("&");  //구분기호
  BTSerial.print(directionFlag); //위치(stair : s, left : l, mid : m, right : r)
}
