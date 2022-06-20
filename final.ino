#include <Keypad.h>
#include<Servo.h>
#include <Adafruit_Fingerprint.h>

//scale
#define Do 262
#define Re 294
#define Me 330
#define Fa 349
#define Sol 392
#define La 440
#define Si 494

//finger print
int finger_TX = 2;
int finger_RX = 4;
#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
SoftwareSerial mySerial(finger_TX, finger_RX);
#else
#define mySerial Serial1
#endif
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
//finger add
uint8_t id_Add = 'A';
uint8_t id_Del = 'D';


//speaker
int speak = 3;
int tempo = 200;

//keypad
const byte ROWS = 4;
const byte COLS = 4;

// 키패드를 누를 때 출력되는 값을 저장할 배열
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
 
byte rowPins[ROWS] = { 9, 8, 7, 6 }; // R1, R2, R3, R4의 차례로 R1~R4와 연결된 디지털 핀번호
byte colPins[COLS] = { 13, 12, 11, 10 }; // C1, C2, C3, C4의 차례로 C1~C4와 연결된 디지털 핀번호

//password
char pw[10] = {'0', '0', '0', '0'};
int pw_len = 4;
char tmp[10] = {};
int tmp_cnt = 0;
int check = 0;    // 모터 상태 0:잠김, 1:열림

// Keypad 라이브러리를 설정 (배열값, 행의 핀, 열의 핀, 행 갯수, 열 갯수)
Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

// Servo Moter set
Servo myservo;
int min_val = 615;  // 펄스폭 최소값 0도
int max_val = 2400; // 펄스폭 최대값 180도

//door senser
int doorState; //0 close , 1 open switch
 
void setup()
{
  myservo.attach(5, min_val, max_val);
  Serial.begin(9600);
  finger.begin(57600);
}
 
void loop()
{
  char key = kpd.getKey(); // 입력받은 키패드의 배열값을 key에 저장
  if (tmp_cnt >= 10) { tmp_cnt = 0; } // 10자리 입력이 넘어가면 자리수 초기화
  if (key) {
    switch(key) {
      case 'A': // 비번 세팅
        Serial.println(key);
        tone(speak, Do, tempo);
        delay(100);
        tone(speak, Sol, tempo);
        pw_change();
        Serial.println("success!!");
        Serial.print("newpassword : ");
        Serial.println(pw);
        tone(speak, Do, tempo);
        delay(100);
        tone(speak, Sol, tempo);
        break;
      case 'B': // 지문인식 ON
        Serial.println(key);
        tone(speak, Do, tempo);
        delay(200);
        tone(speak, Sol, tempo);
        for(int i = 0; i < 50; i++){
          char finger_ck = getFingerprintID();
          Serial.println(finger_ck);
          if(finger_ck == '1' || finger_ck == '2') { break; }
          delay(100);
        }
        break;
      case 'C': // 지문인식 등록
        Serial.println(key);
        tone(speak, Do, tempo);
        delay(200);
        tone(speak, Sol, tempo);
        while(1){
          char finger_set = kpd.getKey();
          id_Add = finger_set;
          if (id_Add == '#') { break; }
          if (id_Add == '0') { break; }
          if (id_Add == '1' || id_Add == '2' || id_Add == '3' || id_Add == '4' || id_Add == '5' || id_Add == '6' || id_Add == '7' || id_Add == '8' || id_Add == '9' ){
            getFingerprintEnroll();
            break;
          } 
        }
        break;
      case 'D': // 지문인식 삭제
        Serial.println(key);
        tone(speak, Do, tempo);
        delay(200);
        tone(speak, Sol, tempo);
        while(1){
          char finger_set = kpd.getKey();
          id_Del = finger_set;
          if (id_Del == '#') { break; }
          if (id_Del == '0') { break; }
          if (id_Del == '1' || id_Del == '2' || id_Del == '3' || id_Del == '4' || id_Del == '5' || id_Del == '6' || id_Del == '7' || id_Del == '8' || id_Del == '9' ){
            deleteFingerprint(id_Del);
            break;
          } 
        }
        break;
      case '*': // 숫자 확인
        Serial.println(key);
        tone(speak, Si, tempo);
        check = pw_cert();
        if(check == 1){ door_open(); }
        else { door_close(); }
        tmp_cnt = 0;
        break;
      case '#': // cancel or door close
        Serial.println(key);
        tone(speak, Si, tempo);
        tmp_cnt = 0;
        door_close();
        delay(20);
        break;
      default:
        Serial.println(key);
        tone(speak, Si, tempo);
        tmp[tmp_cnt] = key;
        tmp_cnt++;
        break;
    }
  }
}

void pw_change() {
  int cnt = 0;
  while(1){
    char cg = kpd.getKey();
    if(cg == '#') {
      pw_len = cnt;
      return;
    }
    if(cg) {
      pw[cnt] = cg;
      cnt++;
    }
  }
}

int pw_cert() {
  int cert_ck = 0;
  if (tmp_cnt != pw_len) { return 0; }
  for (int i = 0; i < tmp_cnt; i++) {
    if(tmp[i] != pw[i]) { break; }
    if((i+1) == tmp_cnt) { cert_ck = 1; }
  }
  if(cert_ck == 1) { return 1; }
  else {return 0;}
}

void door_open() {
  myservo.write(0); 
  delay(20);
  tone(speak, Do, tempo);
  delay(100);
  tone(speak, Me, tempo);
  delay(100);
  tone(speak, Sol, tempo);
  delay(100);
  tone(speak, Si, tempo);
  int tim_cnt = 0;
  doorState = analogRead(A0);
  Serial.println(doorState);
  while(tim_cnt <= 51 && doorState >= 990){
    doorState = analogRead(A0);
    Serial.println(doorState);
    tim_cnt++;
    delay(100);
    if(tim_cnt >= 50) { door_close(); }
  }
}

void door_close() {
  myservo.write(105); 
  delay(20);
  tone(speak, Si, tempo);
  delay(100);
  tone(speak, Sol, tempo);
  delay(100);
  tone(speak, Me, tempo);
  delay(100);
  tone(speak, Do, tempo);
}

// 지문 인식
uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return '2';
  } else {
    Serial.println("Unknown error");
    return p;
  }

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  door_open();
  return '1';
}

// 지문 등록
uint8_t getFingerprintEnroll() {

  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id_Add);
  tone(speak, Do, tempo);
  delay(100);
  tone(speak, Sol, tempo);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  Serial.println("Remove finger");
  tone(speak, Do, tempo);
  delay(100);
  tone(speak, Sol, tempo);
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id_Add);
  p = -1;
  Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id_Add);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  Serial.print("ID "); Serial.println(id_Add);
  p = finger.storeModel(id_Add);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
    tone(speak, Do, tempo);
    delay(100);
    tone(speak, Sol, tempo);
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  return true;
}

// 지문 삭제
uint8_t deleteFingerprint(uint8_t id) {
  uint8_t p = -1;

  p = finger.deleteModel(id_Del);

  if (p == FINGERPRINT_OK) {
    Serial.println("Deleted!");
    tone(speak, Sol, tempo);
    delay(100);
    tone(speak, Do, tempo);
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not delete in that location");
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
  } else {
    Serial.print("Unknown error: 0x"); Serial.println(p, HEX);
  }

  return p;
}
