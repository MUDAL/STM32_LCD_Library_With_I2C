#include "lcd.h"

namespace fourBitCmd
{
	enum
	{
		FUNCTION_SET_8BIT	= 0x03,
		FUNCTION_SET_4BIT = 0x02,
		FUNCTION_SET_2LINE_5x8DOT = 0x28,
		CLEAR_DISPLAY = 0x01,
		DISPLAY_ON_CURSOR_ON = 0x0E,
		DISPLAY_ON_CURSOR_OFF = 0x0C,
		ENTRY_MODE_INCREMENT_CURSOR = 0x06
	};
};

enum PCF8574_Ports
{
	P0,P1,P2,P3,P4,P5,P6,P7
};

/**
@brief LCD pins:
- Register Select (RS)
- Read/Write (RW)
- Enable (EN)
- BackLight Anode (BL)
- Data pins (D4-D7)
*/
enum LCD_Pins
{
	RS = P0, RW = P1, EN = P2, BL = P3,
	D4 = P4, D5 = P5, D6 = P6, D7 = P7
};

enum LCDType
{
	LCD_NO_I2C = 0, /** LCD with no I2C adapter*/
	LCD_I2C /** LCD with I2C adapter (PCF8574T)*/
};

enum NibbleBitPosition
{
	LOW_NIBBLE = 0,
	HIGH_NIBBLE = 4
};

const uint8_t ddramAddr[4][20] = 
{{0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,0x13},
 {0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,0x50,0x51,0x52,0x53},
 {0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27},
 {0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67}
};

/**
@brief Converts integer to string.  
@param[in] integer: Integer to be converted to a string.  
@param[out] pBuffer: String equivalent of the integer. i.e.  
after the function call, the result of the conversion is  
stored here.
@return None
*/
static void IntegerToString(uint32_t integer,char* pBuffer)
{
	if (integer == 0)
	{//Edge case  
		pBuffer[0] = '0';
		return;
	}
	uint32_t copyOfInt = integer;
	uint8_t noOfDigits = 0;

	while(copyOfInt > 0)
	{
		copyOfInt /= 10;
		noOfDigits++;
	}
	while (integer > 0)
	{
		pBuffer[noOfDigits - 1] = '0' + (integer % 10);
		integer /= 10;
		noOfDigits--;
	}
}

void LCD::WriteNibble(char byte,uint8_t nibbleBitPos)
{
	if(lcdType == LCD_NO_I2C)
	{
		const GPIO_PinState pinState[2] = {GPIO_PIN_RESET,GPIO_PIN_SET};
		uint8_t nibbleArr[4] = {0};
		uint8_t j = 0;

		for(uint8_t i = nibbleBitPos; i < nibbleBitPos+4; i++)
		{
			nibbleArr[j] = (byte&(1<<i))>>i;
			j++;
		}
		//Send nibble
		for(uint8_t i = 0; i < 4; i++)
		{
			HAL_GPIO_WritePin(dataPins[i].port,
												dataPins[i].selectedPin,
												pinState[nibbleArr[i]]);
		}
		//High to low pulse on EN pin (to transfer nibble)
		HAL_GPIO_WritePin(en.port,en.selectedPin,GPIO_PIN_SET);
		HAL_Delay(1);
		HAL_GPIO_WritePin(en.port,en.selectedPin,GPIO_PIN_RESET);
		HAL_Delay(1);	
	}
	else
	{
		uint8_t nibble = byte >> nibbleBitPos;
		//Send nibble (P4 to P7)
		i2cData &= ~0xF0;
		i2cData |= (nibble << 4);
		//High to low pulse on EN pin
		i2cData |= (1<<EN);
		I2C_Write(i2cPort,i2cAddr,i2cData);
		HAL_Delay(1);
		i2cData &= ~(1<<EN);
		I2C_Write(i2cPort,i2cAddr,i2cData);
		HAL_Delay(1);
	}
}

void LCD::WriteByte(GPIO_PinState lcdMode,char byte)
{
	if(lcdType == LCD_NO_I2C)
	{
		HAL_GPIO_WritePin(rs.port,rs.selectedPin,lcdMode);//Register select
	}
	else
	{
		//Register select
		i2cData &= ~(1<<RS);
		i2cData |= (lcdMode<<RS);
	}
	LCD::WriteNibble(byte,HIGH_NIBBLE);
	LCD::WriteNibble(byte,LOW_NIBBLE);
}

void LCD::WriteBytes(const char* pData)
{
	while(*pData != '\0')
	{
		LCD::WriteByte(GPIO_PIN_SET,*pData);
		pData++;
	}	
}

void LCD::WriteInteger(uint32_t data)
{
	const uint8_t maxNumberOfDigits = 10;
	char integerToStringBuffer[maxNumberOfDigits] = {0};
	
	if(data < 10)
	{
		LCD::WriteByte(GPIO_PIN_SET,'0');
	}
	IntegerToString(data,integerToStringBuffer);
	LCD::WriteBytes(integerToStringBuffer);		
}

void LCD::Init(void)
{
	//LCD Initialization sequence according to datasheet
	HAL_Delay(16); //Power-on delay (must be greater than 15ms for 4.5v and 40ms for 2.7v)
	LCD::WriteByte(GPIO_PIN_RESET,fourBitCmd::FUNCTION_SET_8BIT);
	HAL_Delay(5); //wait for more than 4.1ms
	LCD::WriteByte(GPIO_PIN_RESET,fourBitCmd::FUNCTION_SET_8BIT);
	HAL_Delay(1); //wait for more than 100us
	//4-bit operation commands
	uint8_t fourBitCommand[5] =
	{
		fourBitCmd::FUNCTION_SET_4BIT,
		fourBitCmd::FUNCTION_SET_2LINE_5x8DOT,
		fourBitCmd::CLEAR_DISPLAY,
		fourBitCmd::DISPLAY_ON_CURSOR_OFF,
		fourBitCmd::ENTRY_MODE_INCREMENT_CURSOR
	};
	for(uint8_t i = 0; i < 5; i++)
	{
		LCD::WriteByte(GPIO_PIN_RESET,fourBitCommand[i]);
	}	
}

LCD::LCD(pinStruct_t* pLCDPins,
				 uint8_t numberOfRows,
				 uint8_t numberOfColumns)
{
	lcdType = LCD_NO_I2C;
	maxRowIndex = numberOfRows - 1;
	maxColIndex = numberOfColumns - 1;
	rs = pLCDPins[0];
	en = pLCDPins[1];
	dataPins[0] = pLCDPins[2];
	dataPins[1] = pLCDPins[3];
	dataPins[2] = pLCDPins[4];
	dataPins[3] = pLCDPins[5];
	LCD::Init();
}

LCD::LCD(I2C_TypeDef* I2Cx,
				 uint8_t i2cDevAddr,
				 uint8_t numberOfRows,
				 uint8_t numberOfColumns)
{
	lcdType = LCD_I2C;
	i2cPort = I2Cx;
	i2cAddr = i2cDevAddr;
	i2cData = 1<<BL; //turn backlight on 
	maxRowIndex = numberOfRows - 1;
	maxColIndex = numberOfColumns - 1;
	LCD::Init();
}

void LCD::SetCursor(uint8_t row,uint8_t column)
{
	if((row > maxRowIndex)||(column > maxColIndex))
	{
		//Out of range
		return;
	}
	/*
	Set DB7 and write address into D4-D7 to set DDRAM address
	*/
	LCD::WriteByte(GPIO_PIN_RESET,((1<<7) | ddramAddr[row][column]));
}

void LCD::Print(char data)
{
	LCD::WriteByte(GPIO_PIN_SET,data);
}

void LCD::Print(const char* pData)
{
	LCD::WriteBytes(pData);
}

void LCD::Print(uint8_t& data)
{
	LCD::WriteInteger(data);
}

void LCD::Print(uint16_t& data)
{
	LCD::WriteInteger(data);
}

void LCD::Print(uint32_t& data)
{
	LCD::WriteInteger(data);
}
	
void LCD::Clear(void)
{
	LCD::WriteByte(GPIO_PIN_RESET,fourBitCmd::CLEAR_DISPLAY);
}
