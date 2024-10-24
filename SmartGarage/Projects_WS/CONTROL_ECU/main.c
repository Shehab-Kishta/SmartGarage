/******************************************************************************
 *
 * File Name: main.c
 *
 * Description: Source file for the CONTROL_ECU
 *
 * Author: Shehab Kishta
 *
 *******************************************************************************/
#include"external_eeprom.h"
#include"avr\io.h"
#include<avr/interrupt.h>
#include<util/delay.h>
#include"std_types.h"
#include"uart.h"
#include"common_macros.h"
#include"dc_motor.h"
#include"timer.h"
#include"buzzer.h"
#include"twi.h"

/*******************************************************************************
 *                                Definitions                                  *
 *******************************************************************************/
#define READY                                              0xFF
#define DONE                                               0xFE
#define PASSWORD_SEND                                      0xFD
#define PASSWORD_MATCH                                     0xFC
#define PASSWORD_NOT_MATCHED                               0xFB
#define PASSWORD_CONFIRMATION_SEND                         0xFA
#define CHANGE_PASSWORD                                    0xF9
#define OPEN_DOOR                                          0xF8
#define CHECK_PASSWORD                                     0xF7
#define WRONG_PASSWORD                                     0xF6
#define CHECK_IF_SAVED                                     0xF5
#define YES_SAVED                                          0xF4
#define NO_SAVED_PASSWORD                                  0xF3
#define DC_ON_TICKS                                        16
#define DC_HOLD_TICKS                                      19
#define TIMER_TICKS_STOP								   3
#define TIMER_TICKS_1MINUTE                                60
#define TIMER_TOTAL_TICKS								   33
#define MAX_WRONG_COUNTER                                  3
#define PASSWORD_SIZE                                	   5
#define DEFUALT_VALUE_OF_EEPROM                            1
typedef enum{
	False, True
}bool;
/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/
void Password_recieve(uint8 a_arr[]);
bool Match_or_NoMatch(uint8 a_arr1[],uint8 a_arr2[]);
void Password_storeInMemory(void);
uint8 Command_recieve(void);
void Command_send(uint8 command);
void Get_savedPassword(uint8 a_arr[]);
void g_tickCounterMotor(void);
void g_tickCounterAlarm(void);

/************************************************************************************************
 *                                GLOBAL VARIABLES                                              *
 ************************************************************************************************/
uint8 g_password[5];
uint8 g_passmatch[5];
uint8 savedpass[5];
uint8 command;
uint8 g_tick=0;
uint8 g_wrong=0;
UART_ConfigType UART_configuration = {EIGHT, DISABLED, ONE, 9600};
Timer1_ConfigType TIMER_configuration= {0, 7812,F_CPU_1024,Compare};
TWI_ConfigType TWI_Configuration = {0b00000010, Scale_ONE};

int main(void)
{
	TWI_init(&TWI_Configuration);
	DcMotor_Init();
	Buzzer_init();
	UART_init(&UART_configuration);
	SREG |= (1<<7);

	while(1){
		switch(Command_recieve())
		{
		case PASSWORD_SEND:
			Password_recieve(g_password);
			break;
		case PASSWORD_CONFIRMATION_SEND:
			Password_recieve(g_passmatch);
			if(Match_or_NoMatch(g_password,g_passmatch)){
				Password_storeInMemory();
				Command_send(PASSWORD_MATCH);
			}
			else
			{
				Command_send(PASSWORD_NOT_MATCHED);
			}
			break;
		case CHECK_PASSWORD:
			Password_recieve(g_password);
			Get_savedPassword(savedpass);
			if(Match_or_NoMatch(g_password,savedpass))
			{
				Command_send(PASSWORD_MATCH);
			}
			else
			{
				Command_send(PASSWORD_NOT_MATCHED);
			}
			break;
		case OPEN_DOOR:
			while(UART_recieveByte() != READY){};
			UART_sendByte(READY);
			Timer1_setCallBack(g_tickCounterMotor);
			Timer1_init(&TIMER_configuration);
			while(g_tick != TIMER_TOTAL_TICKS);
			Timer1_deInit();
			DcMotor_Rotate(DC_MOTOR_STOP, 0);
			g_tick = 0;
			break;
		case WRONG_PASSWORD:
			while(UART_recieveByte() != READY){};
			UART_sendByte(READY);
			Timer1_setCallBack(g_tickCounterAlarm);
			Timer1_init(&TIMER_configuration);
			while(g_tick != TIMER_TICKS_1MINUTE);
			Timer1_deInit();
			Buzzer_off();
			g_tick = 0;
			break;
		case CHECK_IF_SAVED:
			Get_savedPassword(savedpass);
			uint8 counter=0;
			for(uint8 i = 0 ; i < PASSWORD_SIZE; i++)
			{
				if(savedpass[i] == DEFUALT_VALUE_OF_EEPROM)
				{
					counter++;
				}

			}
			if (counter==PASSWORD_SIZE)
			{
				Command_send(NO_SAVED_PASSWORD);
			}
			else
			{
				Command_send(YES_SAVED);
			}
			break;
		}
	}
}
/*
 * Description
 * Functions that responsible for Receiving the input password.
 */
void Password_recieve(uint8 a_arr[])
{
	while(UART_recieveByte() != READY){};
	UART_sendByte(READY);
	for(uint8 i=0 ; i<PASSWORD_SIZE ; i++){
		a_arr[i]=UART_recieveByte();
	}
}
/*
 * Description
 * Functions that responsible for Checking the two input passwords.
 */
bool Match_or_NoMatch(uint8 a_arr1[],uint8 a_arr2[])
{
	uint8 counter=0;
	for(uint8 i=0 ; i<PASSWORD_SIZE ; i++)
	{
		if(a_arr1[i]==a_arr2[i]){
			counter++;
		}
	}
	if(counter==PASSWORD_SIZE){
		return TRUE;
	}
	else{
		return FALSE;
	}
}
/*
 * Description
 * Functions that responsible for Storing the input password.
 */
void Password_storeInMemory(void)
{
	for(uint8 i=0;i<PASSWORD_SIZE;i++){
		EEPROM_writeByte(0x0311+i,g_password[i]);
		_delay_ms(10);
	}
}
/*
 * Description
 * Functions that responsible for Receiving the Command.
 */
uint8 Command_recieve(void)
{
	while(UART_recieveByte() != READY){};
	UART_sendByte(READY);
	command=UART_recieveByte();
	UART_sendByte(DONE);
	return command;
}
/*
 * Description
 * Functions that responsible for Sending the Command.
 */
void Command_send(uint8 command)
{
	UART_sendByte(READY);
	while(UART_recieveByte() != READY){};
	UART_sendByte(command);
}
/*
 * Description
 * Functions that responsible for Getting the Saved password.
 */
void Get_savedPassword(uint8 a_arr[])
{
	for(uint8 i=0 ; i<PASSWORD_SIZE ; i++){
		EEPROM_readByte(0x0311+i, &a_arr[i]);
		_delay_ms(10);
	}
}
/*
 * Description
 * Functions that responsible for Rotating the Motor.
 */
void g_tickCounterMotor(void)
{
	g_tick++;
	if(g_tick == 1)
	{
		DcMotor_Rotate(DC_MOTOR_CW, 100);
	}else if(g_tick == DC_ON_TICKS)
	{
		DcMotor_Rotate(DC_MOTOR_STOP, 0);
	}else if(g_tick == DC_HOLD_TICKS)
	{
		DcMotor_Rotate(DC_MOTOR_ACW, 100);
	}
}
/*
 * Description
 * Functions that responsible for Incrementing the timer seconds.
 */
void g_tickCounterAlarm(void)
{
	g_tick++;
	if(g_tick == 1)
	{
		Buzzer_on();
	}
}
