#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

Adafruit_SH1106G display(128, 64, &Wire, -1);

const int BUTTON_PIN = 2;

// Timings
const unsigned long SLEEP_AFTER = 14000;
const unsigned long EVENT_AFTER = 14000;

const unsigned long ALERT_DURATION = 1800;
const unsigned long ANGRY_DURATION = 2000;
const unsigned long PANIC_DURATION = 1400;

const unsigned long SLEEP_EVENT_DURATION = 3800;
const unsigned long SNOT_GROW_TIME = 2200;
const unsigned long SNOT_HOLD_TIME = 500;
const unsigned long POP_DURATION = 450;

// Eye positions
const int LEFT_EYE_X = 32;
const int RIGHT_EYE_X = 96;
const int EYE_Y = 34;
const int EYE_W = 28;
const int EYE_H = 36;

// Face states
bool sleeping = false;
bool alertActive = false;
bool angryActive = false;
bool panicActive = false;
bool sleepEventActive = false;

bool alarmEvent = false;
bool snotEvent = false;

// Timers
unsigned long lastActivity = 0;
unsigned long reactionStart = 0;
unsigned long sleepStart = 0;
unsigned long eventStart = 0;
unsigned long nextLook = 0;
unsigned long nextBlink = 0;
unsigned long blinkStart = 0;

// Z animation
int zOffset = 0;

// Alternate sleep events
int eventFrame = 0;

// Button debounce
bool buttonReading = HIGH;
bool stableButtonState = HIGH;
unsigned long buttonChangedAt = 0;

// Eye movement
int pupilX = 0;
int pupilY = 0;
int targetX = 0;
int targetY = 0;
bool blinking = false;

// --------------------------------------------------
// SETUP
// --------------------------------------------------

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Wire.begin();

  if (!display.begin(0x3C, true)) {
    while (true) {
      delay(100);
    }
  }

  display.clearDisplay();
  display.display();

  randomSeed(analogRead(A0));

  unsigned long now = millis();
  lastActivity = now;
  nextBlink = now + random(2500, 5000);
  nextLook = now + random(900, 2200);
}

// --------------------------------------------------
// MAIN LOOP
// --------------------------------------------------

void loop() {
  unsigned long now = millis();

  checkButton(now);

  if (!sleeping && !alertActive && !angryActive &&
      !panicActive && !sleepEventActive &&
      now - lastActivity >= SLEEP_AFTER) {
    sleeping = true;
    sleepStart = now;
  }

  updateReaction(now);
  updateSleepEvent(now);

  if (!sleeping && !sleepEventActive) {
    updateEyes(now);
  }

  drawScene(now);
  delay(15);
}

// --------------------------------------------------
// BUTTON: ONE ACTION PER PRESS
// --------------------------------------------------

void checkButton(unsigned long now) {
  bool reading = digitalRead(BUTTON_PIN);

  if (reading != buttonReading) {
    buttonReading = reading;
    buttonChangedAt = now;
  }

  if (now - buttonChangedAt >= 35 &&
      stableButtonState != buttonReading) {

    stableButtonState = buttonReading;

    if (stableButtonState == LOW) {
      lastActivity = now;

      sleeping = false;
      sleepEventActive = false;
      alarmEvent = false;
      snotEvent = false;

      wakeRobot(now);
    }
  }
}

// --------------------------------------------------
// WAKE SEQUENCE
// --------------------------------------------------

void wakeRobot(unsigned long now) {
  sleeping = false;
  sleepEventActive = false;

  alertActive = true;
  angryActive = false;
  panicActive = false;

  reactionStart = now;

  targetX = random(-7, 8);
  targetY = random(-5, 6);
  pupilX = targetX;
  pupilY = targetY;

  nextLook = now;
  blinking = false;
}

void updateReaction(unsigned long now) {
  // Alert: frantic darting eyes
  if (alertActive) {
    if (now - reactionStart >= ALERT_DURATION) {
      alertActive = false;
      angryActive = true;
      reactionStart = now;

      targetX = 0;
      targetY = 0;
      pupilX = 0;
      pupilY = 0;
      blinking = false;
    }
    return;
  }

  // Angry: stay centered for the entire expression
  if (angryActive) {
    targetX = 0;
    targetY = 0;
    pupilX = 0;
    pupilY = 0;
    blinking = false;

    if (now - reactionStart >= ANGRY_DURATION) {
      angryActive = false;
      panicActive = true;
      reactionStart = now;
      nextLook = now;
    }
    return;
  }

  // Panic: existing darting behaviour
  if (panicActive &&
      now - reactionStart >= PANIC_DURATION) {
    panicActive = false;

    targetX = 0;
    targetY = 0;
    nextLook = now + random(900, 2200);
    lastActivity = now;
  }
}

// --------------------------------------------------
// EYE MOVEMENT
// --------------------------------------------------

void chooseTarget() {
  // Gentle default eye movement
  targetX = random(-5, 6);
  targetY = random(-3, 4);
}

void updateEyes(unsigned long now) {
  if (angryActive) {
    targetX = 0;
    targetY = 0;
    pupilX = 0;
    pupilY = 0;
    blinking = false;
    return;
  }

  // Frantic darting ONLY during the ! animation
  if (alertActive) {
    if (now >= nextLook) {
      targetX = random(-8, 9);
      targetY = random(-6, 7);
      nextLook = now + random(35, 71);
    }
  }

  // Preserve existing panic movement
  else if (panicActive) {
    if (now >= nextLook) {
      targetX = random(-7, 8);
      targetY = random(-5, 6);
      nextLook = now + random(70, 151);
    }
  }

  // Default: slow, gentle movement
  else {
    if (now >= nextLook) {
      chooseTarget();
      nextLook = now + random(900, 2200);
    }
  }

  // Natural blinking during the default animation.
  // Do not blink during alert or panic.
  if (!alertActive && !panicActive) {
    if (!blinking && now >= nextBlink) {
      blinking = true;
      blinkStart = now;
    }

    if (blinking && now - blinkStart >= 150) {
      blinking = false;
      nextBlink = now + random(2500, 5500);
    }
  }

  if (pupilX < targetX) pupilX++;
  if (pupilX > targetX) pupilX--;

  if (pupilY < targetY) pupilY++;
  if (pupilY > targetY) pupilY--;
}

// --------------------------------------------------
// SLEEP EVENT TIMING
// --------------------------------------------------

void updateSleepEvent(unsigned long now) {
  if (sleepEventActive) {
    if (now - eventStart >= SLEEP_EVENT_DURATION) {
      sleepEventActive = false;
      alarmEvent = false;
      snotEvent = false;

      wakeRobot(now);
    }
    return;
  }

  if (sleeping && now - sleepStart >= EVENT_AFTER) {
    sleeping = false;
    sleepEventActive = true;
    eventStart = now;

    if (eventFrame == 0) {
      alarmEvent = true;
      snotEvent = false;
      eventFrame = 1;
    }
    else {
      alarmEvent = false;
      snotEvent = true;
      eventFrame = 0;
    }
  }
}

// --------------------------------------------------
// DRAW SCENE
// --------------------------------------------------

void drawScene(unsigned long now) {
  display.clearDisplay();

  if (sleepEventActive) {
    drawSleepEvent(now);
  }
  else if (sleeping) {
    drawSleepEyes();
    drawZs(now);
  }
  else if (alertActive) {
    drawEyes(now, true, false);
    drawExclamation();
  }
  else if (angryActive) {
    drawEyes(now, false, true);
    drawAngryEyebrows();
  }
  else if (panicActive) {
    drawEyes(now, true, false);
  }
  else {
    drawEyes(now, false, false);
  }

  display.display();
}

// --------------------------------------------------
// EYES
// --------------------------------------------------

void drawEyes(unsigned long now, bool wideMovement, bool angry) {
  int eyeW = EYE_W;
  int eyeH = EYE_H;

  if (wideMovement) eyeH = 33;
  if (angry) eyeH = 30;

  int eyeY = EYE_Y - eyeH / 2;

  if (blinking && !wideMovement && !angry) {
    display.fillRoundRect(
      LEFT_EYE_X - eyeW / 2, EYE_Y - 1,
      eyeW, 3, 2, SH110X_WHITE
    );

    display.fillRoundRect(
      RIGHT_EYE_X - eyeW / 2, EYE_Y - 1,
      eyeW, 3, 2, SH110X_WHITE
    );
    return;
  }

  display.fillRoundRect(
    LEFT_EYE_X - eyeW / 2, eyeY,
    eyeW, eyeH, 8, SH110X_WHITE
  );

  display.fillRoundRect(
    RIGHT_EYE_X - eyeW / 2, eyeY,
    eyeW, eyeH, 8, SH110X_WHITE
  );

  int px = angry ? 0 : pupilX;
  int py = angry ? 0 : pupilY;

  int pupilW = 10;
  int pupilH = 14;

  int pupilCenterY = constrain(
    EYE_Y + py,
    eyeY + 8,
    eyeY + eyeH - 8
  );

  int leftCenterX = constrain(
    LEFT_EYE_X + px,
    LEFT_EYE_X - 5,
    LEFT_EYE_X + 5
  );

  int rightCenterX = constrain(
    RIGHT_EYE_X + px,
    RIGHT_EYE_X - 5,
    RIGHT_EYE_X + 5
  );

  display.fillRoundRect(
    leftCenterX - pupilW / 2,
    pupilCenterY - pupilH / 2,
    pupilW, pupilH, 4, SH110X_BLACK
  );

  display.fillRoundRect(
    rightCenterX - pupilW / 2,
    pupilCenterY - pupilH / 2,
    pupilW, pupilH, 4, SH110X_BLACK
  );
}

// --------------------------------------------------
// EXCLAMATION MARK
// --------------------------------------------------

void drawExclamation() {
  display.fillRoundRect(112, 5, 7, 24, 3, SH110X_WHITE);
  display.fillCircle(115, 35, 4, SH110X_WHITE);
}

// --------------------------------------------------
// ANGRY EYEBROWS
// --------------------------------------------------

void drawAngryEyebrows() {
  display.drawLine(15, 8, 45, 17, SH110X_WHITE);
  display.drawLine(15, 9, 45, 18, SH110X_WHITE);
  display.drawLine(15, 10, 45, 19, SH110X_WHITE);

  display.drawLine(83, 17, 113, 8, SH110X_WHITE);
  display.drawLine(83, 18, 113, 9, SH110X_WHITE);
  display.drawLine(83, 19, 113, 10, SH110X_WHITE);
}

// --------------------------------------------------
// SLEEPING EYES
// --------------------------------------------------

void drawSleepEyes() {
  display.fillRoundRect(
    LEFT_EYE_X - 14, EYE_Y,
    28, 4, 2, SH110X_WHITE
  );

  display.fillRoundRect(
    RIGHT_EYE_X - 14, EYE_Y,
    28, 4, 2, SH110X_WHITE
  );
}

// --------------------------------------------------
// FLOATING ZZZ: SLOWLY UP AND GRADUALLY BIGGER
// --------------------------------------------------

void drawZs(unsigned long now) {
  const unsigned long FLOAT_DURATION = 4500;

  unsigned long phase = now % FLOAT_DURATION;

  int y = 48 - (int)(phase * 34UL / FLOAT_DURATION);

  int size;

  if (phase < 1500) {
    size = 1;
  }
  else if (phase < 3000) {
    size = 2;
  }
  else {
    size = 3;
  }

  display.setTextColor(SH110X_WHITE);

  display.setTextSize(size);
  display.setCursor(84 - size * 2, y);
  display.print("Z");

  if (phase >= 700) {
    unsigned long followPhase = phase - 700;
    int secondY = 48 - (int)(followPhase * 34UL / FLOAT_DURATION);

    display.setTextSize(1);
    display.setCursor(101, secondY + 9);
    display.print("z");
  }

  if (phase >= 1400) {
    unsigned long followPhase = phase - 1400;
    int thirdY = 48 - (int)(followPhase * 34UL / FLOAT_DURATION);

    display.setTextSize(1);
    display.setCursor(72, thirdY + 14);
    display.print("z");
  }
}

// --------------------------------------------------
// SLEEP EVENT SELECTOR
// --------------------------------------------------

void drawSleepEvent(unsigned long now) {
  if (alarmEvent) {
    drawAlarmEvent(now);
  }
  else if (snotEvent) {
    drawSnotEvent(now);
  }
}

// --------------------------------------------------
// ALARM CLOCK: STREAKS + WAKE UP!
// --------------------------------------------------

void drawAlarmEvent(unsigned long now) {
  int bounce = ((now / 100) % 2 == 0) ? 0 : 2;

  display.drawCircle(64, 24 + bounce, 13, SH110X_WHITE);
  display.drawCircle(64, 24 + bounce, 12, SH110X_WHITE);

  display.drawLine(
    64, 24 + bounce, 64, 16 + bounce, SH110X_WHITE
  );

  display.drawLine(
    64, 24 + bounce, 71, 28 + bounce, SH110X_WHITE
  );

  display.drawLine(54, 12 + bounce, 49, 8 + bounce, SH110X_WHITE);
  display.drawLine(49, 8 + bounce, 46, 13 + bounce, SH110X_WHITE);

  display.drawLine(74, 12 + bounce, 79, 8 + bounce, SH110X_WHITE);
  display.drawLine(79, 8 + bounce, 82, 13 + bounce, SH110X_WHITE);

  if ((now / 100) % 2 == 0) {
    display.drawLine(42, 19, 34, 16, SH110X_WHITE);
    display.drawLine(41, 24, 31, 24, SH110X_WHITE);
    display.drawLine(43, 29, 35, 33, SH110X_WHITE);

    display.drawLine(86, 19, 94, 16, SH110X_WHITE);
    display.drawLine(87, 24, 97, 24, SH110X_WHITE);
    display.drawLine(85, 29, 93, 33, SH110X_WHITE);
  }
  else {
    display.drawLine(43, 18, 37, 13, SH110X_WHITE);
    display.drawLine(41, 26, 32, 29, SH110X_WHITE);

    display.drawLine(85, 18, 91, 13, SH110X_WHITE);
    display.drawLine(87, 26, 96, 29, SH110X_WHITE);
  }

  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1);
  display.setCursor(38, 45);
  display.print("WAKE UP!");
}

// --------------------------------------------------
// SNOT BUBBLE: GROW, PAUSE, BURST, THEN POP!
// --------------------------------------------------

void drawSnotEvent(unsigned long now) {
  unsigned long elapsed = now - eventStart;

  display.fillRoundRect(
    LEFT_EYE_X - 14, 22,
    28, 4, 2, SH110X_WHITE
  );

  display.fillRoundRect(
    RIGHT_EYE_X - 14, 22,
    28, 4, 2, SH110X_WHITE
  );

  int centerX = 55;
  int centerY = 40;

  if (elapsed < SNOT_GROW_TIME) {
    int radius = 2 + (int)(elapsed * 8UL / SNOT_GROW_TIME);

    if (radius > 10) radius = 10;

    display.fillCircle(
      centerX, centerY, radius, SH110X_WHITE
    );

    display.fillTriangle(
      centerX,
      centerY - radius,
      centerX + radius * 3,
      centerY,
      centerX,
      centerY + radius,
      SH110X_WHITE
    );

    if (radius >= 5) {
      display.drawPixel(
        centerX - 1, centerY - 2, SH110X_BLACK
      );
    }
  }
  else if (elapsed < SNOT_GROW_TIME + SNOT_HOLD_TIME) {
    display.fillCircle(55, 40, 10, SH110X_WHITE);

    display.fillTriangle(
      55, 30,
      85, 40,
      55, 50,
      SH110X_WHITE
    );
  }
  else {
    unsigned long popElapsed =
      elapsed - SNOT_GROW_TIME - SNOT_HOLD_TIME;

    if (popElapsed < POP_DURATION) {
      drawPopStreaks(centerX, centerY, popElapsed);
    }

    display.setTextColor(SH110X_WHITE);
    display.setTextSize(2);
    display.setCursor(37, 35);
    display.print("POP!");
  }
}

// --------------------------------------------------
// SNOT POP: OUTWARD-FLYING STREAKS
// --------------------------------------------------

void drawPopStreaks(int cx, int cy, unsigned long elapsed) {
  int length = 4 + (int)(elapsed / 45);

  if (length > 12) length = 12;

  display.drawLine(
    cx - 3, cy - 4,
    cx - 3 - length, cy - 4 - length,
    SH110X_WHITE
  );

  display.drawLine(
    cx, cy - 5,
    cx, cy - 5 - length,
    SH110X_WHITE
  );

  display.drawLine(
    cx + 4, cy - 4,
    cx + 4 + length, cy - 4 - length,
    SH110X_WHITE
  );

  display.drawLine(
    cx + 5, cy,
    cx + 5 + length, cy,
    SH110X_WHITE
  );

  display.drawLine(
    cx + 4, cy + 4,
    cx + 4 + length, cy + 4 + length,
    SH110X_WHITE
  );

  display.drawLine(
    cx, cy + 5,
    cx, cy + 5 + length,
    SH110X_WHITE
  );

  display.drawLine(
    cx - 4, cy + 4,
    cx - 4 - length, cy + 4 + length,
    SH110X_WHITE
  );

  display.drawLine(
    cx - 5, cy,
    cx - 5 - length, cy,
    SH110X_WHITE
  );
}
