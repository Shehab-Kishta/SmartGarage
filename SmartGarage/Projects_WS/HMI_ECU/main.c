/******************************************************************************
 *
 * File Name: main.c
 *
 * Description: Source file for the HMI_ECU
 *
 * Author: Shehab Kishta
 *
 *******************************************************************************/
#include <avr/io.h>
#include "lcd.h"
#include "keypad.h"
#include "uart.h"
#include "common_macros.h"
#include "std_types.h"
#include "timer.h"
#include <util/delay.h>

/*******************************************************************************
 *                                Definitions                                  *
 *******************************************************************************/
#define ROW_ZERO									0
#define ROW_ONE										1
#define COLUMN_ZERO									0
#define COLUMN_TEN									10
#define PASSWORD_SIZE                               5
#define READY                                       0xFF
#define DONE                                        0xFE
#define PASSWORD_SEND                               0xFD
#define PASSWORD_MATCH                              0xFC
#define PASSWORD_NOT_MATCHED                        0xFB
#define PASSWORD_CONFIRMATION_SEND                  0xFA
#define CHANGE_PASSWORD                             0xF9
#define OPEN_DOOR                                   0xF8
#define CHECK_PASSWORD                              0xF7
#define WRONG_PASSWORD                              0xF6
#define CHECK_IF_SAVED                              0xF5
#define YES_SAVED                                   0xF4
#define NO_SAVED_PASSWORD                           0xF3
#define MAX_WRONG_COUNTER                           3
#define DC_ON_TICKS                                 16
#define DC_HOLD_TICKS                               19
#define TIMER_TICKS_STOP                            3
#define TIMER_TICKS_1MINUTE                         60
#define TIMER_TOTAL_TICKS							33

/*******************************************************************************
 *                             Global Variables                                *
 *******************************************************************************/
static volatile uint8 g_flag=0;
static volatile uint8  g_done;
uint8 g_key;                                  /*global variable to store the key value */
uint8 g_password[PASSWORD_SIZE];              /*global array to store the password */
uint8 command;                                /*global variable to store the commands */
uint8 g_wrong=0;                              /*global variable to count wrong password entered times */
uint8 g_tick=0;                               /*global ticks to count timer seconds */

UART_ConfigType UART_configuration = {EIGHT, DISABLED, ONE, 9600};
Timer1_ConfigType TIMER_configuration= {0, 7812,F_CPU_1024,Compare};

/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/
void Password_creatAndStore(void);
void Password_send(uint8 a_arr[]);
void Command_send(uint8 command);
uint8 Command_recieve(void);
void Main_options(void);
void Password_fillIn(uint8 a_arr[]);
void Password_wrongScreen(void);
void Alert(void);
void Door_isOpeningClosing(void);

int main(void)
{
	LCD_init(); /* Initialize the LCD */

	UART_init(&UART_configuration); /* Initialize the UART with configurations */

	SREG |= (1<<7);       /* Enable I-Bit for Interrupts */

	Password_creatAndStore();
	while(1)
	{
		Main_options();
	}
}
/*
 * Description
 * Functions that responsible for Creating, Sending and Storing the input password.
 */
void Password_creatAndStore(void)
{
	g_flag = 0;
	while(g_flag != 3)
	{
		LCD_clearScreen();
		LCD_displayString("PLZ Enter PASS:");
		LCD_moveCursor(ROW_ONE,COLUMN_ZERO);
		Password_fillIn(g_password);
		Command_send(PASSWORD_SEND);
		Password_send(g_password);

		LCD_clearScreen();
		LCD_displayString("PLZ Re-Enter the");
		LCD_moveCursor(ROW_ONE,COLUMN_ZERO);
		LCD_displayString("Same PASS:");
		LCD_moveCursor(ROW_ONE,COLUMN_TEN);
		Password_fillIn(g_password);
		Command_send(PASSWORD_CONFIRMATION_SEND);
		Password_send(g_password);

		switch(Command_recieve())
		{
		case PASSWORD_MATCH:
			g_flag=3;
			break;

		case PASSWORD_NOT_MATCHED:
			g_flag=0;
			break;
		}
	}
}
/*
 * Description
 * Functions that responsible for Sending the input password.
 */
void Password_send(uint8 a_arr[])
{
	uint8 i;
	UART_sendByte(READY);
	while(UART_recieveByte() != READY);

	for(i=0 ; i<PASSWORD_SIZE ; i++)
	{
		UART_sendByte(a_arr[i]);
	}
}
/*
 * Description
 * Functions that responsible for Sending the Command.
 */
void Command_send(uint8 command)
{
	UART_sendByte(READY);
	while(UART_recieveByte() != READY);
	UART_sendByte(command);
}
/*
 * Description
 * Functions that responsible for Receiving the Command.
 */
uint8 Command_recieve(void)
{
	while(UART_recieveByte() != READY);
	UART_sendByte(READY);
	command = UART_recieveByte();
	return command;
}
/*
 * Description
 * Functions that responsible for:
 * 1- Open Door.
 * 2- Change Password.
 */
void Main_options(void)
{
	g_done = 0;
	LCD_clearScreen();
	LCD_displayString("+ : Open Door");
	LCD_moveCursor(ROW_ONE,COLUMN_ZERO);
	LCD_displayString("- : Change Pass");

	switch(KEYPAD_getPressedKey())
	{
	case '-':
		while(g_done != 1)
		{
			LCD_clearScreen();
			LCD_displayString("PLZ Enter PASS:");
			LCD_moveCursor(ROW_ONE,COLUMN_ZERO);
			Password_fillIn(g_password);
			Command_send(CHECK_PASSWORD);
			Password_send(g_password);
			switch (Command_recieve())
			{
			case PASSWORD_MATCH:
				Password_creatAndStore();
				g_done = 1;
				g_wrong=0;
				break;
			case PASSWORD_NOT_MATCHED:
				Password_wrongScreen();
				break;
			}
		}
		break;

	case '+':
		while(g_done != 1)
		{
			LCD_clearScreen();
			LCD_displayString("PLZ Enter PASS:");
			LCD_moveCursor(ROW_ONE,COLUMN_ZERO);
			Password_fillIn(g_password);
			Command_send(CHECK_PASSWORD);
			Password_send(g_password);
			switch (Command_recieve())
			{
			case PASSWORD_MATCH:
				Command_send(OPEN_DOOR);
				UART_sendByte(READY);
				while(UART_recieveByte() != READY){};
				Timer1_setCallBack(Door_isOpeningClosing);
				Timer1_init(&TIMER_configuration);
				while(g_tick != TIMER_TOTAL_TICKS);
				Timer1_deInit();
				g_tick = 0;
				g_done = 1;
				g_wrong=0;
				break;
			case PASSWORD_NOT_MATCHED:
				Password_wrongScreen();
				break;
			}
		}
		break;
	}
}
/*
 * Description
 * Functions that responsible for wrong password input.
 */
void Password_wrongScreen(void)
{
	g_wrong++;
	if(g_wrong == MAX_WRONG_COUNTER)
	{
		Command_send(WRONG_PASSWORD);
		UART_sendByte(READY);
		while(UART_recieveByte() != READY);
		Timer1_setCallBack(Alert);
		Timer1_init(&TIMER_configuration);
		while(g_tick != TIMER_TICKS_1MINUTE);
		Timer1_deInit();
		g_tick = 0;
		g_done = 1;
		g_wrong=0;
	}
}
/*
 * Description
 * Functions that responsible for fill in the password.
 */
void Password_fillIn(uint8 a_arr[])
{
	uint8 counter=0;
	g_key = 0;
	while(g_key != '=')
	{
		g_key=KEYPAD_getPressedKey();
		if(g_key >= 0 && g_key <= 9)
		{
			a_arr[counter] = g_key;
			LCD_displayCharacter('*');
			counter++;
		}
		_delay_ms(250);		/*Delay for 0.25 seconds*/
	}
}
/*
 * Description
 * Functions that responsible for showing word (Alert) on the LCD.
 */
void Alert(void)
{
	g_tick++;
	if(g_tick == 1)
	{
		LCD_clearScreen();
		LCD_displayString("ALERT!!!!");
	}
}
/*
 * Description
 * Functions that responsible for showing on the LCD:
 * 1- Door UNLocking..
 * 2- Door Locking..
 */
void Door_isOpeningClosing(void)
{
	g_tick++;
	if(g_tick == 1)
	{
		LCD_clearScreen();
		LCD_displayString("Door UNLocking..");
	}
	else if(g_tick == DC_ON_TICKS)
	{
		/*Do Nothing*/
	}
	else if(g_tick == DC_HOLD_TICKS)
	{
		LCD_clearScreen();
		LCD_displayString("Door Locking..");
	}
}
