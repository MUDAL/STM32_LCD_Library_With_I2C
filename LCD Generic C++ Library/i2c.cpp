#include "i2c.h"

//Static functions
static void I2C_ReadByte(I2C_TypeDef* I2Cx,
												 uint8_t slaveAddr,
									       uint8_t regAddr,
									       uint8_t* pData)
{
	volatile uint32_t read_I2C_SR2;
	
	while ((I2Cx->SR2 & I2C_SR2_BUSY) == I2C_SR2_BUSY); //wait for I2C busy bit to be cleared 
	I2Cx->CR1 |= I2C_CR1_START; //Generate start condition
	while((I2Cx->SR1 & I2C_SR1_SB) != I2C_SR1_SB);//wait for start bit to be set
	I2Cx->DR = slaveAddr << 1; //slave address
	while((I2Cx->SR1 & I2C_SR1_ADDR) != I2C_SR1_ADDR); //wait for ADDR bit to be set
	read_I2C_SR2 = I2Cx->SR2;
	while((I2Cx->SR1 & I2C_SR1_TXE) != I2C_SR1_TXE); //wait for TXE bit to be set
	I2Cx->DR = regAddr;
		
	while((I2Cx->SR1 & I2C_SR1_TXE) != I2C_SR1_TXE); 
	I2Cx->CR1 |= I2C_CR1_START; 
	while((I2Cx->SR1 & I2C_SR1_SB) != I2C_SR1_SB);
	I2Cx->DR = slaveAddr << 1 | 1;
	
	while((I2Cx->SR1 & I2C_SR1_ADDR) != I2C_SR1_ADDR); 
	I2Cx->CR1 &= ~I2C_CR1_ACK;//Send NACK
	read_I2C_SR2 = I2Cx->SR2;
		
	I2Cx->CR1 |= I2C_CR1_STOP; //Send STOP
	while((I2Cx->SR1 & I2C_SR1_RXNE) != I2C_SR1_RXNE); //Wait for RXNE bit to be set
	*pData = I2Cx->DR;
}

static void I2C_Read2Bytes(I2C_TypeDef* I2Cx,
													 uint8_t slaveAddr,
										       uint8_t regAddr,
										       uint8_t* pData)
{
	volatile uint32_t read_I2C_SR2;
	
	while ((I2Cx->SR2 & I2C_SR2_BUSY) == I2C_SR2_BUSY); //wait for I2C busy bit to be cleared 
	I2Cx->CR1 |= I2C_CR1_START; //Generate start condition
	while((I2Cx->SR1 & I2C_SR1_SB) != I2C_SR1_SB);//wait for start bit to be set
			
	I2Cx->DR = slaveAddr << 1; //slave address
	while((I2Cx->SR1 & I2C_SR1_ADDR) != I2C_SR1_ADDR); //wait for ADDR bit to be set
	read_I2C_SR2 = I2Cx->SR2;
	while((I2Cx->SR1 & I2C_SR1_TXE) != I2C_SR1_TXE);//wait for TXE bit to be set
	I2Cx->DR = regAddr;
		
	while((I2Cx->SR1 & I2C_SR1_TXE) != I2C_SR1_TXE); 
	I2Cx->CR1 |= I2C_CR1_START; 
	while((I2Cx->SR1 & I2C_SR1_SB) != I2C_SR1_SB);
	I2Cx->DR = slaveAddr << 1 | 1;
	
	while((I2Cx->SR1 & I2C_SR1_ADDR) != I2C_SR1_ADDR); 
	I2Cx->CR1 &= ~I2C_CR1_ACK;//Send NACK
	I2Cx->CR1 |= I2C_CR1_POS; 
	read_I2C_SR2 = I2Cx->SR2;
		
	while((I2Cx->SR1 & I2C_SR1_BTF) != I2C_SR1_BTF);//Wait for BTF bit to be set
	I2Cx->CR1 |= I2C_CR1_STOP; //Send STOP
	pData[0] = I2Cx->DR;
	pData[1] = I2Cx->DR;
}

static void I2C_Read3BytesMin(I2C_TypeDef* I2Cx,
															uint8_t slaveAddr,
															uint8_t regAddr,
															uint8_t* pData,
															uint32_t length)
{
	if(length < 3)
	{
		//Invalid
		return;
	}
	volatile uint32_t read_I2C_SR2;
	while ((I2Cx->SR2 & I2C_SR2_BUSY) == I2C_SR2_BUSY); //wait for I2C busy bit to be cleared
	I2Cx->CR1 |= I2C_CR1_START; //Generate start condition
	while((I2Cx->SR1 & I2C_SR1_SB) != I2C_SR1_SB); //wait for start bit to be set
			
	I2Cx->DR = slaveAddr << 1; //slave address
	while((I2Cx->SR1 & I2C_SR1_ADDR) != I2C_SR1_ADDR); //wait for ADDR bit to be set
	read_I2C_SR2 = I2Cx->SR2;
	while((I2Cx->SR1 & I2C_SR1_TXE) != I2C_SR1_TXE); //wait for TXE bit to be set
	I2Cx->DR = regAddr;
		
	while((I2Cx->SR1 & I2C_SR1_TXE) != I2C_SR1_TXE); 
	I2Cx->CR1 |= I2C_CR1_START; 
	while((I2Cx->SR1 & I2C_SR1_SB) != I2C_SR1_SB); 
	I2Cx->DR = slaveAddr << 1 | 1;
	
	while((I2Cx->SR1 & I2C_SR1_ADDR) != I2C_SR1_ADDR); 
	read_I2C_SR2 = I2Cx->SR2;
	I2Cx->CR1 |= I2C_CR1_ACK; //Send ACK
	
	//Read incoming data
	for (uint32_t i = 0; i < length - 3; i++)
	{
		while((I2Cx->SR1 & I2C_SR1_RXNE) != I2C_SR1_RXNE); //Wait for RXNE bit to be set
		pData[i] = I2Cx->DR;
	}
	
	while((I2Cx->SR1 & I2C_SR1_BTF) != I2C_SR1_BTF);//Wait for BTF bit to be set
	I2Cx->CR1 &= ~I2C_CR1_ACK;//Send NACK	
	pData[length - 3] = I2Cx->DR;
		
	while((I2Cx->SR1 & I2C_SR1_BTF) != I2C_SR1_BTF);
	I2Cx->CR1 |= I2C_CR1_STOP; //Send STOP
	pData[length - 2] = I2Cx->DR;
	pData[length - 1] = I2Cx->DR;	
}

//External functions
void I2C_Init(I2C_TypeDef* I2Cx,
							pinStruct_t& i2cPin1,
							pinStruct_t& i2cPin2)
{
	GPIO_InitTypeDef i2cGPIO1InitStruct = {0};
	i2cGPIO1InitStruct.Pin = i2cPin1.selectedPin;
	i2cGPIO1InitStruct.Mode = GPIO_MODE_AF_OD;
	i2cGPIO1InitStruct.Pull = GPIO_PULLUP;
	i2cGPIO1InitStruct.Alternate = 0x04; //alternate function I2C
	HAL_GPIO_Init(i2cPin1.port,&i2cGPIO1InitStruct);
	
	GPIO_InitTypeDef i2cGPIO2InitStruct = {0};
	i2cGPIO2InitStruct.Pin = i2cPin2.selectedPin;
	i2cGPIO2InitStruct.Mode = GPIO_MODE_AF_OD;
	i2cGPIO2InitStruct.Pull = GPIO_PULLUP;
	i2cGPIO2InitStruct.Alternate = 0x04; //alternate function I2C
	HAL_GPIO_Init(i2cPin2.port,&i2cGPIO2InitStruct);
	
	I2C_HandleTypeDef hi2c;
	hi2c.Instance = I2Cx;
	hi2c.Init.ClockSpeed = 100000;
	hi2c.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	HAL_I2C_Init(&hi2c);	
}

void I2C_Write(I2C_TypeDef* I2Cx,
							 uint8_t slaveAddr,
							 uint8_t data)
{
	volatile uint32_t read_I2C_SR2;
	
	while ((I2Cx->SR2 & I2C_SR2_BUSY) == I2C_SR2_BUSY); //wait for I2C busy bit to be cleared	 
	I2Cx->CR1 |= I2C_CR1_START; //Generate start condition
	while((I2Cx->SR1 & I2C_SR1_SB) != I2C_SR1_SB);//wait for start bit to be set
	I2Cx->DR = slaveAddr << 1; //slave address
	while((I2Cx->SR1 & I2C_SR1_ADDR) != I2C_SR1_ADDR);//wait for ADDR bit to be set
	read_I2C_SR2 = I2Cx->SR2;
	while((I2Cx->SR1 & I2C_SR1_TXE) != I2C_SR1_TXE);//wait for TXE bit to be set
	I2Cx->DR = data;
	
	while(((I2Cx->SR1 & I2C_SR1_TXE) != I2C_SR1_TXE) || 
				 ((I2Cx->SR1 & I2C_SR1_BTF) != I2C_SR1_BTF)); 
	I2Cx->CR1 |= I2C_CR1_STOP; 	
}

void I2C_Write(I2C_TypeDef* I2Cx,
							 uint8_t slaveAddr,
							 uint8_t regAddr,
							 uint8_t* pData,
							 uint32_t length)
{
	volatile uint32_t read_I2C_SR2;
	
	while ((I2Cx->SR2 & I2C_SR2_BUSY) == I2C_SR2_BUSY); //wait for I2C busy bit to be cleared	 
	I2Cx->CR1 |= I2C_CR1_START; //Generate start condition
	while((I2Cx->SR1 & I2C_SR1_SB) != I2C_SR1_SB);//wait for start bit to be set
	I2Cx->DR = slaveAddr << 1; //slave address
	while((I2Cx->SR1 & I2C_SR1_ADDR) != I2C_SR1_ADDR);//wait for ADDR bit to be set
	read_I2C_SR2 = I2Cx->SR2;
	while((I2Cx->SR1 & I2C_SR1_TXE) != I2C_SR1_TXE);//wait for TXE bit to be set
	I2Cx->DR = regAddr;
	
	for (uint32_t i = 0; i < length; i++)
	{
		while((I2Cx->SR1 & I2C_SR1_TXE) != I2C_SR1_TXE); 
		I2Cx->DR = pData[i];
	}
	
	while(((I2Cx->SR1 & I2C_SR1_TXE) != I2C_SR1_TXE) || 
				 ((I2Cx->SR1 & I2C_SR1_BTF) != I2C_SR1_BTF)); 
	I2Cx->CR1 |= I2C_CR1_STOP; 	
}

void I2C_Read(I2C_TypeDef* I2Cx,
						  uint8_t slaveAddr,
						  uint8_t regAddr,
						  uint8_t* pData,
						  uint32_t length)
{
	switch(length)
	{
		case 0:
			//Invalid
			return;
		case 1:
			I2C_ReadByte(I2Cx,slaveAddr,regAddr,pData);
			break;
		case 2:
			I2C_Read2Bytes(I2Cx,slaveAddr,regAddr,pData);
			break;
		default:
			I2C_Read3BytesMin(I2Cx,slaveAddr,regAddr,pData,length);
			break;
	}	
}
