/*
* Fil: Waterdrop.ino
* Creator: Lucas Norrflod
* Date: 6/2-2025
* This code simulate water in a 2d space with an accelerometer and displays it on an oled screen.
*/

//included libraries
#include "U8glib.h"
#include "Wire.h"

// ADXL345 I2C address is 0x53(83)
int ADXL345 = 0x53;  // The ADXL345 sensor I2C address

float X_out, Y_out, Z_out;   // Outputs
const int numofdrops = 60;   //The number of drops that will appear on screen
float xincline;              //incline recieved from ADXL345 sensor on the x axis
float yincline;              //incline recieved from ADXL345 sensor on the y axis
float drops[numofdrops][6];  // [Which drop][x-position; y-position; x-acceleration; y-acceleration; x-velocity; y-velocity]

const float maxAcc = 50;  //The highest acceleration that the waterdrops can recieve. An acceleration of 100 means increasing the velocity by 1.

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NO_ACK);  //instantiating the object

void setup() {
  Wire.begin();  // Initiates the Wire library

  // Set ADXL345 in measuring mode
  Wire.beginTransmission(ADXL345);  // Start communicating with the device
  Wire.write(0x2D);                 // Access/ talk to POWER_CTL Register - 0x2D
  // Enable measurement
  Wire.write(8);  // (8dec -> 0000 1000 binary) Bit D3 High for measuring enable
  Wire.endTransmission();
  delay(10);
  //This loop creates the starting position for each drop.
  for (int x = 0; x < numofdrops; x++) {
    drops[x][0] = x;
  }
  u8g.nextPage();
}

void loop() {
  // === Read acceleromter data === //
  Wire.beginTransmission(ADXL345);
  Wire.write(0x32);  // Start with register 0x32 (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(ADXL345, 6, true);        // Read 6 registers total, each axis value is stored in 2 registers
  X_out = (Wire.read() | Wire.read() << 8);  // X-axis value
  X_out = X_out / 256;                       //For a range of +-2g, we need to divide the raw values by 256, according to the datasheet
  Y_out = (Wire.read() | Wire.read() << 8);  // Y-axis value
  Y_out = Y_out / 256;
  Z_out = (Wire.read() | Wire.read() << 8);  // Z-axis value
  Z_out = Z_out / 256;


  xincline = X_out;  //Assigns the output from the accelerometer's x-axis
  yincline = Y_out;  //Assigns the output from the accelerometer's y-axis

  //This for-loop calculates all the values for every drop.
  for (int i = 0; i < numofdrops; i++) {
    drops[i][2] = map(xincline * 100, -93, 107, -maxAcc, maxAcc);  //Maps the incline value and assigns it to the x-acceration
    drops[i][3] = map(yincline * 100, -110, 90, maxAcc, -maxAcc);  //Maps the incline value and assigns it to the y-acceration

    //calculates drop velocity
    drops[i][4] += drops[i][2] / 100;
    drops[i][5] += drops[i][3] / 100;

    //Friction function
    drops[i][4] = friction(drops[i][4]);
    drops[i][5] = friction(drops[i][5]);

    //Räknar ut pixel koordinat
    drops[i][0] += drops[i][4];
    drops[i][1] += drops[i][5];

    //All these if-statements check if the posiyion of any drop is outside screen.
    if (drops[i][0] < 0) {
      drops[i][0] = 0;  //sets the x-position to 0
      drops[i][2] = 0;  //sets the x-velocity to 0
      drops[i][4] = 0;  //sets the x-acceleration to 0
    } else if (drops[i][0] > 123) {
      drops[i][0] = 123;  //sets the x-position to 123
      drops[i][2] = 0;    //sets the x-velocity to 0
      drops[i][4] = 0;    //sets the x-acceleration to 0
    }
    if (drops[i][1] < 0) {
      drops[i][1] = 0;  //sets the y-position to 0
      drops[i][3] = 0;  //sets the x-velocity to 0
      drops[i][5] = 0;  //sets the x-acceleration to 0
    } else if (drops[i][1] > 63) {
      drops[i][1] = 63;  //sets the y-position to 63
      drops[i][3] = 0;   //sets the x-velocity to 0
      drops[i][5] = 0;   //sets the x-acceleration to 0
    }
    delay(1);
  }

  //Runs a function which updates the drops array to not have any drop with the same position.
  drops[numofdrops][6] = samepos(drops);

  //Draws out the drops
  u8g.firstPage();
  do {
    for (int i = 0; i < numofdrops; i++) {
      u8g.drawPixel(drops[i][0], drops[i][1]);
    }
  } while (u8g.nextPage());
}

/*
*Creates a friction for the drop and returns the velocity
*Parameters: Velocity in float
*/
float friction(float Vel) {
  if (Vel < 0) { //Checks if velocity is negative
    Vel += 0.05; //reduces velocity
    if (Vel > 0) { //checks if velocity is reduced passed 0
      Vel = 0; 
    }
  } else {
    Vel -= 0.05; //reduces the velocity
    if (Vel < 0) { //checks if velocity is reduced passed 0
      Vel = 0;
    }
  }
  return Vel;
}


/*
*Checks if any drop has the same position and changes the position if they have.
*Parameter: An array with [Which drop][x-position; y-position; x-acceleration; y-acceleration; x-velocity; y-velocity]
*/
int samepos(float arr[numofdrops][6]) {
  for (int i = 0; i < numofdrops; i++) { //Goes through every drop
    for (int u = i + 1; u < numofdrops; u++) { //
      if (arr[u][0] == arr[i][0]) {
        if (arr[u][1] == arr[i][1]) {
          if (arr[u][0] >= 60) {    //The if-statement looks if the drop is in the right or left half of the screen
            if (arr[u][1] <= 30) {  //The if-statement looks if the drop is in the upper or lower half of the screen
              arr[u][1] += 1; //changes y-position
            } else {
              arr[u][1] -= 1; //changes y-position
            }
            arr[u][0] -= 1; //changes x-position
          } else {
            if (arr[u][1] <= 30) {  //The if-statement looks if the drop is in the upper or lower half of the screen
              arr[u][1] += 1; //changes y-position
            } else {
              arr[u][1] -= 1; //changes y-position
            }
            arr[u][0] += 1; //changes x-position
          }
          i = 0;
        }
      }
    }
  }
  return arr;
}
