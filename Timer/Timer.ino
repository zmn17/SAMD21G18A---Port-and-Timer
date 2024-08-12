#include "GPIO_DEAKIN.h"
#include "TIMER_DEAKIN.h"

GPIO_DEAKIN gp;
TIMER_DEAKIN timer;

// bool success = false;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  gp.Config_GPIO('A', 20, "OUTPUT");

  // test purpose
  // success = timer.config_timer(3, 0, 1000, 10);
}

void loop() {
  gp.Write_GPIO('A', 20, HIGH);
  timer.wait(20000); // 2000 milliseconds - 2 seconds
  gp.Write_GPIO('A', 20, LOW);
  timer.wait(20000);
}
