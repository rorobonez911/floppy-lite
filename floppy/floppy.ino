struct Drive {
  int stepPin, dirPin;
  unsigned long lastStep, currentInterval, targetInterval;
  bool playing;
  int stepCount;
  byte held[10];
  int heldCount;
};

// Uses PWM pins on Arduino Mega also works on other models like the UNO
Drive d[6] = {
  {2,3,0,0,0,false,0,{0},0},    // Drive 1 (Channel 1)
  {4,5,0,0,0,false,0,{0},0},
  {6,7,0,0,0,false,0,{0},0},
  {8,9,0,0,0,false,0,{0},0},
  {10,11,0,0,0,false,0,{0},0},
  {12,13,0,0,0,false,0,{0},0}   // Drive 6 (Channel 6)
};

// home pulse sends drive to a starting position but its kinda buggy still works
const int STEPS_PER_DIR = 80;
const int HOME_PULSES = 100;

unsigned long noteToInterval(byte note) {
  if (note > 83) note = 83;
  double f = 440.0 * pow(2.0, (note - 69) / 12.0);
  if (f > 400) f = 400;
  if (f < 25)  f = 25;
  return (unsigned long)(1000000.0 / f);
}

void stepOnce(Drive &x) {
  digitalWrite(x.stepPin, HIGH);
  delayMicroseconds(30);
  digitalWrite(x.stepPin, LOW);
  if (++x.stepCount >= STEPS_PER_DIR) {
    x.stepCount = 0;
    digitalWrite(x.dirPin, !digitalRead(x.dirPin));
  }
}

void home(Drive &x) {
  digitalWrite(x.dirPin, LOW);
  for (int i=0;i<HOME_PULSES;i++) {
    digitalWrite(x.stepPin, HIGH); delayMicroseconds(700);
    digitalWrite(x.stepPin, LOW);  delayMicroseconds(700);
  }
  x.stepCount = 0;
}

void startupTone() {
  digitalWrite(3, LOW);
  unsigned long interval = noteToInterval(67);
  unsigned long last = micros();
  unsigned long endt = millis() + 1000;
  while (millis() < endt) {
    unsigned long now = micros();
    if (now - last >= interval) {
      last = now;
      digitalWrite(2, HIGH);
      delayMicroseconds(30);
      digitalWrite(2, LOW);
    }
  }
}

void noteOn(Drive &x, byte note) {
  if (x.heldCount < 10) x.held[x.heldCount++] = note;
  byte active = x.held[x.heldCount - 1];
  x.targetInterval = noteToInterval(active);
  if (!x.playing) x.currentInterval = x.targetInterval;
  x.playing = true;
}

void noteOff(Drive &x, byte note) {
  for (int i=0;i<x.heldCount;i++) {
    if (x.held[i] == note) {
      for (int j=i;j<x.heldCount-1;j++) x.held[j] = x.held[j+1];
      x.heldCount--;
      break;
    }
  }
  if (x.heldCount == 0) { x.playing = false; return; }
  byte active = x.held[x.heldCount - 1];
  x.targetInterval = noteToInterval(active);
}

void setup() {
  Serial.begin(115200);
  for (int i=0;i<6;i++) {
    pinMode(d[i].stepPin, OUTPUT);
    pinMode(d[i].dirPin, OUTPUT);
    digitalWrite(d[i].stepPin, LOW);
    digitalWrite(d[i].dirPin, LOW);
  }
  for (int i=0;i<6;i++) home(d[i]);
  startupTone();
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

    if (x.currentInterval != x.targetInterval) {
      long diff = (long)x.targetInterval - (long)x.currentInterval;
      long step = diff / 20;
      if (step == 0) step = (diff > 0 ? 1 : -1);
      x.currentInterval += step;
    }

    if (now - x.lastStep >= x.currentInterval) {
      x.lastStep = now;
      stepOnce(x);
    }
  }
}