#ifndef I2C_H
#define I2C_H

#include "pinmap.h"

extern void I2C_Init(I2C_TypeDef* I2Cx,
										 pinStruct_t& i2cPin1,
										 pinStruct_t& i2cPin2);

extern void I2C_Write(I2C_TypeDef* I2Cx,
											uint8_t slaveAddr,
											uint8_t data);

extern void I2C_Write(I2C_TypeDef* I2Cx,
											uint8_t slaveAddr,
											uint8_t regAddr,
											uint8_t* pData,
											uint32_t length);

extern void I2C_Read(I2C_TypeDef* I2Cx,
										 uint8_t slaveAddr,
										 uint8_t regAddr,
										 uint8_t* pData,
										 uint32_t length);

#endif //I2C_H
