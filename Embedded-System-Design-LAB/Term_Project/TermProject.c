#include "stm32f10x.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_rcc.h"
#include "core_cm3.h"
#include "touch.h"
#include "lcd.h"
#include "misc.h"
#include "stm32f10x_adc.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#define TRUE 1
#define FALSE 0
uint16_t value;
int stop = 0;

volatile uint32_t ADC_Value[2];

int color[12] = 
{WHITE,CYAN,BLUE,RED,MAGENTA,LGRAY,GREEN,YELLOW,BROWN,BRRED,GRAY};

// Pet information structure
struct Pet{
    uint16_t age;
    uint16_t weight;
    uint16_t jender;
} pet;


/* function prototype */
void RCC_Configure(void);
void GPIO_Configure(void);
void USART1_Init(void);
void USART2_Init(void);
void NVIC_Configure(void);

void Delay(void);

void Delay(void) {
   int i;

   for (i = 0; i < 50000; i++) {}
}

void motorDelay(){
  int i;
  for(i = 0; i < 2000000; i++){}
}
//---------------------------------------------------------------------------------------------------

void RCC_Configure(void)
{
   // USART2_TX : PA2, USART2_RX : PA3
   //UART PA2, PA2 + press sensor PA1 + detect sensor PA4
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
   /* USART1 clock enable */
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);   
   /* USART2 clock enable */
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
   // Servo motor PB0
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
   // Servo pulse
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
   // ADC1 = press + detect
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
   // DMA = press + detect
   RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
   /* Alternate Function IO clock enable */
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

}

void GPIO_Configure(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

   // TODO: Initialize the GPIO pins using the structure 'GPIO_InitTypeDef' 
   // and the function 'GPIO_Init'
   
    // press sensor = PA1
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // detect sensor = PA4
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // servo motor = PB0
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* USART2 Pin Setting */
    //TX PA9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA,&GPIO_InitStructure);
    //RX PA10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA,&GPIO_InitStructure);
    
    /* USART2 Pin Setting */
    //TX PA2
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA,&GPIO_InitStructure);
    //RX PA3
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA,&GPIO_InitStructure);
}





// press sensor + detect sensor DMA setting
void DMA_Configure(void){
    DMA_InitTypeDef DMA_Instructure;
    
    DMA_DeInit(DMA1_Channel1);
    DMA_Instructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
    DMA_Instructure.DMA_MemoryBaseAddr = (uint32_t)&ADC_Value;
    DMA_Instructure.DMA_Mode = DMA_Mode_Circular;
    DMA_Instructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_Instructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_Instructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_Instructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
    DMA_Instructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
    DMA_Instructure.DMA_Priority = DMA_Priority_VeryHigh; 
    DMA_Instructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Instructure.DMA_BufferSize = 2;
    
    DMA_Init(DMA1_Channel1, &DMA_Instructure);
    DMA_Cmd(DMA1_Channel1, ENABLE);
}


// press sensor + detect sensor ADC setting
void ADC_Configure(void){
    ADC_InitTypeDef  ADC_InitStructure;
    
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_Mode =  ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_NbrOfChannel = 2;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; 
    
    ADC_Init(ADC1, &ADC_InitStructure);
    
    ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_239Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 2, ADC_SampleTime_239Cycles5);
    ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);
    ADC_DMACmd(ADC1, ENABLE);
    ADC_Cmd(ADC1, ENABLE);
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}



// USART1 setting
void USART1_Init(void)
{
    USART_InitTypeDef USART1_InitStructure;

    USART_Cmd(USART1, ENABLE);

    USART1_InitStructure.USART_BaudRate = 9600;
    USART1_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART1_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART1_InitStructure.USART_Parity = USART_Parity_No;
    USART1_InitStructure.USART_StopBits = USART_StopBits_1;
    USART1_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART1, &USART1_InitStructure);

    USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);
   
}

// USART2 setting
void USART2_Init(void){
    USART_InitTypeDef USART2_InitStructure;
    
    USART_Cmd(USART2, ENABLE);

    USART2_InitStructure.USART_BaudRate = 9600;
    USART2_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART2_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART2_InitStructure.USART_Parity = USART_Parity_No;
    USART2_InitStructure.USART_StopBits = USART_StopBits_1;
    USART2_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART2, &USART2_InitStructure);

    USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);
}

// USART NVIC for Interrupt order and channel 
void NVIC_Configure(void) {

    NVIC_InitTypeDef NVIC_InitStructure;
    
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

    // UART1
    // 'NVIC_EnableIRQ' is only required for USART setting
    NVIC_EnableIRQ(USART1_IRQn);
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // UART2
    // 'NVIC_EnableIRQ' is only required for USART setting
    NVIC_EnableIRQ(USART2_IRQn);
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    
}

//send data to UART1 
void sendDataUART1(uint16_t data) {
  USART_SendData(USART1, data);  
}

//send data to UART2
void sendDataUART2(uint16_t data) {
  USART_SendData(USART2, data);  
}

//UART1 interrupt handler
void USART1_IRQHandler() {
    uint16_t word;
    if(USART_GetITStatus(USART1,USART_IT_RXNE)!=RESET){ 
      word = USART_ReceiveData(USART1);
      sendDataUART2(word);
      
      USART_ClearITPendingBit(USART1,USART_IT_RXNE);
    }
}

//UART2 interrupt handler
int idx = 0;
char temp[10];
int usart2 = 0;
void USART2_IRQHandler() { 
    uint16_t word;
    if(USART_GetITStatus(USART2,USART_IT_RXNE)!=RESET){ 
        word = USART_ReceiveData(USART2);
        temp[idx] = word;
        sendDataUART1(temp[idx]);
        idx++;
        USART_ClearITPendingBit(USART2,USART_IT_RXNE);
    }
    usart2 = 1;
}

// Freqeuncy and Pulse setting for servo motor
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
    
    // set center
    TIM_OCInitStructure.TIM_Pulse= 1500;
    TIM_OC3Init(TIM3, &TIM_OCInitStructure);
    TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Disable);
    TIM_ARRPreloadConfig(TIM3, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
}

// servo motor move left to right respectively
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
    TIM_OCInitStructure.TIM_Pulse= 600; //end of left
    TIM_OC3Init(TIM3, &TIM_OCInitStructure);
    TIM_Cmd(TIM3, ENABLE);
    
    motorDelay();
    
    TIM_Cmd(TIM3, DISABLE);
    TIM_OCInitStructure.TIM_Pulse= 2400; //end of right
    TIM_OC3Init(TIM3, &TIM_OCInitStructure);
    TIM_Cmd(TIM3, ENABLE);
    
    motorDelay();
}

// smartphone recieve
char str1[12] = "enter age:\n\r";
char str2[15] = "enter weight:\n\r";
char str3[31] = "enter gender 1.male 2.female:\n\r";
char str5[20]="divide by comma(,)\n\r";
char senmsg[10]="detected\n\r";
char senmis[10]="missing\n\r";

// send message to smartphone
void Input(){
     for(int i = 0;i < 11;i++){
      sendDataUART2(str1[i]);
      Delay();
    }
       for(int i = 0;i < 14;i++){
      sendDataUART2(str2[i]);
      Delay();
    }
       for(int i = 0;i < 31;i++){
      sendDataUART2(str3[i]);
      Delay();
      }
     
     for(int i = 0;i < 19;i++){
      sendDataUART2(str5[i]);
      Delay();
      }
    
  }

char age[20];
char gender[25];
char weight[20];
char timer[15];
char stimer[15];
int main(void)
{
  //Configure & init ------------------------------
  uint16_t x, y, nx, ny;
  int timeFlag = FALSE;
  char cmd;
  SystemInit();
  RCC_Configure();
  GPIO_Configure();
  USART1_Init();
  USART2_Init();
  NVIC_Configure();
  PWM_Configure();
  ADC_Configure();
  DMA_Configure();
  time_t start = time(NULL); //for executing motor 15sec
  time_t sstart = time(NULL);
  //LCD & Touch------------------------------------
  LCD_Init();
  LCD_Clear(WHITE);
  char weightPrompt[30];
  char sensorPrompt[30]; 
  
  Input();
 
  //RUN------------------------------------
  while(1){
    
    if(ADC_Value[0]>900){
     sprintf(weightPrompt, "Weight :   Detected");
     LCD_ShowString(0, 270, weightPrompt, BLACK, WHITE);
    }
    else{
     sprintf(weightPrompt, "Weight : Undetected");
     LCD_ShowString(0, 270, weightPrompt, BLACK, WHITE);
    }
    
     sprintf(weightPrompt, "Weight : %4d",ADC_Value[0]);
     LCD_ShowString(0, 290, weightPrompt, BLACK, WHITE);

    if(ADC_Value[1]>3000){
     sprintf(sensorPrompt, "IR :   Detected");
     LCD_ShowString(0, 250, sensorPrompt, BLACK, WHITE);
    }
    else{
     sprintf(sensorPrompt, "IR : Undetected");
     LCD_ShowString(0, 250, sensorPrompt, BLACK, WHITE);
    }
    
    if(usart2 == 0){     //setting mode
      LCD_ShowString(0,20,"SETTING",RED, WHITE); //let user know this mode.
      timeFlag = FALSE;     //not checking
    }
    else{         //complete mode
      char *ptr = strtok(temp, ",");
      pet.age = atoi(ptr);
      ptr = strtok(NULL,",");
      pet.weight = atoi(ptr);
      ptr= strtok(NULL,",");
      pet.jender = atoi(ptr);
      
      sprintf(age, "Age : %3d", pet.age);
      sprintf(weight, "Weight : %3d", pet.weight);
      if(pet.jender==1){
        sprintf(gender, "Gender :   MALE");
      }
      else if(pet.jender==2){
        sprintf(gender, "Gender : FEMALE");
      }
      
      LCD_ShowString(0,20,"Automatic Feeder",RED, WHITE); 
      
      timeFlag=TRUE;        //checking time
    }
    LCD_ShowString(0,60,age,RED, WHITE); //let user know this mode.
    LCD_ShowString(0,80,gender,RED, WHITE); //let user know this mode.
    if(pet.weight){
      LCD_ShowString(0,100,weight,RED, WHITE); //let user know this mode.
    }
    time_t end = time(NULL);// check the end time
    
    if(timeFlag==TRUE){ //if button is pressed and more than 15sec passed, execute the motor. 
      int diff=(int)(end-start);
      sprintf(timer," TIMER: %2d",diff);
      LCD_ShowString(150, 150, timer, BLACK, WHITE);
      if(diff>30){
        servo_control();
        motorDelay();
        PWM_Configure();
        start=time(NULL);
      }
      
      if((ADC_Value[0]>1000) && (ADC_Value[1]>1000)){
        for(int i = 0;i < 10;i++){
          sendDataUART2(senmsg[i]);
          Delay();
        }
      }
      if(ADC_Value[1]>1000){
        sstart = time(NULL);
      }
      int sdiff= (int)(end-sstart);
      sprintf(stimer,"STIMER: %2d",sdiff);
      LCD_ShowString(150, 180, stimer, BLACK, WHITE);
      if(sdiff>10){
        
        for(int i = 0;i < 10;i++){
          sendDataUART2(senmis[i]);
          Delay();
          sstart=time(NULL);
        }
         
      }

   }
 
   motorDelay();
  }
}