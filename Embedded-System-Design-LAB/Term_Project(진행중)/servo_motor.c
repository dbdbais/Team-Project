#include "stm32f10x.h"
#include "core_cm3.h"
#include "misc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_adc.h"
#include "lcd.h"

uint16_t value;
int on = 0;
int off= 0;
int stop = 0;

int color[12] = 
{WHITE,CYAN,BLUE,RED,MAGENTA,LGRAY,GREEN,YELLOW,BROWN,BRRED,GRAY};

void delay(void) {
   int i;

   for (i = 0; i < 8000000; i++) {}
}

void RCC_Configure(void){
    // ADC port clock Enable
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); //TMI3
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //Servo
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);//Button
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE); //joystick down
    
    /* Alternate Function IO clock enable */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
}

//--------------------------------------------------------------------

void GPIO_Configure(void){
    GPIO_InitTypeDef  GPIO_InitStructure;

   // TODO: Initialize the GPIO pins using the structure 'GPIO_InitTypeDef' and the function 'GPIO_Init'

    //Servo pin setting
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* button pin setting */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOD,&GPIO_InitStructure);
    
    //joystick down
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOC,&GPIO_InitStructure);
}

//-----------------------------------------------------------------
void EXTI_Configure(void)
{
    EXTI_InitTypeDef EXTI_InitStructure;

    /* Button */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource11);
    EXTI_InitStructure.EXTI_Line = EXTI_Line11;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
    
    //joystick down
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource2);
    EXTI_InitStructure.EXTI_Line = EXTI_Line2;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
   
   // NOTE: do not select the UART GPIO pin used as EXTI Line here
}

void NVIC_Configure(void) {

    NVIC_InitTypeDef NVIC_InitStructure;
    
    // TODO: fill the arg you want
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

    // User S1 Button11
    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // Joystick Down2
    NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void EXTI15_10_IRQHandler(void) { // when the button is pressed

   if (EXTI_GetITStatus(EXTI_Line11) != RESET) {
        if (GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_11) == Bit_RESET) {
            on = 1;
        }
        EXTI_ClearITPendingBit(EXTI_Line11);
   }
}

void EXTI2_IRQHandler(){ //down
  if (EXTI_GetITStatus(EXTI_Line2) != RESET) {
    if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_2) == Bit_RESET) {
          on = 0;
      }
    EXTI_ClearITPendingBit(EXTI_Line2);
  }
}

//-----------------------------------------------------------------------

void PWM_Configure(void){
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
    
    // sysclk / period / prescaler = 72MHz / 20000 / 72 = 50Hz
    TIM_TimeBaseStructure.TIM_Period = 20000 ;         
    TIM_TimeBaseStructure.TIM_Prescaler = 72;           
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;       
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down; 
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    
    
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCPolarity =  TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    
    TIM_OCInitStructure.TIM_Pulse= 1500; //정중앙
    TIM_OC3Init(TIM3, &TIM_OCInitStructure);
    TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Disable);
    TIM_ARRPreloadConfig(TIM3, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
}

void servo_control(void){
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
    
    // sysclk / period / prescaler = 72MHz / 20000 / 72 = 50Hz
    TIM_TimeBaseStructure.TIM_Period = 20000 ;         
    TIM_TimeBaseStructure.TIM_Prescaler = 72;           
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;       
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down; 
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    
    
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCPolarity =  TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  
    TIM_Cmd(TIM3, DISABLE);
    TIM_OCInitStructure.TIM_Pulse= 600; //왼쪽끝
    TIM_OC3Init(TIM3, &TIM_OCInitStructure);
    TIM_Cmd(TIM3, ENABLE);
    
    delay();
    
    TIM_Cmd(TIM3, DISABLE);
    TIM_OCInitStructure.TIM_Pulse= 2400; //오른쪽끝
    TIM_OC3Init(TIM3, &TIM_OCInitStructure);
    TIM_Cmd(TIM3, ENABLE);
    
    delay();
}

int main() {
  
  // LCD 관련 설정은 LCD_Init에 구현되어 있으므로 여기서 할 필요 없음
  SystemInit();
  RCC_Configure();
  GPIO_Configure();
  
  EXTI_Configure();
  NVIC_Configure();
  
  PWM_Configure();
  // ------------------------------------
  
  LCD_Init();
  
  while(1){
      if(on){
            servo_control();
      }
      else{
            PWM_Configure();
      }
  }
}