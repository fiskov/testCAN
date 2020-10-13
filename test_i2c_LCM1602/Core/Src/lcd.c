#include "lcd.h"
//------------------------------------------------
I2C_HandleTypeDef *hi2c_p;
uint8_t portLCD; //  младшие 4 бита при передаче 
uint8_t addr=0x0;

typedef union
{
  uint8_t byte;
  struct 
  {
    uint8_t rs : 1;
    uint8_t rw : 1;
    uint8_t e : 1;
    uint8_t light : 1;
    uint8_t ch : 4;
  } bits;
} byte_union_t;
byte_union_t portLCDu;
//------------------------------------------------
#define   e_set() LCD_WriteByteI2CLCD(portLCD |= 0x04)  
#define e_reset() LCD_WriteByteI2CLCD(portLCD &= ~0x04) //установка линии Е в 0

#define   rs_set() LCD_WriteByteI2CLCD(portLCD |= 0x01) //установка линии RS в 1 - команды
#define rs_reset() LCD_WriteByteI2CLCD(portLCD &= ~0x01) //установка линии RS в 0 - символы 

#define   setled() LCD_WriteByteI2CLCD(portLCD |= 0x08) 
#define resetled() LCD_WriteByteI2CLCD(portLCD &= ~0x08)

#define setwrite() LCD_WriteByteI2CLCD(portLCD &= ~0x02) //будем записывать в экран (считывать не придется)
#define DELAY 1
//------------------------------------------------
void LCD_WriteByteI2CLCD(uint8_t bt)
{
	HAL_I2C_Master_Transmit(hi2c_p,(uint16_t) addr, &bt, 1, 100);
}
//------------------------------------------------
void sendhalfbyte(uint8_t c)
{
	c <<= 4;  
	LCD_WriteByteI2CLCD(portLCD | c | 0x04); //установка линии Е в 1
  LCD_WriteByteI2CLCD(portLCD);
}
//------------------------------------------------
void sendbyte(uint8_t c, uint8_t mode)
{
	if (mode==0) rs_reset(); else rs_set();
	sendhalfbyte(c >> 4);
  sendhalfbyte(c);
}
//------------------------------------------------
void LCD_Clear(void)
{
	sendbyte(0x01,0);
	HAL_Delay(DELAY);
}
//------------------------------------------------
void LCD_SendChar(char ch)
{
	sendbyte(ch,1);
}
//------------------------------------------------
void LCD_String(char* st)
{
  for (uint8_t i=0; st[i]!=0; sendbyte(st[i++], 1));
}
//------------------------------------------------
void LCD_SetPos(uint8_t x, uint8_t y)
{
	switch(y)
	{
		case 0:
			sendbyte(x|0x80,0);
			break;
		case 1:
			sendbyte((0x40+x)|0x80, 0);
			break;
		case 2:
			sendbyte((0x14+x)|0x80, 0);
			break;
		case 3:
			sendbyte((0x54+x)|0x80, 0);
			break;
	}
}
//------------------------------------------------

void LCD_ini(I2C_HandleTypeDef *hi2c0, uint8_t addr_)
{
	hi2c_p = hi2c0;
	addr = addr_;
  
	sendbyte(0x33,0);
	sendbyte(0x32,0);
	HAL_Delay(DELAY);
	sendbyte(0x28,0);//режим 4 бит, 2 линии 	
  HAL_Delay(DELAY);
	sendbyte(0x0C,0);//дисплей включаем (D=1), курсоры никакие не нужны
	HAL_Delay(DELAY);
	sendbyte(0x01,0);//уберем мусор
  HAL_Delay(DELAY);
	sendbyte(0x06,0);//пишем влево
  HAL_Delay(DELAY);
	setled();//подсветка
	setwrite();//передаем данные
}

