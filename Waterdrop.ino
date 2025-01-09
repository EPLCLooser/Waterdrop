//included libraries
#include "U8glib.h"
#include "Wire.h"

// ADXL345 I2C address is 0x53(83)
int ADXL345 = 0x53;  // The ADXL345 sensor I2C address

float X_out, Y_out, Z_out;  // Outputs
const int arrlength = 5;
int oldestVal = 0;
float xVals[arrlength];
float yVals[arrlength];
float zVals[arrlength];
float xAcc;
float yAcc;
float xVel;
float yVel;
int xVal = 124 / 2;
int yVal = 64 / 2;
const float maxAcc = 100;


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
  /*
  Serial.print("Xa= ");
  Serial.print(X_out);
  Serial.print("   Ya= ");
  Serial.print(Y_out);
  *Serial.print("   Za= ");
  Serial.println(Z_out);
  */

  //Lägger in accelerometerns värden i en array
  xVals[oldestVal] = X_out;
  yVals[oldestVal] = Y_out;

  oldestVal++;
  if (oldestVal == arrlength) {
    oldestVal = 0;
  }
  delay(10);

  //mapar om medelvärdet av lutningens värde från accelerometern till acceleration i antal pixlar/sekund
  xAcc = map(tilt_med(xVals)*100, -92, 108, -maxAcc, maxAcc);
  yAcc = map(tilt_med(yVals)*100, -110, 90, maxAcc, -maxAcc);

  //Räknar ut pixel hastighet
  xVel += xAcc/100;
  yVel += yAcc/100;

  //Räknar ut pixel koordinat
  xVal += xVel;
  yVal += xVel;

  Serial.println(tilt_med(xVals));
  Serial.println(xAcc);
  Serial.println(tilt_med(yVals));
  Serial.println(yAcc);
  /*
  if (xVal < 0) {
    xVal = 0;
    xVel = 1;
  }
  if (xVal > 123) {
    xVal = 123;
    xVel = -1;
  }
  if (yVal < 0) {
    yVal = 0;
    yVel = 1;
  }
  if (yVal > 63) {
    yVal = 63;
    yVel = -1;
  }
  */
  //ritar ut pixeln
  u8g.firstPage();
  do {
    u8g.drawPixel(xVal, yVal);
  } while (u8g.nextPage());
}

float tilt_med(float Vals[]) {
  float sum = 0;
  for (int i = 0; i < arrlength; i++) {
    sum += Vals[i];
  }
  float med = sum / arrlength;
  return med;
}
