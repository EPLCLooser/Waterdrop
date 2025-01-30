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
int drops[numofdrops][6];  // [vilken droppe][x-position; y-position; x-acceleration; y-acceleration; x-velocity; y-velocity]
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
  for (int x = 0; x < numofdrops; x++) {
    drops[x][0] = x;
  }
  u8g.nextPage();
  Serial.println(drops[0][0]);
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
  for (int i = 0; i < numofdrops; i++) {
    drops[i][2] = map(xincline * 100, -93, 107, -maxAcc, maxAcc);
    drops[i][3] = map(yincline * 100, -110, 90, maxAcc, -maxAcc);

    //Räknar ut pixel hastighet
    drops[i][4] += drops[i][2] / 100;
    drops[i][5] += drops[i][3] / 100;

    //friktion för vattnet
    drops[i][4] = friction(drops[i][4]);
    drops[i][5] = friction(drops[i][5]);

    //Räknar ut pixel koordinat
    drops[i][0] += drops[i][4];
    drops[i][1] += drops[i][5];

    if (drops[i][0] < 0) {
      drops[i][0] = 0;
      drops[i][2] = 0;
      drops[i][4] = 0;
    } else if (drops[i][0] > 123) {
      drops[i][0] = 123;
      drops[i][2] = 0;
      drops[i][4] = 0;
    }
    if (drops[i][1] < 0) {
      drops[i][1] = 0;
      drops[i][3] = 0;
      drops[i][5] = 0;
    } else if (drops[i][1] > 63) {
      drops[i][1] = 63;
      drops[i][3] = 0;
      drops[i][5] = 0;
    }
    
  }

  // kollar om någon droppe har samma position som någon annan och ändrar positionen
  samepos(drops);

  //ritar ut droppar
  u8g.firstPage();
  do {
    for (int i = 0; i < numofdrops; i += 2) {
      u8g.drawPixel(drops[i], drops[i + 1]);
    }
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

int samepos(int arr[numofdrops][6]) {
  for (int i = 0; i < numofdrops; i++) {
    for (int u = 0; u < numofdrops; u += 1) {
      if (arr[u][0] == arr[i][0] && u != i) {
        if (arr[u][1] == arr[i][1]) {
          if (arr[u][0] == 123) {
            if (arr[u][1] <= 30) {
              arr[u][1] += 1;
            } else {
              arr[u][1] -= 1;
            }
            arr[u][0] -= 1;
          } else {
            if (arr[u][1] <= 30) {
              arr[u][1] += 1;
            } else {
              arr[u][1] -= 1;
            }
            arr[u][0] += 1;
          }
          i = 0;
        }
      }
    }
  }
  return arr;
}
