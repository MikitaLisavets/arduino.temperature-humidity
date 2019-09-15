#include <dht11.h>
#define PIN_BTN A0
#define PIN_DATA A1
#define PIN_POWER A2

dht11 DHT;

int anodPins[] = {7, 4, 3, 6};
int segmentsPins[] = {9, 2, 15, 10, 16, 8, 5, 14};


int state;

unsigned long ms_mode = 0;
unsigned long ms_dht = 0;

bool manualMode = false;
bool btnState = false;
bool revers = false;


byte seg[15] = {
  B11111100,
  B00001100,
  B11011010,
  B10011110,
  B00101110,
  B10110110,
  B11110110,
  B00011100,
  B11111110,
  B10111110,
  B11110001,
  B11100010,
  B00100100,
  B01100110,
  B00000000
};

byte seg_reverse[15] = {
  B11111100,
  B01100000,
  B11011010,
  B11110010,
  B01100110,
  B10110110,
  B10111110,
  B11100000,
  B11111110,
  B11110110,
  B10011100,
  B00011110,
  B00100100,
  B00101110,
  B00000000
};


int err[4] = {
  B00000010,
  B00000010,
  B00000010,
  B00000010,
};

static byte arr[4];

int mode = 0; 
int changemodeperiod = 2000;
int refreshdataperiod = 2000;

void setup() {
  pinMode(PIN_BTN, INPUT_PULLUP);
  pinMode(PIN_POWER, OUTPUT);
  digitalWrite(PIN_POWER, HIGH);
  for (int i = 0; i < 4; i++) pinMode(anodPins[i], OUTPUT);
  for (int i = 0; i < 8; i++) pinMode(segmentsPins[i], OUTPUT);

  ms_mode = millis();
  ms_dht = millis();

  if (digitalRead(PIN_BTN) == LOW) {
    revers = true;
    for (int i = 0; i < 15; i++) seg[i] = seg_reverse[i];
  }

  state = DHT.read(PIN_DATA);
  getDigits();
  while(digitalRead(PIN_BTN) == LOW) {};
  Serial.begin(9600);
}

void loop() {
  if (!manualMode) manualMode = !digitalRead(PIN_BTN);

  if (!manualMode) {
    if ((millis() - ms_mode) > changemodeperiod) {
      ms_mode = millis();
      mode = mode == 1 ? 0 : 1;
  
      if ((millis() - ms_dht) > refreshdataperiod) {
        ms_dht = millis();
        state = DHT.read(PIN_DATA);
      }
      getDigits();
    }
    displayMessage(arr);
  } else {
    while(true) {
      if (!btnState && digitalRead(PIN_BTN) == LOW) {
        btnState = true;
        mode = mode == 1 ? 0 : 1;
      }
      if (btnState && digitalRead(PIN_BTN) == HIGH) {
        btnState = false;
      }

      if ((millis() - ms_dht) > refreshdataperiod) {
        ms_dht = millis();
        state = DHT.read(PIN_DATA);
      }
      getDigits();
      displayMessage(arr);
    }
  }
}

boolean array_cmp(byte *a, byte *b, int l){
     byte n;
     for (n = 0; n < l; n++) if (a[n]!=b[n]) return false;
     return true;
}

void getDigits() {
  if (state == DHTLIB_OK) {
    if (mode == 0) {
      arr[0] = seg[11]; arr[3] = seg[10];
      arr[2] = seg[DHT.temperature % 10];
      if (DHT.temperature > 9)
        arr[1] = seg[(int)(DHT.temperature / 10)];
      else arr[1] = seg[14];
      arr[0] = seg[11]; arr[3] = seg[10];
    }
    else if (mode == 1) {
      arr[0] = seg[11]; arr[3] = seg[10];
      arr[2] = seg[DHT.humidity % 10];
      if (DHT.humidity > 9)
        arr[1] = seg[(int)(DHT.humidity / 10)];
      else arr[1] = seg[14];
      arr[0] = seg[13]; arr[3] = seg[12];
    }
  }
  else {
    for (int i = 0; i < 4; i++)
      arr[i] = err[i];
  }

}

void displayMessage(byte dig[4]) {
  if (revers) {
   for (int i = 0; i < 4; i++) {
      for (int k = 0; k < 8; k++) {
        digitalWrite(segmentsPins[k], ((bitRead(dig[i], 7 - k) == 1) ? HIGH : LOW));
      }
      digitalWrite(anodPins[i], LOW);
      delay(1);
      digitalWrite(anodPins[i], HIGH);
    } 
  } else {
   for (int i = 0; i < 4; i++) {
      for (int k = 0; k < 8; k++) {
        digitalWrite(segmentsPins[k], ((bitRead(dig[i], 7 - k) == 1) ? HIGH : LOW));
      }
      digitalWrite(anodPins[3 - i], LOW);
      delay(1);
      digitalWrite(anodPins[3 - i], HIGH);
    } 
  }
}
