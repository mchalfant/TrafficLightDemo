//****************************************************************//
//                                                                //
//  Class: CECS 447                                               //
//  Project name: Table TrafficLight                              //
//  File name: TableTrafficLight.c                                //
//  Target Board: TM4C123GHPM                                     //
//                                                                //
//  Created by Michael Chalfant on Septmeber 3, 2016              //
//  Copyright  2016 Michael Chalfant. All rights reserved.        //  
//																																//
//  Basic Setup: 																									//	
//																																//
//  East facing red light connected to PB5												//
//  East facing yellow light connected to PB4											//
//  East facing green light connected to PB3											//
//  North facing red light connected to PB2												//
//  North facing yellow light connected to PB1										//
//  North facing green light connected to PB0											//
//																																//
//  North facing car detector connected to PE1 (1=car present)		//
//  East facing car detector connected to PE0 (1=car present)			//
//                                                                //
//****************************************************************//

#include "PLL.h"
#include "SysTick.h"

#define TRAFFIC_LIGHT           (*((volatile unsigned long *)0x400050FC))
#define GPIO_PORTB_OUT          (*((volatile unsigned long *)0x400050FC)) // bits 5-0
#define GPIO_PORTB_DIR_R        (*((volatile unsigned long *)0x40005400))
#define GPIO_PORTB_AFSEL_R      (*((volatile unsigned long *)0x40005420))
#define GPIO_PORTB_DEN_R        (*((volatile unsigned long *)0x4000551C))
#define GPIO_PORTB_AMSEL_R      (*((volatile unsigned long *)0x40005528))
#define GPIO_PORTB_PCTL_R       (*((volatile unsigned long *)0x4000552C))
#define GPIO_PORTE_IN           (*((volatile unsigned long *)0x4002400C)) // bits 1-0
#define SENSOR                  (*((volatile unsigned long *)0x4002400C))

#define GPIO_PORTE_DIR_R        (*((volatile unsigned long *)0x40024400))
#define GPIO_PORTE_AFSEL_R      (*((volatile unsigned long *)0x40024420))
#define GPIO_PORTE_DEN_R        (*((volatile unsigned long *)0x4002451C))
#define GPIO_PORTE_AMSEL_R      (*((volatile unsigned long *)0x40024528))
#define GPIO_PORTE_PCTL_R       (*((volatile unsigned long *)0x4002452C))
#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))
#define SYSCTL_RCGC2_GPIOE      0x00000010  // port E Clock Gating Control
#define SYSCTL_RCGC2_GPIOB      0x00000002  // port B Clock Gating Control


struct TrafficLightState {
  unsigned long Out; 
  unsigned long Time;  
  unsigned long Next[4];
};

typedef const struct TrafficLightState TLS;
#define goNorth   0
#define waitNorth 1
#define goEast   2
#define waitEast 3

TLS FSM[4]={
 {0x21,3000,{goNorth,waitNorth,goNorth,waitNorth}}, 
 {0x22, 500,{goEast,goEast,goEast,goEast}},
 {0x0C,3000,{goEast,goEast,waitEast,waitEast}},
 {0x14, 500,{goNorth,goNorth,goNorth,goNorth}}};

unsigned long stateIndex; 
unsigned long sensorInput; 
 
int main(void){ 
	volatile unsigned long delay;
  PLL_Init();       
  SysTick_Init();   
  SYSCTL_RCGC2_R |= 0x12;           // 1) B E
  delay = SYSCTL_RCGC2_R;           // 2) no need to unlock
  GPIO_PORTE_AMSEL_R &= ~0x03;      // 3) disable analog function on PE1-0
  GPIO_PORTE_PCTL_R &= ~0x000000FF; // 4) enable regular GPIO
  GPIO_PORTE_DIR_R &= ~0x03;        // 5) inputs on PE1-0
  GPIO_PORTE_AFSEL_R &= ~0x03;      // 6) regular function on PE1-0
  GPIO_PORTE_DEN_R |= 0x03;         // 7) enable digital on PE1-0
  GPIO_PORTB_AMSEL_R &= ~0x3F;      // 8) disable analog function on PB5-0
  GPIO_PORTB_PCTL_R &= ~0x00FFFFFF; // 9) enable regular GPIO
  GPIO_PORTB_DIR_R |= 0x3F;         // 10) outputs on PB5-0
  GPIO_PORTB_AFSEL_R &= ~0x3F;      // 11) regular function on PB5-0
  GPIO_PORTB_DEN_R |= 0x3F;         // 12) enable digital on PB5-0
  stateIndex = goNorth;  
  while(1){
    TRAFFIC_LIGHT = FSM[stateIndex].Out;  // set lights
    SysTick_Wait10ms(FSM[stateIndex].Time);
    sensorInput = SENSOR;     // read sensors
    stateIndex = FSM[stateIndex].Next[sensorInput];  
  }
}

