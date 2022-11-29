#include <stdio.h>
#include "stm32f10x.h"

#define R_CC 0x40021000
#define PORT_B 0x40010C00               // pb 8 selection
#define PORT_C 0x40011000               // pc 5 stick, pc 8 relay
#define PORT_D 0x40011400               // pd 2,3,4 led, pd 11,12 user

typedef volatile unsigned int* VOL;

void delay()
{
  int i;
  for(i = 0; i < 10000000; i++) {}
}
             
int main(void)
{
  *(VOL) (R_CC + 0x18) |= 0x38;                 // clock enable portB, port C, port D
  *(VOL) (PORT_B + 0x04) = 0x8;                 // configuration high, selection
  *(VOL) (PORT_C + 0x00) = 0x800000;            // configuration low, stick 5
  *(VOL) (PORT_C + 0x04) = 0x3;                 // configuration high, relay 8        
  *(VOL) (PORT_D + 0x00) = 0x00033300;          // configuration low, led 2,3,4
  *(VOL) (PORT_D + 0x04) = 0x88000;             // configuration high, user 11, 12

  while(1){
    if(!(*(VOL)(PORT_C + 0x08) & 0x20)){                        //up 7
      *(VOL) (PORT_C + 0x10) = 0x100;           // relay start
      delay();
      *(VOL) (PORT_C + 0x14) = 0x100;           // relay stop
    }
    else if((*(VOL)(PORT_D + 0x08) & 0x800) != 0x800){          // 11
      *(VOL) (PORT_D + 0x10) = 0x4;             // led on
      delay();
      *(VOL) (PORT_D + 0x14) = 0x4;             // led off
    }
    else if(!(*(VOL)(PORT_D + 0x08) & 0x1000)){                 // 12
      *(VOL) (PORT_D + 0x10) = 0x8;             // led on
      delay();
      *(VOL) (PORT_D + 0x14) = 0x8;             // led off
    }
    else if(!(*(VOL)(PORT_B + 0x08) & 0x100)){                  // 8
      *(VOL) (PORT_D + 0x10) = 0x10;            // led on
      delay();
      *(VOL) (PORT_D + 0x14) = 0x10;            // led off
    }
  }
  return 0;
}