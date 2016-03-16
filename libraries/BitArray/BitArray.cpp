/*####################################################################
 FILE: BitArray.cpp
 VERSION: 1S0A
 PURPOSE: A new more efficient way to store boolean values at the bit level
 LICENSE: GPL v3 (http://www.gnu.org/licenses/gpl.html)

 GET UPDATES: https://www.virtuabotix.com/resources.php?product=bitarray

                                                        ++++++ ++   ++
                          '@          @@@:              @@@@@@ @@` '@@
                      .@@@@@#         @`@@@@@.            @@   @@# @'@
                     @@@.  `@        ;@    .@@            @@   @.@ @.@
                    `@      @@       @@     @+            @@   @ @ @.@
                     @`      @:      @`     @`            @@   @ @@@.@
                     @#      ;@@@@@@@@      @             @@   @ #@,.@
                     '@                     @             @@   @ .@ .@
                     ;@                     @@.      @@@
           `@      `@@.                      +@@    @@`@@
           @@@'   @@@                          @@; @@   @@
          @@ ,@@#@@                             ,@@@     @@
         @@    #@                                 ,       @+
        @@                  .#@@@@@@@@@@@`                @,
        @                @@@@@+,     `:+@@               @@
        #@               @:             @@              '@
         @@              @#             @.              @,
          @@             `@            `@     ++        @
          `@        #     @+           @@    ;@@@       @@
          @+       @@@    .@   `       @`   .@@@@@       @,
         +@       @@@@@    @+'@@,  @@@#@    @@@@@@@      '@
         @.      @@@@@@+   `@@@@   :@'@;   @@@@@@@@@      @@   .+`
        @@      @@@@@@@@`     @;    @@    #@@@@@@@@@,      @@@@@@@
  @@@: `@      +@@@@@@@@@    +@      @.   `@@@@@@@@@@       '`   @
  @,@@@@;      @@@@@@@@@@    @:  @@  +@    ;@@@@@@@@@#           @;
  @           @@@@@@@@@@,   #@  @@@@  @#    #@@@@@@@@@           @@
 '@           @@@@@@@@@@    @,  @``@` `@    @@@@@@@@@@,          `@
 @@          ;@@@@@@@@@+   @@  @@  @@  @@   @@@@@@@@@@@           @`
 @,          @@@@@@@@@@@   @. .@    @#  @' @@@@@@@@@@@@          @@:
 @           @@@@@@@@@@@@ @@  @@    ,@  :@'@@@@@@@@@@@@       :@@@`
 @@#         @@@@@@@@@@@@,@` ;@ '@@' @@  @@@@@:@@@@@@@@`      @'
  ;@@@      ;@@@@@@@@@@@@@@  @'@@.,@@,@; @@@@@ #@@@@@@@`      @
     @      @@@@@@@@' @@@@@ #@@+    ;@@@.@@@@.  @@@;          @
     @          .;+@  '@@@@#@'@      ;@@@@@@@                 @
     @                 @@@@@@:@      +@;@@@@@                 @
     @`                @@@@@@:@      #@@@@@@@@  @@@:          @
     @`         .@@@#  @@@@@@+@      @@@@@@+ @;+@+@@@@@@      @
     @`      @@@@@;;@ @@.@@@@@@#    :@@@@@@` ;@@#     .@      @@+
     @`     '@,     @.@  @@@@@@@@' @@@@@@@@@  @@,     +@       :@@@.
   `@@`     ;@      @@# +@@@@@@  @@@ @@@@@ @+  @      @+          @@
 +@@@       `@      .@  @, @@@@@    @@@@@  `@         @           '@
#@:          @         @@  #@@@@.   @@@@@   @@       .@           @#
 @           @         @    @@@@@  @@@@@    :@       @@           @`
 @;          @#       +@    .@@@@@.@@@@#   `@:       @            @
 #@          ,@        @@    @@@@@@@@@@    @#       @@       ,   @@
  @           @,        @#    @@@@@@@@#    @`      `@       @@@@@@;
  @@@@@@@     :@         @    '@@@@@@@     +@      @#      ;@
   #;.  @@     @@       @@     @@@@@@@.@    @@    @@       @;
        `@      @+     @@   #@@@@@@@@@@@+    @+  @@       +@
         @@      @@   #@    @@@@@@@@@@@@@    `@.@@        @:
          @.      @@.:@     @@@@@@@@@@@@@#    '@#        +@
          '@       #@@,    #@@@@@@@@@@@@@@               @;
           @;              @@@@@@@@@@@@@@@'              `@`
          `@               @@@@@@@@@@@@@@@@               #@
          @+              '@@@@@@@@@@@@@@@@                @@
         @@                '@@@@@@@@@@@:                    @:
        @@                                          '      @@
        +@;     `@;                               @@@@    @@
         :@+   ,@'@@                            ;@@  @@# @@
          .@@ :@;  @@@                         @@:    .@@@
            @@@;     @@:                     `@@        '
             @;       ;@@                    .@
                       '@      +@@@@@@@.      @
                       @@     .@###@@@@@      @`
                       @;     @@       @.     @+
                       @`     @        @@    ,@@
                       @@@.  ;@        `@;@@@@@`
                        :@@@@@#         @@@.

   @@@@@  @@@@@@  @@@@@@ @@@ @@@     @@@@@    @@@@@  @@@@@@:  @@@@@
  @@@@@@@ @@@@@@@ @@@@@@ @@@ @@@    @@@@@@@  @@@@@@@ @@@@@@@: @@@@@
  @@@ @@@ @@@@@@@ @@@@@@ @@@ @@@    @@@ @@@  @@@ @@@ @@   @@@ @@@@@
  @@@ @@@ @@@  @@ @@@    @@@ @@@    @@@ @@@  @@@ @@@ @@    @@ @@@
  @@@ @@@ @@@@@@@ @@@    @@@@@@@    @@@ @@@  @@@ @@@ @@    @@ @@@
  @@@ @@@ @@@@@@@ @@@@@  @@@@@@@    @@@      @@@ @@@ @@    @@ @@@@@
  @@@ @@@ @@@@@@@ @@@@@  @@@@@@@    @@@      @@@ @@@ @@    @@ @@@@@
  @@@ @@@ @@@@    @@@    @@ @@@@    @@@ @@@  @@@ @@@ @@    @@ @@@
  @@@ @@@ @@@@    @@@    @@ @@@@    @@@ @@@  @@@ @@@ @@    @@ @@@
  @@@ @@@ @@@@    @@@    @@ @@@@    @@@ @@@  @@@ @@@ @@   @@@ @@@
  @@@@@@@ @@@@    @@@@@@ @@  @@@    @@@@@@@  @@@@@@@ @@@@@@@  @@@@@
   @@@@@  @@@@    @@@@@@ @@  @@@     @@@@@    @@@@@  @@@@@@   @@@@@

 HISTORY:
 Joseph Dattilo (Virtuabotix LLC) - Version 1S0A (04/13/12)
#######################################################################*/

#ifndef BITARRAY_H

//##########arduino specific code##########
//this section is required for use in the Arduino Environment replace with the standard library in C++
#include <stddef.h>

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
  #include <pins_arduino.h>
#endif

//#####end arduino specific code##########

#include "BitArray.h"


//####################Functions###################################

BitRef BitArray::getBit(unsigned char myPOS)
{
	BitRef myReturn = {(0x01 & (_myBITS >> myPOS)),myPOS,*this  };
		return myReturn;
}

void BitArray::setBit(unsigned char position, bool newValue)
	{
		if( getBit(position).value != newValue)
		{
		_myBITS = _myBITS ^ (0x80 >> (8-(position+1)));
		}
	}


void BitArray::setBits(unsigned char myBITS)
	{
		_myBITS = myBITS;
	}


void BitRef::setBit (bool newValue)
	{
		parent.setBit(position, newValue);
	}
//-------------------------------------------------------------

//######################operators##############################

void BitArray::operator=(unsigned char newValue)
{
	setBits(newValue);
}


BitRef BitArray::operator[](unsigned char myPOS)
	{
		return getBit(myPOS);
	}

bool BitRef::operator!()
	{
		return !value;
	}

void BitRef::operator = (bool newValue)
	{
		parent.setBit(position, newValue);
	}

void BitRef::operator = (BitRef newValue)
	{
		parent.setBit(position, newValue.value);
	}

bool BitRef::operator!=(bool myComp)
	{
		return value != myComp;
	}

bool BitRef::operator== (bool myComp)
	{
		return value == myComp;
	}

bool BitRef::operator!=(BitRef myComp)
	{
		return value != myComp.value;
	}

bool BitRef::operator== (BitRef myComp)
	{
		return value == myComp.value;
	}


//-------------------------------------------------


#endif // BITARRAY_H
