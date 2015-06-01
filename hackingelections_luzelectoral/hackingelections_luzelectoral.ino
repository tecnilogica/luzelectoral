#include <Adafruit_NeoPixel.h>
//Library available at https://github.com/adafruit/Adafruit_NeoPixel


#define PIN 6
#define NUMLEDS 33


#define NUMPARTIES 6
#define NUMMUNICIPALITIES 56
#define SEATMULTIPLIER 6
#define PARTYGUTTER 6
#define SCROLLDELAY 40


Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMLEDS, PIN, NEO_GRB + NEO_KHZ800);


//Party colors
byte colors[NUMPARTIES][3] = {
  { 15, 25, 140}, /*PP 0*/
  {255, 10,  10}, /*PS 1*/
  {180,  0, 140}, /*PO 2*/
  {255, 55,   0}, /*CI 3*/
  {  0, 155, 35}, /*NA 4*/
  {185, 185, 80}  /*OT 5*/
};


//Seat distribution
//Party * 256 + seats
//If a municipality has less than 6 parties, Arduino pads the array with zeroes,
//so {3, 258, 513} is really {3, 258, 513, 0 ,0 ,0} This is relevant later.
int municipalities[NUMMUNICIPALITIES][NUMPARTIES] = {
  //TEST CASE
  //{1, 257, 513, 769, 1025, 1281},
  {3, 258, 513},
  {11, 265, 517, 773, 1285},
  {21, 274, 526, 1032, 773, 1281},
  {4, 258, 769},
  {274, 15, 515},
  {5, 259, 514, 769},
  {13, 268, 515, 769},
  {1047, 271, 12, 519, 1283},
  {13, 1036, 261, 515, 770},
  {8, 262, 1284, 515, 771},
  {16, 270, 515},
  {42, 281, 522, 773, 1025, 1281},
  {4, 260},
  {48, 293, 539, 785},  /*****/
  {1058, 263, 519, 2},
  {31, 279, 1299, 525, 781},
  {3, 258},
  {1026, 257},
  {286, 28, 518, 769},
  {1025},
  {1027, 258, 1, 513},
  {1029, 4, 259, 515},
  {2, 258, 513},
  {262, 5, 516, 1026, 769},
  {5, 260, 515},
  {20, 270, 1037, 522, 770},
  {1283, 257},
  {3, 1027, 258},
  {15, 266, 516, 772},
  {1028, 258, 1, 513},
  {5, 261, 514, 769, 1025},
  {10, 1033, 263, 517, 770},
  {5, 259, 1027, 514},
  {4, 258, 513},
  {270, 11, 521, 1288, 771},
  {265, 1288, 7, 519, 771},
  {259, 2, 513},
  {2, 258, 513},
  {13, 1036, 261, 515, 770},
  {15, 266, 516, 772},
  {22, 269, 518, 772},
  {2, 258},
  {4, 258, 513},
  {1, 257},
  {5, 259, 514, 769},
  {10, 261, 515, 771},
  {6, 259, 513, 769},
  {4, 258, 513},
  {3, 258},
  {1030, 260, 3, 514},
  {5, 260, 514, 1026, 769},
  {4, 260, 513},
  {12, 1290, 264, 517, 773},
  {7, 260, 514, 769, 1281},
  {4, 258, 513},
  {11, 264, 520, 1028, 771, 1281}
};


//Array of seats (0 0 0 0 0 1 1 1 1 3 3 3...)
//for a given municipality
byte *seats = 0;

//Lenght of *seats. We can use sizeof, but this is cleaner.
int totalSeats = 0;

//Starting point for displaying the seats
int scrollOffset = 0;

//Randomly choosen municipality
byte currentMunicipality;


void setup() {

  //Serial.begin(57600);
  
  //Reset rand generator
  randomSeed(analogRead(0));

  strip.begin();
  strip.show();

  getPartyData();

}


void loop() {

  //Light up the pixels with the color of the party
  //Turn them off if they are in the gutter zone
  for (byte x = 0; x < NUMLEDS; x++) {
    byte party = seats[x + scrollOffset];
    if (party != 0xFF) {
      strip.setPixelColor(x, colors[party][0], colors[party][1], colors[party][2]);
    } else {
      strip.setPixelColor(x, 0, 0, 0);
    }
  }
  strip.show();

  //Scroll the data
  scrollOffset++;
  
  //If we have no more seats to show, get another municipality
  if (scrollOffset >= totalSeats - NUMLEDS) {
    scrollOffset = 0;
    getPartyData();
  }

  //Wait a bit. What's the rush?
  delay(SCROLLDELAY);

}


void getPartyData() {

  //Pick up a random municipality
  currentMunicipality = random(NUMMUNICIPALITIES);
  //currentMunicipality = 13;
  //Serial.print("M");
  //Serial.print(currentMunicipality+1);
  
  //Reset the variables
  byte distribution[NUMPARTIES] = {0, 0, 0, 0, 0, 0};
  totalSeats = 0;
  byte zeroSeats = 0;

  //For each item in the array, we discard it if its value is zero (now it is relevant)
  //If it is not, we get the party and the total of seats
  //We update totalSeats to alloc memory later.
  for (byte f = 0; f < sizeof(municipalities[currentMunicipality]) / sizeof(int); f++) {
    if (municipalities[currentMunicipality][f] == 0) {
      zeroSeats++;
      continue;
    }
    byte partyID = municipalities[currentMunicipality][f] >> 8;
    byte seats = municipalities[currentMunicipality][f] & 0x00FF;
    distribution[partyID] = seats;
    totalSeats += (seats * SEATMULTIPLIER);
  }

  //We update totalSeats again, adding the gutter pixels
  totalSeats += (NUMPARTIES-zeroSeats-1) * PARTYGUTTER;

  //We add even more pixels, to add some padding after and before the votes
  totalSeats += NUMLEDS * 2;
  //Serial.print(" - T");
  //Serial.println(totalSeats);
  //Serial.println();

  //C-style memory allocation
  if (seats != 0) {
    seats = (byte*) realloc(seats, totalSeats * sizeof(byte));
  } else {
    seats = (byte*) malloc(totalSeats * sizeof(byte));
  }

  //We reset all the array pixels to the off state
  for (int f = 0; f < totalSeats; f++) {
    seats[f] = -1;
  }

  //We leave the first NULEDS as off...
  int currentSeat = NUMLEDS;
  
  //... and fill the rest of the array
  for (byte f = 0; f < NUMPARTIES; f++) {
    for (byte g = 0; g < distribution[f]; g++) {
      for (byte m = 0; m < SEATMULTIPLIER; m++) {
        seats[currentSeat++] = f;
      }
    }
    if (distribution[f] > 0) {
      currentSeat += PARTYGUTTER;
    }
  }

}


