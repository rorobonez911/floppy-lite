struct Drive {
  int stepPin, dirPin;
  unsigned long interval_us;
  unsigned long lastStep;
  int stepCount;
  byte held[10];
  int heldCount;
  bool playing;
};

Drive d[6] = {
  {2,3,0,0,0,{0},0,false},
  {4,5,0,0,0,{0},0,false},
  {6,7,0,0,0,{0},0,false},
  {8,9,0,0,0,{0},0,false},
  {10,11,0,0,0,{0},0,false},
  {12,13,0,0,0,{0},0,false}
};

unsigned long noteToInterval(byte n) {
  if (n > 83) n = 83;
  double f = 440.0 * pow(2.0, (n - 69) / 12.0);
  if (f > 400) f = 400;
  if (f < 25)  f = 25;
  return (unsigned long)(1000000.0 / f);
}

void stepOnce(Drive &x) {
  digitalWrite(x.stepPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(x.stepPin, LOW);
  if (++x.stepCount >= 80) {
    x.stepCount = 0;
    digitalWrite(x.dirPin, !digitalRead(x.dirPin));
  }
}

void fastHome(Drive &x) {
  digitalWrite(x.dirPin, LOW);
  for (int i=0;i<140;i++) {  
    digitalWrite(x.stepPin, HIGH); delayMicroseconds(500);
    digitalWrite(x.stepPin, LOW);  delayMicroseconds(500);
  }
  x.stepCount = 0;
  x.lastStep = micros();
}

void fastStartupAll() {
  double f = 440.0 * pow(2.0, (67 - 69) / 12.0);
  unsigned long interval_us = 1000000.0 / f;
  unsigned long last = micros();
  unsigned long endt = millis() + 400;  

  while (millis() < endt) {
    unsigned long now = micros();
    if (now - last >= interval_us) {
      last = now;
      for (int i=0;i<6;i++) {
        digitalWrite(d[i].stepPin, HIGH);
      }
      delayMicroseconds(10);
      for (int i=0;i<6;i++) {
        digitalWrite(d[i].stepPin, LOW);
      }
    }
  }
}

void noteOn(Drive &x, byte n) {
  if (x.heldCount < 10) x.held[x.heldCount++] = n;
  byte a = x.held[x.heldCount - 1];
  x.interval_us = noteToInterval(a);
  x.playing = true;
}

void noteOff(Drive &x, byte n) {
  for (int i=0;i<x.heldCount;i++) {
    if (x.held[i] == n) {
      for (int j=i;j<x.heldCount-1;j++) x.held[j] = x.held[j+1];
      x.heldCount--;
      break;
    }
  }
  if (x.heldCount == 0) { x.playing = false; return; }
  byte a = x.held[x.heldCount - 1];
  x.interval_us = noteToInterval(a);
}

void setup() {
  Serial.begin(115200);
  for (int i=0;i<6;i++) {
    pinMode(d[i].stepPin, OUTPUT);
    pinMode(d[i].dirPin, OUTPUT);
  }

  for (int i=0;i<6;i++) fastHome(d[i]);
  fastStartupAll();
}

void loop() {
  if (Serial.available() >= 3) {
    byte s = Serial.read();
    byte n = Serial.read();
    byte v = Serial.read();
    byte cmd = s & 0xF0;
    byte ch  = s & 0x0F;
    if (ch < 6) {
      if (cmd == 0x90 && v > 0) noteOn(d[ch], n);
      if (cmd == 0x80 || (cmd == 0x90 && v == 0)) noteOff(d[ch], n);
    }
  }

  unsigned long now = micros();
  for (int i=0;i<6;i++) {
    Drive &x = d[i];
    if (!x.playing) continue;
    if (now - x.lastStep >= x.interval_us) {
      x.lastStep = now;
      stepOnce(x);
    }
  }
}
