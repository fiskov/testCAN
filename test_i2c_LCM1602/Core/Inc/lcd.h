#ifndef LCD_H_
#define LCD_H_
//------------------------------------------------
#include "stm32f4xx_hal.h"

//------------------------------------------------
void LCD_ini(I2C_HandleTypeDef *hi2c0, uint8_t addr_);
void LCD_Clear(void);
void LCD_SendChar(char ch);
void LCD_String(char* st);
void LCD_SetPos(uint8_t x, uint8_t y);
//------------------------------------------------
#endif /* LCD_H_ */
