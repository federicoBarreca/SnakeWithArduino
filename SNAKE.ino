// import the library that interfaces the led matrix
#include "LedControl.h"

// constants of the pushbutton pin
const int BTNUP = 12;     
const int BTNDOWN = 13;   
const int BTNLEFT = 10;   
const int BTNRIGHT = 11; 

// variable for reading the pushbutton status
int buttonStateRight = 0, buttonStateUp = 0, buttonStateLeft = 0, buttonStateDown = 0;  

// set the speed of the snake to a default value
int speed = 20000;

// initialize the object for the led matrix control
LedControl lc = LedControl(7, 6, 5, 1);

// initialize the vector positions of the snake
char posSerp[64] = { -1 };

// declare the variable for the length of the snake
char lenSerp;

// declare the fruit postion variable
char frutta;

// map the directions to constants
const char RIGHT = 0;
const char LEFT = 1;
const char UP = 2;
const char DOWN = 3;

// declare the variable for the current direction
char dir;

// declare the variables of the gameover management
char gameOver, flash;

// set the custom clock to 0
int clock = 0;

void setup() {
  // Led matrix initialization
  lc.shutdown(0, false);
  lc.setIntensity(0, 1);
  lc.clearDisplay(0);

  // initialize the pushbutton pin as an input
  pinMode(BTNUP, INPUT);
  pinMode(BTNDOWN, INPUT);
  pinMode(BTNLEFT, INPUT);
  pinMode(BTNRIGHT, INPUT);

  // initialize the random seed
  randomSeed(analogRead(0));

  // initialize the game settings
  snakeInit();

  // prepare the serial communication to port 9600
  Serial.begin(9600);
}


void loop() {

  //increase the clock value by one
  clock++;

  // check if the gameover occurred
  if (gameOver) {

    // read the state of the pushbutton value
    buttonStateRight = digitalRead(BTNRIGHT);
    buttonStateUp = digitalRead(BTNUP);
    buttonStateLeft = digitalRead(BTNLEFT);
    buttonStateDown = digitalRead(BTNDOWN);

    // set the new speed value according to the pushed button
    if (buttonStateLeft)
      speed = 20000;
    if (buttonStateUp)
      speed = 15000;
    if (buttonStateDown)
      speed = 10000;
    if (buttonStateRight)
      speed = 5000;

    // if a button was pressed then restart the game
    // compute a flashing animation of the snake's body otherwise
    if (buttonStateRight | buttonStateUp | buttonStateLeft | buttonStateDown) {
      snakeInit();
    } else {
      char tempPosX, tempPosY;
      for (int i = 0; i < lenSerp; i++) {
        getXY(posSerp[i], &tempPosX, &tempPosY);
        if (flash % 2 == 0) {
          lc.setLed(0, tempPosX, tempPosY, false);
        } else {
          lc.setLed(0, tempPosX, tempPosY, true);
        }
      }
      flash++;

      // delay that determines the speed of the flashing animation
      delay(500);
    }

  } else {

    // read the state of the pushbutton value:
    buttonStateRight = digitalRead(BTNRIGHT);
    buttonStateLeft = digitalRead(BTNLEFT);
    buttonStateUp = digitalRead(BTNUP);
    buttonStateDown = digitalRead(BTNDOWN);

    // if a button was pressed compute and set the new direction
    // don't change the direction only if the current one is the same or opposite of the chosen one
    if (buttonStateRight | buttonStateLeft | buttonStateUp | buttonStateDown) {

      if (buttonStateRight == HIGH && dir != LEFT) {
        dir = RIGHT;
      }
      if (buttonStateLeft == HIGH && dir != RIGHT) {
        dir = LEFT;
      }
      if (buttonStateUp == HIGH && dir != DOWN) {
        dir = UP;
      }
      if (buttonStateDown == HIGH && dir != UP) {
        dir = DOWN;
      }
    }

    // if the clock is higher than the speed then compute the animation
    if (clock > speed) {

      //compute the new snake's head position considering the possible overflow
      char headPosX, headPosY, tempPosX, tempPosY, headPos;
      getXY(posSerp[0], &headPosX, &headPosY);
      switch (dir) {
        case RIGHT:
          headPosX = (headPosX + 1) % 8;
          break;
        case DOWN:
          headPosY = (headPosY + 1) % 8;
          break;
        case LEFT:
          headPosX = !headPosX ? headPosX + 7 : headPosX - 1;
          break;
        case UP:
          headPosY = !headPosY ? headPosY + 7 : headPosY - 1;
          break;
      }

      // convert the cartesian coordinates of the head to the vector one
      headPos = getPos(headPosX, headPosY);

      // if the new head position is overlapping a part of the body of the snake then trigger gameover
      for (int i = 1; i < lenSerp; i++) {
        if (posSerp[i] == headPos) {
          gameOver = 1;
          break;
        }
      }

      if (!gameOver) {

        // make the positions of the snake slide toward the next one
        for (int i = lenSerp - 1; i >= 0; i--) {
          posSerp[i + 1] = posSerp[i];
        }

        // set the new position of the snake's head
        posSerp[0] = headPos;

        char fruttaMangiata = posSerp[0] == frutta;

        // check if the fruit was eaten by the snake
        if (fruttaMangiata) {

          // increase the length of the snake
          lenSerp++;

          // randomly generate a new fruit dot on the led matrix
          genFrutta();

          // notice: the previous fruit dot takes the place of the head of the snake and the length is increase by one
          // so is not necessary to turn on/off any led

        } else {
          //turn head led on
          lc.setLed(0, headPosX, headPosY, true);

          //turn tail led off
          getXY(posSerp[lenSerp], &tempPosX, &tempPosY);
          lc.setLed(0, tempPosX, tempPosY, false);
          posSerp[lenSerp] = -1;
        }
      }

      // reset the custom clock variable
      clock = 0;
    }
  }
}

// compute the cartesian coordinates from a vector position
void getXY(char pos, char* x, char* y) {
  *x = pos % 8;
  *y = pos / 8;
}

// compute the vector position from cartesian coordinates
char getPos(char x, char y) {
  return y * 8 + x;
}

// randomly generate a fruit dot and turn it on on the led matrix
void genFrutta() {
  frutta = random(64);
  // avoid any overlapping of the newly generated fruit with the snake's body
  for (int i = 1; i < lenSerp; i++) {
    if (frutta == posSerp[i]) {
      frutta = random(64);
      i = 1;
    }
  }

  // turn on the led corresponding to the fruit
  lc.setLed(0, frutta % 8, frutta / 8, true);
}

// initialize the game settings
void snakeInit() {

  // clear the matrix
  clearMatrix();

  // set the position of the snake's head randomly
  posSerp[0] = random(64);

  // turn on the led corresponding to the head of the snake
  lc.setLed(0, posSerp[0] % 8, posSerp[0] / 8, true);
  lenSerp = 1;

  // randomly generate a new fruit dot
  genFrutta();

  // set the default direction to RIGHT
  dir = RIGHT;

  // set the gameover variables and the clock to 0
  gameOver = 0;
  flash = 0;
  clock = 0;
}

// turn off each led of the matrix and reset the positions of the snake
void clearMatrix() {
  for (int i = 0; i < 64; i++) {
    posSerp[0] = -1;
    char tempPosX, tempPosY;
    getXY(i, &tempPosX, &tempPosY);
    lc.setLed(0, tempPosX, tempPosY, false);
  }
}