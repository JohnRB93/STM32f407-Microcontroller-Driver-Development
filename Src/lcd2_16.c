#include<stdio.h>
#include<string.h>
#include<stdarg.h>
#include<stdlib.h>
#include"lcd2_16.h"
#include"tim.h"
#include"gpio.h"



static void LCD_SendCommand8Bit(uint16_t cmd);
static void LCD_SendChar(char ch);
static void LCD_ExecuteCommand(void);
static uint8_t LCD_CheckFormatSpecifiers(const char* str);
static void LCD_ConvertIntToString(int32_t num, char* str);
static void LCD_ConvertUnsignedIntToString(uint32_t num, char* str);
static uint8_t LCD_CountDigits(uint32_t num);
static void LCD_ReverseString(char* str, uint8_t len);
static uint8_t LCD_StrTokenByFormatter(const char* src, char* token, lcd_formatter_specifier* pFormat);
static void LCD_AppendCharToString(char* str, uint32_t intChr);
static void LCD_StrLenErrorMsg(void);

static uint8_t ddramAddr;
static uint8_t scrollFlag;

/***************************************************************************************/


/***************** User Application Exposed Function Definitions ***********************/

/*
 * @fn				- LCD_Init8Bit
 *
 * @brief			- This function initializes the 2x16 LCD Module in 8-bit mode. As part of
 * 				  	  the initialization, the Timer 2 peripheral is also initialized so that
 * 				  	  each command to the LCD module can receive the proper amount of delay;
 *
 * @param[void]		- None.
 *
 * @return[void]	- None.
 *
 * @note			- None.
 */
void LCD_Init8Bit(void)
{
	// Initialize GPIO D pins for the LCD
	GPIO_PinConfig_t lcdPins;
	for(uint8_t i = 0; i < 10; i++)
	{
		lcdPins.GPIO_PinNumber = i;
		lcdPins.GPIO_PinMode = GPIO_MODE_OUT;
		lcdPins.GPIO_PinOPType = GPIO_OP_TYPE_PP;
		lcdPins.GPIO_PinPuPdControl = GPIO_PIN_PD;
		lcdPins.GPIO_PinSpeed = GPIO_SPEED_FAST;
		GPIO_Init(GPIOD, lcdPins);
	}
	GPIOD->MODER = 0x155555;
	// Initialize the TIM2 timer
	TIM2_5_Init(TIM2, PULSE_MODE_DISABLE);
	// Begin the initialization of the LCD
	TIM2_5_Delay_us(TIM2, 15000);
	LCD_SendCommand8Bit(FUNCTION_SET(1, 1, 1));
	TIM2_5_Delay_us(TIM2, 41000);
	LCD_SendCommand8Bit(FUNCTION_SET(1, 1, 1));
	TIM2_5_Delay_us(TIM2, 100);
	LCD_SendCommand8Bit(FUNCTION_SET(1, 1, 1));
	LCD_SendCommand8Bit(FUNCTION_SET(1, 1, 1));
	LCD_SendCommand8Bit(0b0000001000);
	LCD_SendCommand8Bit(0b0000000001);
	TIM2_5_Delay_us(TIM2, 2000);
	LCD_SendCommand8Bit(ENTRY_MODE_SET(0, 0));
	GPIOD->ODR = 0x0;
	// Set DDRAM to 0x00 and set display on
	ddramAddr = 0;
	LCD_SendCommand8Bit(SET_DDRAM_ADDRESS(ddramAddr));
	LCD_SendCommand8Bit(DISPLAY_ON_OFF(1, 0, 0));
	scrollFlag = 0;
}

/*
 * @fn				- LCD_SetRowTop
 *
 * @brief			- This function sets the DDRAM address of the LCD module to the
 * 					  begining of the top row(0x0). This does NOT alter the contents
 * 					  of the addresses.
 *
 * @param[void]		- None.
 *
 * @return[void]	- None.
 *
 * @note			- None.
 */
void LCD_SetRowTop(void)
{
	ddramAddr = 0;
	LCD_SendCommand8Bit(SET_DDRAM_ADDRESS(ddramAddr));
}

/*
 * @fn				- LCD_SetRowBottom
 *
 * @brief			- This function sets the DDRAM address of the LCD module to the
 * 					  begining of the bottom row(0x40). This does NOT alter the contents
 * 					  of the addresses.
 *
 * @param[void]		- None.
 *
 * @return[void]	- None.
 *
 * @note			- None.
 */
void LCD_SetRowBottom(void)
{
	ddramAddr = 0x40;
	LCD_SendCommand8Bit(SET_DDRAM_ADDRESS(ddramAddr));
}

/*
 * @fn				- LCD_ToggleRow
 *
 * @brief			- This function sets the DDRAM address of the LCD module to either top
 * 					  or bottom row, which ever it's currently not on. This does NOT alter
 * 					  the contents of the addresses.
 *
 * @param[void]		- None.
 *
 * @return[void]	- None.
 *
 * @note			- None.
 */
void LCD_ToggleRow(void)
{
	if(ddramAddr >= 0x0 && ddramAddr < 0x40)
	{
		ddramAddr = 0x40;
		LCD_SendCommand8Bit(SET_DDRAM_ADDRESS(ddramAddr));
	}
	else
	{
		ddramAddr = 0x0;
		LCD_SendCommand8Bit(SET_DDRAM_ADDRESS(ddramAddr));
	}

}

/*
 * @fn				- LCD_SetDdramAddress
 *
 * @brief			- This function sets the DDRAM address of the LCD module to the
 * 					  address specified by the provided argument.
 *
 * @param[uint8_t]		- DDRAM address to be set.
 *
 * @return[void]	- None.
 *
 * @note			- None.
 */
void LCD_SetDdramAddress(uint8_t _ddramAddr)
{
	ddramAddr = _ddramAddr;
	LCD_SendCommand8Bit(SET_DDRAM_ADDRESS(ddramAddr));
}

/*
 * @fn				- LCD_GetScrollFlag
 *
 * @brief			- This function returns the scrolling status of the LCD module.
 *
 * @param[void]		- None.
 *
 * @return[uint8_t]	- 0 if not scrolling, else 1.
 *
 * @note			- None.
 */
uint8_t LCD_GetScrollFlag(void)
{
	return scrollFlag;
}

/*
 * @fn				- LCD_ScrollDisplay
 *
 * @brief			- This function sets the LCD module to scrolling mode.
 *
 * @param[void]		- None.
 *
 * @return[void]	- None.
 *
 * @note			- None.
 */
void LCD_ScrollDisplay(void)
{
	LCD_SendCommand8Bit(CURSOR_DISPLAY_SHIFT(1, 0));
	TIM2_5_Delay_us(TIM2, 500000);
}

/*
 * @fn				- LCD_ClearScreen
 *
 * @brief			- This function clears the screen of the LCD module.
 *
 * @param[void]		- None.
 *
 * @return[void]	- None.
 *
 * @note			- This clears the contents of the DDRAM addresses.
 */
void LCD_ClearScreen(void)
{
	LCD_SendCommand8Bit(CLEAR_DISPLAY);
	TIM2_5_Delay_us(TIM2, 1600);
}

/*
 * @fn				- LCD_Print
 *
 * @brief			- This function displays the contents passed through the str argument.
 * 					  The functionality of this function is similar to how printf() works.
 * 					  You must pass a string, and if you want to insert values into that
 * 					  string, you must use format specifiers (%d, %c, %s...) and pass a
 * 					  variable number of arguments according to each format specifier.
 * 					  Ex. LCD_Print("My value: %d", 42);
 * 					  // Prints "My value: 42"
 *
 * @param[const char*]	- String to be displayed.
 * @param[...]			- Variable arguments to be inserted into the string to be displayed.
 *
 * @return[void]	- None.
 *
 * @note			- As of now, only %d, %i, %u, %c, and %s works.
 */
void LCD_Print(const char* str, ...)
{
	// Get string length and check if the string is too long
	uint8_t len = (uint8_t)strlen(str);
	if(len > MAX_STRING_LENGTH)
	{
		LCD_StrLenErrorMsg();
		return;
	}
	//Check string for any format specifiers
	if(!LCD_CheckFormatSpecifiers(str))
	{	// No specifiers found
		// Send String normally
		LCD_SendCommand8Bit(SET_DDRAM_ADDRESS(ddramAddr));
		for(uint8_t i = 0; i < len; i++)
			LCD_SendChar(str[i]);
		// Check if scrolling is needed to see all characters on LCD screen
		if((uint8_t)strlen(str) > LCD_SCREEN_LENGTH)
			scrollFlag = 1;
		return;
	}

	// Create a copy of the string to send to LCD_SendChar()
	char outputStr[MAX_STRING_LENGTH];
	strcpy(outputStr, str);

	// Prepare variable arguments
	va_list args;
	va_start(args, str);
	char numStr[9];

	// Tokenize the string by each format specifier
	lcd_formatter_specifier frmtSpcr[1];
	uint8_t cnt = 0;
	char strToken[40];

	while(LCD_StrTokenByFormatter(str, strToken, frmtSpcr))
	{
		if(cnt == 0)
		{// For first iteration, copy the contents of the token to the output string
			strcpy(outputStr, strToken);
		}
		else // For subsequent iterations, concatinate the token to the output string
			strcat(outputStr, strToken);

		switch(*frmtSpcr)
		{
			case LCD_FORMAT_BYTE:
			case LCD_FORMAT_SHORT:
			case LCD_FORMAT_INT:
				LCD_ConvertIntToString(va_arg(args, int32_t), numStr);
				LCD_ReverseString(numStr, (uint8_t)strlen(numStr));
				strcat(outputStr, numStr);
				break;
			case LCD_FORMAT_LONG: break;
			case LCD_FORMAT_LONG_LONG: break;
			case LCD_FORMAT_U_BYTE:
			case LCD_FORMAT_U_SHORT:
			case LCD_FORMAT_U_INT:
				LCD_ConvertUnsignedIntToString(va_arg(args, uint32_t), numStr);
				LCD_ReverseString(numStr, (uint8_t)strlen(numStr));
				strcat(outputStr, numStr);
				break;
			case LCD_FORMAT_U_LONG: break;
			case LCD_FORMAT_U_LONG_LONG: break;
			case LCD_FORMAT_FLOAT: break;
			case LCD_FORMAT_DOUBLE: break;
			case LCD_FORMAT_CHARACTER:
				LCD_AppendCharToString(outputStr, va_arg(args, uint32_t));
				break;
			case LCD_FORMAT_STRING:
				strcat(outputStr, va_arg(args, char*));
				break;
			case LCD_FORMAT_POINTER: break;
		}
		cnt++;
	}
	// Concatinate remaining token to the output string
	strcat(outputStr, strToken);
	// Clean up the arguments
	va_end(args);
	// Release the dynamically allocated memory
	//free(frmtSpcrs);
	//free(strToken);
	// Check if the string length is too long, if so, exit with an error message on the LCD
	if((uint8_t)strlen(outputStr) > MAX_STRING_LENGTH)
	{
		LCD_StrLenErrorMsg();
		return;
	}
	// Send the output string to the LCD
	LCD_SendCommand8Bit(SET_DDRAM_ADDRESS(ddramAddr));
	for(char* p = outputStr; *p != '\0'; p++)
		LCD_SendChar(*p);
	// Check if scrolling is needed to see all characters on LCD screen
	if((uint8_t)strlen(outputStr) > LCD_SCREEN_LENGTH)
		scrollFlag = 1;
}


/***************************************************************************************/


/***************** Private Helper Function Definitions *********************************/

static void LCD_SendCommand8Bit(uint16_t cmd)
{
	for(uint8_t i = 0; i < 10; i++)
		GPIO_WriteToOutputPin(GPIOD, i, ((cmd >> i) & 0x1));
	LCD_ExecuteCommand();
}

static void LCD_SendChar(char ch)
{
	// Check for escape characters
	if(ch == '\n')
	{
		LCD_ToggleRow();
		return;
	}
	if(ch == '\t')
	{
		if(ddramAddr + 4 > 0x7F)
		{
			ddramAddr = 0;
			LCD_SendCommand8Bit(SET_DDRAM_ADDRESS(ddramAddr));
		}
		ddramAddr += 4;
		LCD_SendCommand8Bit(SET_DDRAM_ADDRESS(ddramAddr));
		return;
	}
	LCD_SendCommand8Bit(WRITE_DATA_TO_RAM((uint16_t)ch));
	LCD_SendCommand8Bit(SET_DDRAM_ADDRESS(++ddramAddr));
}

static void LCD_ExecuteCommand(void)
{
	GPIO_WriteToOutputPin(GPIOD, PIN_E, SET);
	TIM2_5_Delay_us(TIM2, 100);
	GPIO_WriteToOutputPin(GPIOD, PIN_E, RESET);
	TIM2_5_Delay_us(TIM2, 100);
}

static uint8_t LCD_CheckFormatSpecifiers(const char* str)
{
	for(const char* p = str; *p != '\0'; p++)
		if(FRMT_SPCR(p))
			return 1;

	return 0;
}

static void LCD_ConvertIntToString(int32_t num, char* str)
{
	int32_t tmp = num;
	uint8_t count = 0;
	if(tmp < 0)
	{	// Number is negative
		tmp *= -1;
		*str = '-';
		str++;
		count = LCD_CountDigits(tmp) + 1;
	}
	else // Number is positive
		count = LCD_CountDigits(tmp);

	for(uint8_t i = 0; i < count; i++)
	{
		*str = (tmp % 10) + 48;
		str++;
		tmp /= 10;
	}
	*str = '\0';
	str -= count;
}

static void LCD_ConvertUnsignedIntToString(uint32_t num, char* str)
{
	uint32_t tmp = num;
	uint8_t count = LCD_CountDigits(tmp);
	for(uint8_t i = 0; i < count; i++)
	{
		*str = (tmp % 10) + 48;
		str++;
		tmp /= 10;
	}
	*str = '\0';
	str -= count;
}

static uint8_t LCD_CountDigits(uint32_t num)
{
	if(num == 0)
		return 1;

	uint8_t count = 0;
	while (num > 0)
	{
		num /= 10;
		count++;
	}
	return count;
}

static void LCD_ReverseString(char* str, uint8_t len)
{
	char tmpStr[len];
	strcpy(tmpStr, str);
	for(uint8_t i = 0; i < len; i++)
		str[(len-1) - i] = tmpStr[i];
}

static uint8_t LCD_StrTokenByFormatter(const char* src, char* token, lcd_formatter_specifier* pFormat)
{
	static uint8_t i = 0;
	uint8_t j = 0;
	const char* p = src + i;
	while(*p != '\0')
	{
		if(FRMT_SPCR(p))
		{
			switch(*(p+1))
			{
				case 'i':
				case 'd': *pFormat = LCD_FORMAT_INT;       break;
				case 'u': *pFormat = LCD_FORMAT_U_INT;     break;
				case 'f': *pFormat = LCD_FORMAT_FLOAT;     break;
				case 'c': *pFormat = LCD_FORMAT_CHARACTER; break;
				case 's': *pFormat = LCD_FORMAT_STRING;    break;
				case 'p': *pFormat = LCD_FORMAT_POINTER;   break;
			}
			token[j] = '\0';
			i += 2;
			return 1;
		}
		token[j] = src[i];
		p++;
		i++;
		j++;
	}
	token[j] = '\0';
	i = 0;
	return 0;
}

static void LCD_AppendCharToString(char* str, uint32_t intChr)
{
	char* p = str;
	uint8_t i = 0;
	while(*p != '\0')
	{
		p++;
		i++;
	}

	str[i] = intChr;
	str[i+1] = '\0';
}

static void LCD_StrLenErrorMsg(void)
{
	LCD_SendCommand8Bit(CLEAR_DISPLAY);
	TIM2_5_Delay_us(TIM2, 1600);
	LCD_SetRowTop();
	char* errorMsg = "String too long.";
	for(uint8_t i = 0; i < (uint8_t)strlen(errorMsg); i++)
		LCD_SendChar((char)errorMsg[i]);
	LCD_SetRowBottom();
	errorMsg = "Send string less than 40.";
	for(uint8_t i = 0; i < (uint8_t)strlen(errorMsg); i++)
		LCD_SendChar((char)errorMsg[i]);
	TIM2_5_Delay_us(TIM2, 1000000);
	scrollFlag = 1;
}
