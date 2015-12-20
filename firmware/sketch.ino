#include <AESLib.h>
#include <sha256.h>

#include <Wire.h>
#include "Adafruit_FRAM_I2C.h"

Adafruit_FRAM_I2C fram = Adafruit_FRAM_I2C();
uint16_t framAddr = 0; 

String inData;

uint8_t salt[]={
  0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b
};

uint8_t* aes_key;

void printHash(uint8_t* hash) {
  for (int i=0; i<32; i++) {
    Serial.print("0123456789abcdef"[hash[i]>>4]);
    Serial.print("0123456789abcdef"[hash[i]&0xf]);
  }
  Serial.println();
}

uint8_t* kdf(char* pwd) {
  Sha256.initHmac(salt, 20);
  Sha256.print(pwd);
  uint8_t* auth_hash = Sha256.resultHmac();

  //  for (int i = 0; i < 5; i++) {
  //    Sha256.initHmac(auth_hash, 32);
  //    Sha256.print(pwd);
  //    auth_hash = Sha256.resultHmac();
  //  }

  // Write pass hash to FRAM
  //  for (uint16_t i = 32704; i < 32768; i++) {
  //    fram.write8(i, auth_hash[i - 32704]);
  //  }

  return auth_hash;

  //  char test[65];
  //  for (uint16_t i = 32704; i < 32768; i++) {
  //    test[i - 32704] = (char)fram.read8(i);
  //  }

  //  aes_key = (uint8_t*) malloc(32);
  //  memcpy(aes_key, auth_hash, 32);
  //
  //  Sha256.initHmac(auth_hash, 32);
  //  Sha256.print(pwd);
  //  aes_key = Sha256.resultHmac();
  //
  //  printHash(aes_key);
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

  getKey(auth_hash, pass);

  if (isAuth == 1) {
    Serial.println("AUTH-SUCCESS");
    delay(100);
  } else {
    Serial.println("AUTH-FAIL");
    delay(100);
  }
}

void getAcc(int id) {
  int offset = id*32;

  uint8_t* u = (uint8_t*) malloc(32);
  uint8_t* p = (uint8_t*) malloc(32);
  uint8_t* t = (uint8_t*) malloc(32);
  for (uint16_t i = offset; i < offset+32; i++) {
    t[i] = fram.read8(i);
  }
  for (uint16_t i = offset+32; i < offset+64; i++) {
    u[i-32] = fram.read8(i);
  }
  for (uint16_t i = offset+64; i < offset+96; i++) {
    p[i-64] = fram.read8(i);
  }

  char* type = (char*) malloc(32);
  char* user = (char*) malloc(32);
  char* pass = (char*) malloc(32);
  memcpy(type, t, 32);
  memcpy(user, u, 32);
  memcpy(pass, p, 32);
  aes256_dec_single(aes_key, user);
  aes256_dec_single(aes_key, pass);
  Serial.println(type);
  Serial.println(user);
  Serial.println(pass);

  free(t);
  free(u);
  free(p);
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

  char* command = strtok(input, "&");
  if (strcmp(command, "NG-INIT-HANDSHAKE") == 0) {
    Serial.println("PASSWORKS");
    delay(100);
  } else if (strcmp(command, "AUTH") == 0) {
    char* user;
    char* pass;

    user = strtok(0, "&");
    delay(100);

    pass = strtok(0, "&");
    delay(100);

    auth(user, pass);
  } else if (strcmp(command, "GET") == 0) {

    char* user = "aaaaaaaaaaaaaaaa";
    aes256_enc_single(aes_key, user);

    char* pass = "bbbbbbbbbbbbbbbb";
    aes256_enc_single(aes_key, pass);

    char* type = "cccccccccccccccc";

    for (uint16_t i = 0; i < 32; i++) {
      fram.write8(i, (uint8_t)type[i]);
    }
    for (uint16_t i = 32; i < 64; i++) {
      fram.write8(i, (uint8_t)user[i-32]);
    }
    for (uint16_t i = 64; i < 96; i++) {
      fram.write8(i, (uint8_t)pass[i-64]);
    }

    getAcc(0);
  }
}

void setup() {
  Serial.begin(9600);

  fram.begin();

  /*uint8_t key[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    char data[] = "0123456789012345"; //16 chars == 16 bytes
    aes128_enc_single(key, data);
    Serial.print("encrypted:");
    Serial.println(data);
    aes128_dec_single(key, data);
    Serial.print("decrypted:");
    Serial.println(data);

    unsigned long start = millis();

    kdf("masterpass");

    unsigned long end = millis();

    printHash(aes_key);
    printHash(auth_hash);

    free(aes_key);

    Serial.print("Time elapsed: ");
    Serial.println((end - start) / 1000.0);*/
}

void loop() {
  while (!Serial.available());
  readS();
}
