/*####################################################################
 FILE: simpleExample.ino
 VERSION: 1S0A
 PURPOSE: Bits are your friends. Use them well.
 LICENSE: GPL v3 (http://www.gnu.org/licenses/gpl.html)

 GET UPDATES: https://www.virtuabotix.com/resources.php?product=bitarray

 HISTORY:
 Joseph Dattilo (Virtuabotix LLC) - Version 1S0A (04/12/12)
#######################################################################*/

#include <BitArray.h>

BitArray myBits= BitArray(0xAA); //starts out binary 01010101

void setup()
{

Serial.begin(9600);

Serial.println("This is an example of using a BitArray to manipulate individual bits");

  for(int i = 0; i <8;i++)
  {
  	Serial.print(myBits[i].value);
  }
  
  for(int i = 0; i <8;i++)
  myBits[i] = !myBits[i]; 
  Serial.println();
  for(int i = 0; i <8;i++)
  {
  	Serial.print(myBits[i].value);
  }
  
  for(int i = 0; i <8;i++)
  myBits[i] = 1; 
  Serial.println();
  for(int i = 0; i <8;i++)
  {
  	Serial.print(myBits[i].value);
  }
  
  for(int i = 0; i <8;i++)
  myBits[i] = 0; 
  Serial.println();
  for(int i = 0; i <8;i++)
  {
  	Serial.print(myBits[i].value);
}

}

void loop()
{

}