/*####################################################################
 FILE: BitArray.h
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
#define BITARRAY_H

class BitArray;
//must declare BitArray because of the fact that it is referenced in the BitRef structure

typedef struct BitRef
{
	//internal structure
	bool value;
	unsigned char position;
	BitArray& parent;

	//operator overloads
	void operator=(bool newValue);
	//allows you to set the bit through the use of the = operator-> myBitArray[myBit]= myBoolValue
	void operator=(BitRef newValue);
	//allows you to set the bit through the use of the = operator-> myBitArray[myBit]= myBitArray[anotherBit];
	bool operator!();
	//returns the oposite boolean value
	//(i.e. if a 1 is stored in myBitArray[myBit], !myBitArray[myBit] would return a 0)

	bool operator!=(bool myComp);
	//allows the comparison of a BitRef directly to a boolean value
	bool operator== (bool myComp);
	//allows the comparison of a BitRef directly to a boolean value

	bool operator!= (BitRef myComp);
	//allows the comparison of a BitRef to the value of another BitRef
	bool operator== (BitRef myComp);
	//allows the comparison of a BitRef to the value of another BitRef

	//functions
	void setBit(bool newValue);
	//allows the a call to change the bit value in the parent BitArray to the designated value
	//(this is where the magic happens)
};

class BitArray
{

//internal structure
private:
unsigned char _myBITS; //this is the only data structure in BitArray

public:
//operator overloads
BitRef operator[](unsigned char myPOS);
//this function returns a BitRef to the indexed Bit, this is what allows
//manipulation of individual bits.
void operator=(unsigned char newValue);
//this function allows assignment of the whole array through the = sign, you may have to cast as
//(char) in some compilers.

//initialization routines
BitArray(unsigned char myBITS) {setBits(myBITS);}; //initialize with a char (8 bit number)
BitArray() {setBits(0);}; //empty initialization defaults to zeros


//functions
BitRef getBit(unsigned char myPOS);
//this is the function called by the indexing [] overload it performs bitwise
//operations and creates a BitRef structure to return.
void setBit(unsigned char position, bool newValue);
//this is the function called by a BitRef object to change an individual bit value

void setBits(unsigned char myBITS);
//this is the function called by initialization and the = operator

};

#endif//BITARRAY_H