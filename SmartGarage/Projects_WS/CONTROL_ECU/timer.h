 /******************************************************************************
 *
 * Module: TIMER1
 *
 * File Name: timer.h
 *
 * Description: Header file for the TIMER1 driver
 *
 * Author: Shehab Kishta
 *
 *******************************************************************************/
#ifndef TIMER_H_
#define TIMER_H_

#include "std_types.h"
#include "common_macros.h"
#include "avr/io.h"
#include "avr/interrupt.h"
/*******************************************************************************
 *                                Definitions                                  *
 *******************************************************************************/

/*TIMER1 REGISTERS*/
#define TIMER1_CONTROL_REGIRSTER_A             		TCCR1A
#define TIMER1_CONTROL_REGIRSTER_B             		TCCR1B
#define TIMER1_INITIAL_VALUE_REGISTER          		TCNT1
#define TIMER1_OUTPUT_COMPARE_REGISTER_A       		OCR1A
#define TIMER1_INTERRUPT_MASK_REGISTER         		TIMSK
#define TIMER1_INTERRUPT_FLAG_REGISTER         		TIFR

/*TIMER1_CONTROL_REGIRSTER*/
#define TIMER1_FORCE_OUTPUT_COMPARE_BIT_A       	FOC1A
#define TIMER1_FORCE_OUTPUT_COMPARE_BIT_B       	FOC1B
#define TIMER1_WAVE_FORM_GENERATION_BIT10			WGM10
#define TIMER1_WAVE_FORM_GENERATION_BIT11			WGM11
#define TIMER1_WAVE_FORM_GENERATION_BIT12      		WGM12
#define TIMER1_WAVE_FORM_GENERATION_BIT13      		WGM13

/*TIMER1_INTERRUPT_FLAG_REGISTER*/
#define TIMER1_OUTPUT_COMPARE_MATCH_INTERRUPT  		OCIE1A
#define TIMER1_OUTPUT_NORMAL_INTERRUPT       		TOIE1

/*******************************************************************************
 *                         Configurations                                      *
 *******************************************************************************/
extern volatile uint8 g_timeValue;

typedef enum
{
	NO_CLOCK,F_CPU_CLOCK,F_CPU_8,F_CPU_64,F_CPU_256,F_CPU_1024
}Timer1_Prescaler;

typedef enum
{
	Normal,Compare
}Timer1_Mode;

typedef struct {
	uint16 initial_value;
	uint16 compare_value; // it will be used in compare mode only.
	Timer1_Prescaler prescaler;
	Timer1_Mode mode;
} Timer1_ConfigType;

/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/

/*
 * Description:  Function to Initialize Timer Driver
 */
void Timer1_init(const Timer1_ConfigType * Config_Ptr);

/*
 * Description: Function to DeInit the timer to start again from beginning
 */
void Timer1_deInit(void);

/*
 * Description: Function to set the Call Back function address.
 */
void Timer1_setCallBack(void(*a_ptr)(void));

#endif /* TIMER_H_ */
