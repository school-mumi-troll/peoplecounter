#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#include <SPI.h>
#include <SD.h>
#include <NewPing.h>

#define SONAR_NUM 2      // Number of sensors.
#define MAX_DISTANCE 200 // Maximum distance (in cm) to ping.
#define WAIT_ANY 0
#define WAIT_POS 1
#define WAIT_NEG 2
#define WAIT_END 3

#define FORWARD 0
#define BACKWARD 1

NewPing sonar[SONAR_NUM] = {   // Sensor object array.
  NewPing(9, 8, MAX_DISTANCE), // Each sensor's trigger pin, echo pin, and max distance to ping.
  NewPing(7, 6, MAX_DISTANCE)
};
int state = WAIT_ANY;
int dir = FORWARD;
int thr = 40;
int offset = 16;
int countend = 0;
int countwait = 0;
int skipwait = 7;
int waitend = 0;

File myFile;

void setup() {
  unsigned long a1, a2;
  long d1, d2, dd, sum;
  Serial.begin(115200);
  while (!Serial) ; // wait for serial
  delay(200);
  Serial.println("People Counter Test");
  Serial.println("-------------------");
  Serial.print("Initializing SD card...");

  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");

  sum = 0;
  Serial.println("Calibration:");
  for (uint8_t j = 0; j < 20; j++) {
    delay(50);
    a1 = sonar[0].ping_cm();
    delay(50);
    a2 = sonar[1].ping_cm();
    d1 = a1 - a2;
    d2 = a2 - a1;
    dd = d1 - d2;
    sum += dd;
    Serial.print(dd);
    Serial.print(" ");
  }
  Serial.println();
  offset = sum / 20;
  Serial.println(offset);

}

int detectFSM() {
  int personin = 0;
  int personout = 0;
  unsigned long a1, a2;
  long d1, d2, dd;
  

  delay(50);
  a1 = sonar[0].ping_cm();
  delay(50);
  a2 = sonar[1].ping_cm();

  d1 = a1 - a2;
  d2 = a2 - a1;
  //Serial.print(d1);
  //Serial.print(" ");
  //Serial.print(d2);
  //Serial.print(" ");
  dd = d1 - d2 - offset;
 
  if (state == WAIT_ANY) {
    if (dd < -thr) {
      state = WAIT_POS;
      dir = FORWARD;
    } else if (dd > thr) {
      state = WAIT_NEG;
      dir = BACKWARD;
    }
  } else if (state == WAIT_NEG) {
    if (dd < -thr) {
      state = WAIT_END;
    } else {
      if (dd > -thr && dd < thr) {
        //Serial.print(" skip pos");
        countwait++;
        if (countwait > skipwait) {
          countwait = 0;
          state = WAIT_ANY;
        }
      } else {
        countwait = 0;
      }
    }
  } else if (state == WAIT_POS) {
    if (dd > thr) {
      state = WAIT_END;
    } else {
      if (dd > -thr && dd < thr) {
        //Serial.print(" skip neg");
        countwait++;
        if (countwait > skipwait) {
          countwait = 0;
          state = WAIT_ANY;
        }
      } else {
        countwait = 0;
      }
    }
  } else if (state == WAIT_END) {
    if (dd < thr && dd > -thr) {
      countend++;
    }
    if (countend > waitend) {
      if (dir == FORWARD) {
        //Serial.println("+1 person");
        personin += 1;
      } else if (dir == BACKWARD) {
        //Serial.println("-1 person");
        personout += 1;
      }

      countend = 0;
      state = WAIT_ANY;
    }
  }
  
  //Serial.print(countwait);
  //Serial.print(" ");
  //Serial.print(countend);
  //Serial.print("|");
  Serial.print(personin);
  Serial.print(" ");
  Serial.print(personout);
  Serial.print(" ");
  Serial.print(dd);

  Serial.print(" ");
  Serial.print(state);
  Serial.println();
  if (personin > 0) {
    return personin;
  } else if (personout > 0) {
    return -personout;
  }
  return 0;
}

void loop() {
  tmElements_t tm;
  int person = 0;

  while (true) {
    person = detectFSM();
    if (person != 0) break;
  }

  if (RTC.read(tm)) {
    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    char fileName[16];
    char timeText[16];
    char dateText[16];
    sprintf(fileName, "%04d%02d%02d.txt", tmYearToCalendar(tm.Year), tm.Month, tm.Day);
    sprintf(dateText, "%04d-%02d-%02d", tmYearToCalendar(tm.Year), tm.Month, tm.Day);
    sprintf(timeText, "%02d:%02d:%02d", tm.Hour, tm.Minute, tm.Second);
    Serial.print(dateText);
    Serial.print(" ");
    Serial.print(timeText);
    Serial.print(" ");
    Serial.println(person);

    myFile = SD.open(fileName, FILE_WRITE);
    // if the file opened okay, write to it:
    if (!myFile) {
      // if the file didn't open, print an error:
      Serial.print("error opening ");
      Serial.println(fileName);
    } else {
      Serial.print("Writing to ");
      Serial.print(fileName);



      myFile.print(dateText);
      myFile.print(" ");
      myFile.print(timeText);
      myFile.print(" ");
      myFile.println(person);

      // close the file:
      myFile.close();
      Serial.println(" - done.");
    }

  } else {
    if (RTC.chipPresent()) {
      Serial.println("The DS1307 is stopped.  Please run the SetTime");
      Serial.println("example to initialize the time and begin running.");
      Serial.println();
    } else {
      Serial.println("DS1307 read error!  Please check the circuitry.");
      Serial.println();
    }
    delay(9000);
  }
  //delay(1000);
}


