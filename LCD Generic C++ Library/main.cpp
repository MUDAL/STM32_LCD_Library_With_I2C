#include "lcd.h"

int main(void)
{
	HAL_Init();
	__HAL_RCC_GPIOB_CLK_ENABLE();	
	__HAL_RCC_I2C1_CLK_ENABLE();
	
	pinStruct_t SCL = {GPIOB,GPIO_PIN_8};
	pinStruct_t SDA = {GPIOB,GPIO_PIN_9};
	I2C_Init(I2C1,SDA,SCL);
	
	static LCD lcd(I2C1,0x27,2,16); 
	
	lcd.Print("I like football");
	HAL_Delay(1500);
	lcd.Clear();
	lcd.Print("What's up");
	HAL_Delay(1500);
	
	while(1)
	{
	}
}

extern "C" void SysTick_Handler(void)
{
  HAL_IncTick();
}

