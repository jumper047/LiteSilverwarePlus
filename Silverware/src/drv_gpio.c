#include "project.h"
#include "drv_gpio.h"
#include "defines.h"

void gpiopa_init(void)
{
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    GPIO_InitTypeDef    GPIO_InitStructure,GPIO_InitStructure1;
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   
    GPIO_Init(GPIOA, &GPIO_InitStructure); 
    
    GPIO_InitStructure1.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure1.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure1.GPIO_PuPd = GPIO_PuPd_UP;    
    GPIO_InitStructure1.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure1); 
    
    
}

void gpio_init(void)
{
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB | RCC_AHBENR_GPIOFEN , ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    
    GPIO_InitTypeDef    GPIO_InitStructure;
     EXTI_InitTypeDef   EXTI_InitStructure;
     NVIC_InitTypeDef   NVIC_InitStructure;
    
    GPIO_InitStructure.GPIO_Pin = LED1PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_Init(LED1PORT, &GPIO_InitStructure); 
    GPIO_SetBits(LED1PORT,LED1PIN);
    
    EXTI_InitStructure.EXTI_Line = EXTI_Line1;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
    
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource1);

        
    NVIC_InitStructure.NVIC_IRQChannel=EXTI0_1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPriority=0x01;
    NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
    NVIC_Init(&NVIC_InitStructure);	

}




#ifdef FPV_ON
// init fpv pin separately because it may use SWDAT/SWCLK don't want to enable it right away
int gpio_init_fpv(void)
{
	// only repurpose the pin after rx/tx have bound
	extern int rxmode;
	if (rxmode == RXMODE_NORMAL)
	{
		// set gpio pin as output
		GPIO_InitTypeDef GPIO_InitStructure;

		// common settings to set ports
      GPIO_InitStructure.GPIO_Pin =  FPV_PIN;
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
      GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
      GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
      GPIO_Init(FPV_PORT,&GPIO_InitStructure);
        
		return 1;
	}
	return 0;
}
#endif




