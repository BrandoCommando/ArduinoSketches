/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the Uno and
  Leonardo, it is attached to digital pin 13. If you're unsure what
  pin the on-board LED is connected to on your Arduino model, check
  the documentation at http://www.arduino.cc

  This example code is in the public domain.

  modified 8 May 2014
  by Scott Fitzgerald
 */


// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin 13 as an output.
  for(int i=1;i<=13;i++)
    pinMode(i, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  int i;
  for(i=1;i<=13;i++)
    digitalWrite(i, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(500);              // wait for a second
  for(i=1;i<=13;i++)
    digitalWrite(i, LOW);    // turn the LED off by making the voltage LOW
  delay(500);              // wait for a second
}
