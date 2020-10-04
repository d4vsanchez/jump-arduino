#include <LiquidCrystal.h>
#include <math.h>

short buttonState = LOW;
const short BALL_UPDATE_MS = 20;
const short GAME_UPDATE_MS = 500;
const unsigned short BUTTON_PIN = 7;

const short ROWS = 2;
const short COLUMNS = 16;
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

int nextUpdate = 0;
int nextBallUpdate = 0;
unsigned int inAirStart = 0;
const short USER_POSITION = 8192;
unsigned short currentObstacles = 1;
uint8_t obstacleBlock[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00100,
  B01110,
  B11111,
};
uint8_t ballBlock[8] = {
  B00000,
  B00000,
  B00000,
  B01110,
  B11111,
  B11111,
  B11111,
  B01110,
};
uint8_t deflatedBallBlock[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B01110,
  B11111,
  B11111,
};

unsigned short generateObstacles();
void paintObstacles();
bool isInAir();
bool hasLost();
void paintBall();
void gameOver();

unsigned short generateObstacles() {
  long randNumber = random(0, 9);
  unsigned short newNumber = currentObstacles << 1;

  if (randNumber < 3 && !(currentObstacles & 0x1)) {
    newNumber = newNumber | 0x1;
  }

  return newNumber;
}

void paintObstacles() {
  unsigned short printObstacles = currentObstacles;

  for (short i = 15; i >= 0; --i) {
    bool hasObstacle = printObstacles & 0x1;
    char printCharacter = hasObstacle ? (char) 0 : ' ';

    lcd.setCursor(i, 1);
    lcd.print(printCharacter);

    printObstacles = printObstacles >> 1;
  }
}

bool isInAir() {
  return inAirStart > 0 && (millis() - inAirStart) < (GAME_UPDATE_MS * 2);
}

bool hasLost() {
  return (USER_POSITION & currentObstacles) != 0 && !isInAir();
}

void paintBall() {
  short columnPosition = COLUMNS - (log10(USER_POSITION) / log10(2));
  lcd.setCursor(columnPosition - 1, 0);
  lcd.print(' ');

  if ((USER_POSITION & currentObstacles) == 0) {
    lcd.setCursor(columnPosition - 1, 1);
    lcd.print(' ');
  }

  lcd.setCursor(columnPosition - 1, !isInAir());
  char printCharacter = hasLost() ? (char) 2 : (char) 1;
  lcd.print(printCharacter);
}

void gameOver() {
  for (short i = 0; i < COLUMNS; i++) {
    lcd.setCursor(i, 0);
    lcd.print(' ');
  }

  lcd.setCursor(0, 0);
  lcd.print("GAME OVER");
}

void setup() {
  // To prevent using a potentiometer
  analogWrite(6, 75);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  lcd.begin(COLUMNS, ROWS);
  lcd.createChar(0, obstacleBlock);
  lcd.createChar(1, ballBlock);
  lcd.createChar(2, deflatedBallBlock);
}

void loop() {
  int ms = millis();

  if (ms >= nextUpdate && !hasLost()) {
    currentObstacles = generateObstacles();
    paintObstacles();
    nextUpdate = ms + GAME_UPDATE_MS;
  }

  if (ms >= nextBallUpdate) {
    paintBall();
    nextBallUpdate = ms + BALL_UPDATE_MS;
  }

  if (hasLost()) {
    gameOver();
  }

  buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == HIGH && !isInAir()) {
    inAirStart = ms;
  }
}
