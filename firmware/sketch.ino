#include <AESLib.h>
#include <sha256.h>

#include <Wire.h>
#include "Adafruit_FRAM_I2C.h"

#define ACC_I 32671
uint8_t acc_num;

const int buttonPin = 4;

String inData;
char* delimiter = " ";

int iters = 0;

Adafruit_FRAM_I2C fram = Adafruit_FRAM_I2C();
uint16_t framAddr = 0; 

uint8_t* aes_key;

void userHash(const char *str) {
  int h = 0;
  while (*str)
    h = h << 1 ^ *str++;
  h = h % 50 + 50;
  iters = h;
}

void printHash(uint8_t* hash) {
  for (int i=0; i<32; i++) {
    Serial.print("0123456789abcdef"[hash[i]>>4]);
    Serial.print("0123456789abcdef"[hash[i]&0xf]);
  }
  Serial.println();
}

uint8_t* kdf(char* pwd) {
  Serial.println("Deriving authentication hash...");
  uint8_t salt[32];
  for (uint16_t i = 32672; i < 32704; i++) {
    salt[i - 32672] = fram.read8(i);
  }
  Sha256.initHmac(salt, 32);
  Sha256.print(pwd);
  uint8_t* auth_hash = Sha256.resultHmac();

  for (int i = 0; i < iters; i++) {
    const uint8_t* temp = auth_hash;
    Sha256.initHmac(temp, 32);
    Sha256.print(pwd);
    auth_hash = Sha256.resultHmac();
  }

  // Write pass hash to FRAM
  //  for (uint16_t i = 32704; i < 32768; i++) {
  //    fram.write8(i, auth_hash[i - 32704]);
  //  }

  // Writing salt to FRAM
  //  uint8_t some_salt[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
  //
  //  for (uint16_t i = 32672; i < 32704; i++) {
  //    fram.write8(i, some_salt[i - 32672]);
  //  }

  //  fram.write8(ACC_I, 0);

  return auth_hash;
}

void getKey(const uint8_t* auth_hash, char* pwd) {
  Sha256.initHmac(auth_hash, 32);
  Sha256.print(pwd);
  aes_key = Sha256.result();
  //  printHash(aes_key);
}

void auth(char* user, char* pass) {
  uint8_t* auth_hash = kdf(pass);

  byte isAuth = 0;

  uint8_t stored_hash[64];
  for (uint16_t i = 32704; i < 32768; i++) { 
    stored_hash[i - 32704] = fram.read8(i);
  }

  //  printHash(stored_hash);
  //  printHash(auth_hash);

  isAuth = 1;
  for (byte i = 0; i < 64; i++) {
    if (auth_hash[i] != stored_hash[i]) {
      isAuth = 0;
      break;
    }
  }

  if (isAuth == 1) {
    Serial.println("AUTH-SUCCESS");
    Serial.println("Keeping AES key...");
    getKey(auth_hash, pass);
  } else {
    Serial.println("AUTH-FAIL");
  }
}

void getAccNum() {
  Serial.print("NUM ");
  Serial.println(acc_num);
  delay(100);
}

void saveAcc(int id, char* type, char* user, char* pass) {
  int offset = id*96;

  aes256_enc_single(aes_key, type);

  aes256_enc_single(aes_key, user);

  aes256_enc_single(aes_key, pass);

  for (uint16_t i = offset; i < offset+32; i++) {
    fram.write8(i, (uint8_t)type[i - offset]);
  }
  for (uint16_t i = offset+32; i < offset+64; i++) {
    fram.write8(i, (uint8_t)user[i-offset-32]);
  }
  for (uint16_t i = offset+64; i < offset+96; i++) {
    fram.write8(i, (uint8_t)pass[i-offset-64]);
  }

  acc_num++;
  fram.write8(ACC_I, acc_num);
}

void getAcc(int id) {
  int offset = id*96;

  uint8_t* u = (uint8_t*) malloc(32);
  uint8_t* p = (uint8_t*) malloc(32);
  uint8_t* t = (uint8_t*) malloc(32);

  for (uint16_t i = offset; i < offset+32; i++) {
    t[i-offset] = fram.read8(i);
  }
  for (uint16_t i = offset+32; i < offset+64; i++) {
    u[i-offset-32] = fram.read8(i);
  }
  for (uint16_t i = offset+64; i < offset+96; i++) {
    p[i-offset-64] = fram.read8(i);
  }

  char* type = (char*) malloc(32);
  char* user = (char*) malloc(32);
  char* pass = (char*) malloc(32);

  memcpy(type, t, 32);
  memcpy(user, u, 32);
  memcpy(pass, p, 32);

  aes256_dec_single(aes_key, type);
  aes256_dec_single(aes_key, user);
  aes256_dec_single(aes_key, pass);

  Serial.print(id);
  Serial.print(" ");
  Serial.print(type);
  Serial.print(" ");
  Serial.print(user);
  Serial.print(" ");
  Serial.println(pass);

  free(t);
  free(u);
  free(p);
  free(type);
  free(user);
  free(pass);
}

void updateAcc(int id, char* type, char* user, char* pass) {
  int offset = id*96;

  aes256_enc_single(aes_key, type);

  aes256_enc_single(aes_key, user);

  aes256_enc_single(aes_key, pass);

  for (uint16_t i = offset; i < offset+32; i++) {
    fram.write8(i, (uint8_t)type[i - offset]);
  }
  for (uint16_t i = offset+32; i < offset+64; i++) {
    fram.write8(i, (uint8_t)user[i-offset-32]);
  }
  for (uint16_t i = offset+64; i < offset+96; i++) {
    fram.write8(i, (uint8_t)pass[i-offset-64]);
  }
}

void deleteAcc(int id) {
  Serial.println("Start delete");
  int offset = id*96;

  for (uint16_t i = offset; i < offset+32; i++) {
    fram.write8(i, 0);
  }
  for (uint16_t i = offset+32; i < offset+64; i++) {
    fram.write8(i, 0);
  }
  for (uint16_t i = offset+64; i < offset+96; i++) {
    fram.write8(i, 0);
  }

  Serial.println("Deleted acc");

  for (uint16_t i = offset; i < acc_num*96; i++) {
    uint8_t temp = fram.read8(i + 95);
    fram.write8(i, temp);
  }

  Serial.println("shift others");

  acc_num--;
  fram.write8(ACC_I, acc_num);
}

void login(int id) {
  int offset = id*96;

  uint8_t* u = (uint8_t*) malloc(32);
  uint8_t* p = (uint8_t*) malloc(32);
  uint8_t* t = (uint8_t*) malloc(32);

  for (uint16_t i = offset; i < offset+32; i++) {
    t[i-offset] = fram.read8(i);
  }
  for (uint16_t i = offset+32; i < offset+64; i++) {
    u[i-offset-32] = fram.read8(i);
  }
  for (uint16_t i = offset+64; i < offset+96; i++) {
    p[i-offset-64] = fram.read8(i);
  }

  char* type = (char*) malloc(32);
  char* user = (char*) malloc(32);
  char* pass = (char*) malloc(32);

  memcpy(type, t, 32);
  memcpy(user, u, 32);
  memcpy(pass, p, 32);

  aes256_dec_single(aes_key, type);
  aes256_dec_single(aes_key, user);
  aes256_dec_single(aes_key, pass);

  free(t);
  free(u);
  free(p);

  Serial.println("Press the button to login...");
  int buttonState = LOW;
  int ledState = HIGH;

  while(buttonState == LOW) {
    digitalWrite(7, ledState);
    delay(100);
    buttonState = digitalRead(buttonPin);
    ledState = !ledState;
  }

  digitalWrite(7, LOW);

  Keyboard.print(user);
  Keyboard.print("\t");
  Keyboard.println(pass);

  free(type);
  free(user);
  free(pass);
}

void readS() {
  char input[128];
  input[0] = '\0';

  while (Serial.available() > 0) {
    char received = Serial.read();
    inData += received; 

    if (received == '\n') {
      inData.replace("\n", "");
      inData.toCharArray(input, 128);
      inData = ""; 
    }
  }

  char* command = strtok(input, delimiter);
  if (strcmp(command, "handshake") == 0) {
    Serial.println("Welcome to Passworks");
    delay(100);
  } else if (strcmp(command, "auth") == 0) {
    char* user;
    char* pass;

    user = strtok(0, delimiter);
    delay(100);

    pass = strtok(0, delimiter);
    delay(100);

    userHash(user);

    auth(user, pass);
  } else if (strcmp(command, "get") == 0) {
    int id = atoi(strtok(0, delimiter));
    getAcc(id);
  } else if (strcmp(command, "save") == 0) {
    int id = atoi(strtok(0, delimiter));

    char* type = strtok(0, delimiter);
    char* user = strtok(0, delimiter);
    char* pass = strtok(0, delimiter);

    saveAcc(id, type, user, pass);
  } else if (strcmp(command, "delete") == 0) {
    int id = atoi(strtok(0, delimiter));
    deleteAcc(id);
  } else if (strcmp(command, "num") == 0) {
    getAccNum();
  } else if (strcmp(command, "login") == 0) {
    int id = atoi(strtok(0, delimiter));
    login(id);
  } else if (strcmp(command, "update") == 0) {
    int id = atoi(strtok(0, delimiter));

    char* type = strtok(0, delimiter);
    char* user = strtok(0, delimiter);
    char* pass = strtok(0, delimiter);

    updateAcc(id, type, user, pass);
  }
}

void setup() {
  Serial.begin(9600);

  Keyboard.begin();

  if (fram.begin()) {
    Serial.println("FRAM found");
  }

  acc_num = fram.read8(ACC_I);

  pinMode(buttonPin, INPUT);
  pinMode(7, OUTPUT);
}

void loop() {
  while (!Serial.available());
  readS();
}
