//included libraries
#include "U8glib.h"
#include "Wire.h"

// ADXL345 I2C address is 0x53(83)
int ADXL345 = 0x53;  // The ADXL345 sensor I2C address

float X_out, Y_out, Z_out;  // Outputs
const int numofdrops = 1;
int oldestVal = 1;
float xincline;
float yincline;
float xAcc;
float yAcc;
float xVel;
float yVel;
int dropspos[numofdrops*2];
const float maxAcc = 50;



U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NO_ACK);

//variables

void setup() {
  Serial.begin(9600);  // Initiate serial communication for printing the results on the Serial monitor
  Wire.begin();        // Initiate the Wire library
  // Set ADXL345 in measuring mode
  Wire.beginTransmission(ADXL345);  // Start communicating with the device
  Wire.write(0x2D);                 // Access/ talk to POWER_CTL Register - 0x2D
  // Enable measurement
  Wire.write(8);  // (8dec -> 0000 1000 binary) Bit D3 High for measuring enable
  Wire.endTransmission();
  delay(10);
  for (int x=0; x<numofdrops; x+=2){
    dropspos[x]=x;
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

  //Lägger in accelerometerns värden i en array
  xincline = X_out;
  yincline = Y_out;

  //mapar om medelvärdet av lutningens värde från accelerometern till acceleration i antal pixlar/sekund
  xAcc = map(xincline * 100, -93, 107, -maxAcc, maxAcc);
  yAcc = map(yincline * 100, -110, 90, maxAcc, -maxAcc);

  //Räknar ut pixel hastighet
  xVel += xAcc / 100;
  yVel += yAcc / 100;


  //friktion för vattnet
  xVel = friction(xVel);
  yVel = friction(yVel);


  //Räknar ut pixel koordinat
  dropspos[0] += xVel;
  dropspos[1] += yVel;

  if (dropspos[0] < 0) {
    dropspos[0] = 0;
    xAcc = 0;
    xVel = 0;
  }
  if (dropspos[0] > 123) {
    dropspos[0] = 123;
    xAcc = 0;
    xVel = 0;
  }
  if (dropspos[1] < 0) {
    dropspos[1] = 0;
    yAcc = 0;
    yVel = 0;
  }
  if (dropspos[1] > 63) {
    dropspos[1] = 63;
    yAcc = 0;
    yVel = 0;
  }

  //ritar ut pixeln
  u8g.firstPage();
  do {
    u8g.drawPixel(dropspos[0], dropspos[1]);
  } while (u8g.nextPage());
  delay(10);
}

//skapar friktion för droppen
float friction(float Vel) {
  if (Vel < 0) {
    Vel += 0.05;
    if (Vel > 0) {
      Vel = 0;
    }
  } else {
    Vel -= 0.05;
    if (Vel < 0) {
      Vel = 0;
    }
  }
  return Vel;
}

int corner(){
}
