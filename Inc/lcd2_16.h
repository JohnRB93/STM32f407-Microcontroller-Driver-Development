#ifndef __LCD2_16_
#define __LCD2_16_

#include<stdint.h>
#include"gpio.h"

// GPIO Pin Macros for the LCD (Port D is used for all)
#define PIN_DB0				GPIO_PIN_NO_0
#define PIN_DB1				GPIO_PIN_NO_1
#define PIN_DB2				GPIO_PIN_NO_2
#define PIN_DB3				GPIO_PIN_NO_3
#define PIN_DB4				GPIO_PIN_NO_4
#define PIN_DB5				GPIO_PIN_NO_5
#define PIN_DB6				GPIO_PIN_NO_6
#define PIN_DB7				GPIO_PIN_NO_7
#define PIN_RW				GPIO_PIN_NO_8
#define PIN_RS				GPIO_PIN_NO_9
#define PIN_E				GPIO_PIN_NO_10

// Row Macros
#define TOP_ROW				0
#define BOTTOM_ROW			1

// LCD Command Macros
#define CLEAR_DISPLAY									(uint16_t)0b0000000001
#define RETURN_HOME										(uint16_t)0b0000000010
#define ENTRY_MODE_SET(ID, SH)							(uint16_t)((0b0000000100) | (ID << 1) | (SH << 0))
#define DISPLAY_ON_OFF(D, C, B)							(uint16_t)((0b0000001000) | (D << 2) | (C << 1) | (B << 0))
#define CURSOR_DISPLAY_SHIFT(SC, RL)					(uint16_t)((0b0000010000) | (SC << 3) | (RL << 2))
#define FUNCTION_SET(DL, N, F)							(uint16_t)((0b0000100000) | (DL << 4) | (N << 3) | (F << 2))
#define SET_CGRAM_ADDRESS(addr)							(uint16_t)((0b0001000000) | (addr))
#define SET_DDRAM_ADDRESS(addr)							(uint16_t)((0b0010000000) | (addr))
#define READ_BUSY_FLAG_AND_ADDRESS						(uint16_t)0b0100000000
#define WRITE_DATA_TO_RAM(data)							(uint16_t)((0b1000000000) | (data))
#define READ_DATA_FROM_RAM								(uint16_t)0b1100000000

// Maximum String Length
#define MAX_STRING_LENGTH		40U
#define LCD_SCREEN_LENGTH		16U

// Format Specifier Condition
#define FRMT_SPCR(p) ((*p == '%') && \
						((*(p+1) == 'd') || \
						 (*(p+1) == 'c') || \
						 (*(p+1) == 'f') || \
						 (*(p+1) == 'u') || \
						 (*(p+1) == 'i') || \
						 (*(p+1) == 's')))

// Format Specifier Enumerator
enum
{
	LCD_FORMAT_BYTE,
	LCD_FORMAT_SHORT,
	LCD_FORMAT_INT,
	LCD_FORMAT_LONG,
	LCD_FORMAT_LONG_LONG,
	LCD_FORMAT_U_BYTE,
	LCD_FORMAT_U_SHORT,
	LCD_FORMAT_U_INT,
	LCD_FORMAT_U_LONG,
	LCD_FORMAT_U_LONG_LONG,
	LCD_FORMAT_FLOAT,
	LCD_FORMAT_DOUBLE,
	LCD_FORMAT_CHARACTER,
	LCD_FORMAT_STRING,
	LCD_FORMAT_POINTER
}typedef lcd_formatter_specifier;


/***************************************************************************************/
/*                         APIs supported by this driver                               */
/***************************************************************************************/

void LCD_Init8Bit(void);
void LCD_SetRowTop(void);
void LCD_SetRowBottom(void);
void LCD_ToggleRow(void);
void LCD_SetDdramAddress(uint8_t _ddramAddr);
uint8_t LCD_GetScrollFlag(void);
void LCD_ScrollDisplay(void);
void LCD_ClearScreen(void);
void LCD_Print(const char* string, ...);




#endif /* __LCD2_16_ */
