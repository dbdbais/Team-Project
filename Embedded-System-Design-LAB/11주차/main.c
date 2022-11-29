#include "stm32f10x.h"
#include "core_cm3.h"
#include "misc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_adc.h"
#include "lcd.h"
#include "touch.h"
int color[12] = 
{WHITE,CYAN,BLUE,RED,MAGENTA,LGRAY,GREEN,YELLOW,BROWN,BRRED,GRAY};
int btn = 0;
int Tim2_cnt = 0;
int flag_cnt5 = 0;

uint16_t value;
//LED1 : PD2, LED2: PD3
void RCC_Configure(void){
    // ADC port clock Enable
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); //TMI2
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); //TMI3
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE); //LED
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); //LED
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //Servo
    
    /* Alternate Function IO clock enable */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
}

void GPIO_Configure(void){
    GPIO_InitTypeDef  GPIO_InitStructure;

   // TODO: Initialize the GPIO pins using the structure 'GPIO_InitTypeDef' and the function 'GPIO_Init'
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; // TIM2
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; //TIM3
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
  
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //LED1
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; //LED2
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
}

void delay(void) {
   int i;

   for (i = 0; i < 8000000; i++) {}
}

void PWM_Configure(void){
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
    
    TIM_TimeBaseStructure.TIM_Period = 20000 ;
    TIM_TimeBaseStructure.TIM_Prescaler = 72;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;
    
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCPolarity =  TIM_OCPolarity_High;//카운터값이 지정한 CCRx값과 동일할 때 인터럽트.
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse= 2000;
    TIM_OC3Init(TIM3, &TIM_OCInitStructure);
    
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Disable);
    TIM_ARRPreloadConfig(TIM3, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
    
    delay();
    
    TIM_Cmd(TIM3, DISABLE);
    TIM_OCInitStructure.TIM_Pulse= 1500;
    TIM_OC3Init(TIM3, &TIM_OCInitStructure);
    TIM_Cmd(TIM3, ENABLE);
    
    delay();
    
    TIM_Cmd(TIM3, DISABLE);
    TIM_OCInitStructure.TIM_Pulse= 1000;
    TIM_OC3Init(TIM3, &TIM_OCInitStructure);
    TIM_Cmd(TIM3, ENABLE);
}

void TIM_Configure(void){
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
    
    TIM_TimeBaseStructure.TIM_Period = 10000 ;
    TIM_TimeBaseStructure.TIM_Prescaler = 7200;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;
    
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    TIM_Cmd(TIM2, ENABLE);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    
}

/*void EXTI_Configure(void){
    EXTI_InitTypeDef EXTI_InitStructure;
    
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource0);
    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling
    EXTI_Init(&EXTI_InitStructure);
}*/

void NVIC_Configure(void){
    NVIC_InitTypeDef NVIC_InitStructure;
    
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

    // TODO: Initialize the NVIC using the structure 'NVIC_InitTypeDef' and the function 'NVIC_Init'
    NVIC_EnableIRQ(TIM2_IRQn);
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    NVIC_EnableIRQ(TIM3_IRQn);
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void TIM2_IRQHandler(){
    if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET){
      if(btn){
          if(Tim2_cnt%2 == 0){
              GPIO_SetBits(GPIOD,GPIO_Pin_2);
          }
          else{
            GPIO_ResetBits(GPIOD, GPIO_Pin_2);
          }
          if(Tim2_cnt%5 == 0){
            if(flag_cnt5 == 0){
              GPIO_SetBits(GPIOD,GPIO_Pin_3);
              flag_cnt5 = 1;
            }
            else{
              GPIO_ResetBits(GPIOD, GPIO_Pin_3);
              flag_cnt5 = 0;
            }
          }
          Tim2_cnt++;
      }
      else{
        Tim2_cnt = 0;
        flag_cnt5 = 0;
      }
    }
    TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
}

int main() {
  uint16_t x, y,nx,ny;
  
  // LCD 관련 설정은 LCD_Init에 구현되어 있으므로 여기서 할 필요 없음
  SystemInit();
  RCC_Configure();
  GPIO_Configure();
  TIM_Configure();
  PWM_Configure();
  NVIC_Configure();
  // ------------------------------------

  LCD_Init();
  Touch_Configuration();
  Touch_Adjust();
  LCD_Clear(WHITE);
  
  char str[11] = "THU_Team02";

  LCD_ShowString(0, 0, str, BLACK, WHITE);

  LCD_DrawRectangle(50, 50, 100, 100);
  LCD_ShowString(60,65,"BUT",RED, WHITE);
  LCD_ShowString(0,20,"OFF",RED, WHITE);
  
  while(1){
  // TODO : LCD 값 출력 및 터치 좌표 읽기
      
      Touch_GetXY(&x,&y,1);
      Convert_Pos(x,y,&nx,&ny);
      if(nx >= 50 && nx <=100 && ny >=50 && ny <=100){
        if(btn == 0){
            btn = 1;
            LCD_ShowString(0,20," ON",RED, WHITE);
        }
        else{
            btn = 0;
            LCD_ShowString(0,20,"OFF",RED, WHITE);
            GPIO_ResetBits(GPIOD, GPIO_Pin_2);
            GPIO_ResetBits(GPIOD, GPIO_Pin_3);
          
        }
      }
      
  }
}