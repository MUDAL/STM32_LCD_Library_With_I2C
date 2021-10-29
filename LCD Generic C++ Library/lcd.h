#ifndef LCD_H
#define LCD_H

#include "pinmap.h"
#include "i2c.h"

/**
@brief LCD library. Works for LCD with or without an   
I2C adapter(PCF8574T). The PCF8574T has a default I2C  
address of 0x27.  
*/

class LCD
{
	private:
		uint8_t lcdType;
		uint8_t maxRowIndex;
		uint8_t maxColIndex;
		I2C_TypeDef* i2cPort;
		uint8_t i2cAddr;
		uint8_t i2cData;
		pinStruct_t rs;
		pinStruct_t en; 
		pinStruct_t dataPins[4];
		void WriteNibble(char byte,uint8_t nibbleBitPos);
		void WriteByte(GPIO_PinState lcdMode,char byte);
		void WriteBytes(const char* pData);
		void WriteInteger(uint32_t data);
		void Init(void);
	
	public:
		LCD(pinStruct_t* pLCDPins,uint8_t numberOfRows,uint8_t numberOfColumns);
		LCD(I2C_TypeDef* I2Cx,uint8_t i2cDevAddr,uint8_t numberOfRows,uint8_t numberOfColumns);
		void SetCursor(uint8_t row,uint8_t column);
		void Print(char data);
		void Print(const char* pData);
		void Print(uint8_t& data);
		void Print(uint16_t& data);
		void Print(uint32_t& data);
		void Clear(void);
};

#endif //LCD_H
