/*####################################################################
 FILE: arrayOfBitArrays.ino
 VERSION: 1S0A
 PURPOSE: Bits are your friends. Use them well.
 LICENSE: GPL v3 (http://www.gnu.org/licenses/gpl.html)

 GET UPDATES: https://www.virtuabotix.com/resources.php?product=bitarray

 HISTORY:
 Joseph Dattilo (Virtuabotix LLC) - Version 1S0A (04/12/12)
#######################################################################*/

#include <BitArray.h>

BitArray myBits[256]; 

void setup()
{

Serial.begin(9600);

delay(10);

Serial.println("This example creates and demonstrates");
Serial.println("the use of an array of 256 BitArrays.");
Serial.println("");
Serial.println("BitArrays can be as large as your micro-controller can handle");


for(int x = 0; x <256;x++)
{
  myBits[x]=x;
}
for(int x = 0; x <256;x++)
{
  for(int i = 0; i <8;i++)
  {
  	Serial.print(myBits[x][i].value);
  }
  
  if((x+1)%4 == 0 && x != 0) Serial.println();
  
}
Serial.println();
Serial.print("All that in "); Serial.print(sizeof(myBits)); Serial.println(" bytes.");

}

void loop()
{

}
