#include <AESLib.h>
#include <sha256.h>

#include <Wire.h>
#include "Adafruit_FRAM_I2C.h"

#define INPUT_SIZE 1024

Adafruit_FRAM_I2C fram = Adafruit_FRAM_I2C();
uint16_t framAddr = 0;

char user[33];
char pass[33];

byte isAuth = 0;

uint8_t salt[] = {
  0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b
};

uint8_t* aes_key;
uint8_t* auth_hash;

void kdf(char* pwd) {
  Sha256.initHmac(salt, 20);
  Sha256.print(pwd);
  auth_hash = Sha256.resultHmac();

  for (int i = 0; i < 98; i++) {
    Sha256.initHmac(auth_hash, 256);
    Sha256.print(pwd);
    auth_hash = Sha256.resultHmac();
  }

  aes_key = (uint8_t*) malloc(32);
  memcpy(aes_key, auth_hash, 32);

  Sha256.initHmac(auth_hash, 32);
  Sha256.print(pwd);
  uint8_t* auth_hash = Sha256.resultHmac();
}

void printHash(uint8_t* hash) {
  for (int i=0; i<32; i++) {
    Serial.print("0123456789abcdef"[hash[i]>>4]);
    Serial.print("0123456789abcdef"[hash[i]&0xf]);
  }
  Serial.println();
}

void auth() {
  char u[33];
  for (uint16_t i = 32704; i < 32736; i++) {
    u[i-32704] = (char)fram.read8(i);
  }
  u[32] = '\0';

  char p[33];
  for (uint16_t i = 32736; i < 32768; i++) {
    p[i-32736] = (char)fram.read8(i);
  }
  p[32] = '\0';

  if ((strcmp(user, u) == 0) && (strcmp(pass, p) == 0)) {
    isAuth = 1;
    Serial.println("AUTH-SUCCESS");
    delay(100);
  }
}

void readS() {
  char in[INPUT_SIZE + 1];
  byte size = Serial.readBytes(in, INPUT_SIZE);
  in[size] = 0;

  char* command = strtok(in, "&");
  delay(100);
  if (strcmp(command, "NG-INIT-HANDSHAKE") == 0) {
    Serial.println("PASSWORKS");
    delay(100);
  } else if (strcmp(command, "AUTH") == 0) {
    command = strtok(0, "&");
    strcpy(user, command);
    user[32] = '\0';
    delay(100);

    command = strtok(0, "&");
    strcpy(pass, command);
    pass[32] = '\0';
    delay(100);

    auth();
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
  while(!Serial.available());
  readS();
}
