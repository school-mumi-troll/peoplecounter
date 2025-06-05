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
  NewPing(3, 5, MAX_DISTANCE), // Each sensor's trigger pin, echo pin, and max distance to ping.
  NewPing(4, 6, MAX_DISTANCE)
};
int state = WAIT_ANY;
int dir = FORWARD;
int thr = 40;
int offset = 16;
int countend = 0;
int countwait = 0;
int skipwait = 7;
int waitend = 0;
void setup() {
  Serial.begin(115200); // Open serial monitor at 115200 baud to see ping results.
  unsigned long a1, a2;
  long d1, d2, dd, sum;
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
int personin = 0;
int personout = 0;
void loop() {
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
}

