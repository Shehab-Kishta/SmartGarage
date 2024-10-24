 /******************************************************************************
 *
 * Module: TIMER1
 *
 * File Name: timer.c
 *
 * Description: Source file for the TIMER1 driver
 *
 * Author: Shehab Kishta
 *
 *******************************************************************************/
#include"timer.h"

volatile uint8 g_timeValue=0;
static void (*g_Timer1_callBackPtr)(void) = NULL_PTR;
/*******************************************************************************
 *                      Functions Definitions                                  *
 *******************************************************************************/
ISR(TIMER1_OVF_vect)
{
	if(g_Timer1_callBackPtr != NULL_PTR)
	{
		/* Call the Call Back function in the application after the edge is detected */
		(*g_Timer1_callBackPtr)();
	}
}

ISR(TIMER1_COMPA_vect)
{
	if(g_Timer1_callBackPtr != NULL_PTR)
	{
		/* Call the Call Back function in the application after the edge is detected */
		(*g_Timer1_callBackPtr)();
	}
}
/*
 * Description
 * Function to initialize the Timer driver.
 */
void Timer1_init(const Timer1_ConfigType * Config_Ptr)
{
	/*
	 * Configure initial value for Timer1 to start count from it
	 * 0XFFFF so that value doesn't exceed 16_bits
	 */
	TIMER1_INITIAL_VALUE_REGISTER = ( (Config_Ptr -> initial_value) ) & 0XFFFF;
	/*
	 * TIMER Pre-scaler value for Timer1 in TCCR1B Register
	 * 0XF8 to make sure that the least 3-bits in TCCR1B register=0
	 * ORing with the chosen timer Pre-scaler to enter it into the least 3-bits
	 */
	TIMER1_CONTROL_REGIRSTER_B =  (TIMER1_CONTROL_REGIRSTER_B & 0XF8) | (Config_Ptr -> prescaler);
	/*
	 * Compare mode:
	 *              OC1A/OC1B disconnected
	 *              Clear COM1A0/COM1A1 bits in TCCR1A register
	 *              Clear COM1B0/COM1B1 bits in TCCR1A register
	 */
	TIMER1_CONTROL_REGIRSTER_A = TIMER1_CONTROL_REGIRSTER_A & 0X0F;

	switch(Config_Ptr -> mode)
	{
	case Normal:

		/*
		 * Normal Overflow mode:
		 *                      Clear WGM10/WMG11 bit in TCCR1A register
		 *                      Clear WGM12/WGM13 bit in TCCR1B register
		 */

		TIMER1_CONTROL_REGIRSTER_A = CLEAR_BIT(TIMER1_CONTROL_REGIRSTER_A, TIMER1_WAVE_FORM_GENERATION_BIT10);
		TIMER1_CONTROL_REGIRSTER_A = CLEAR_BIT(TIMER1_CONTROL_REGIRSTER_A, TIMER1_WAVE_FORM_GENERATION_BIT11);
		TIMER1_CONTROL_REGIRSTER_B = CLEAR_BIT(TIMER1_CONTROL_REGIRSTER_B, TIMER1_WAVE_FORM_GENERATION_BIT12);
		TIMER1_CONTROL_REGIRSTER_B = CLEAR_BIT(TIMER1_CONTROL_REGIRSTER_B, TIMER1_WAVE_FORM_GENERATION_BIT13);

		/*
		 * Make FOC1A/FOC1B to be Active as it is normal mode
		 */
		TIMER1_CONTROL_REGIRSTER_A =  SET_BIT(TIMER1_CONTROL_REGIRSTER_A,TIMER1_FORCE_OUTPUT_COMPARE_BIT_A);
		TIMER1_CONTROL_REGIRSTER_A =  SET_BIT(TIMER1_CONTROL_REGIRSTER_A,TIMER1_FORCE_OUTPUT_COMPARE_BIT_B);

		/*
		 * Enable Timer1 overflow interrupt
		 */
		TIMER1_INTERRUPT_MASK_REGISTER = SET_BIT(TIMER1_INTERRUPT_MASK_REGISTER,TIMER1_OUTPUT_NORMAL_INTERRUPT);

		break;


	case Compare:
		/*
		 *  Compare mode:
		 *              Clear WGM10/WGM11 bits in TCCR1A register
		 *              Set WGM12 bit in TCCR1B register
		 *              Clear WGM13 bit in TCCR1B register
		 */
		TIMER1_CONTROL_REGIRSTER_A = CLEAR_BIT(TIMER1_CONTROL_REGIRSTER_A, TIMER1_WAVE_FORM_GENERATION_BIT10);
		TIMER1_CONTROL_REGIRSTER_A = CLEAR_BIT(TIMER1_CONTROL_REGIRSTER_A, TIMER1_WAVE_FORM_GENERATION_BIT11);
		TIMER1_CONTROL_REGIRSTER_B = SET_BIT(TIMER1_CONTROL_REGIRSTER_B, TIMER1_WAVE_FORM_GENERATION_BIT12);
		TIMER1_CONTROL_REGIRSTER_B = CLEAR_BIT(TIMER1_CONTROL_REGIRSTER_B, TIMER1_WAVE_FORM_GENERATION_BIT13);

		/*
		 *  Make FOC1A/FOC1B to be Active as it is compare mode
		 */
		TIMER1_CONTROL_REGIRSTER_A =  SET_BIT(TIMER1_CONTROL_REGIRSTER_A,TIMER1_FORCE_OUTPUT_COMPARE_BIT_A);
		TIMER1_CONTROL_REGIRSTER_A =  SET_BIT(TIMER1_CONTROL_REGIRSTER_A,TIMER1_FORCE_OUTPUT_COMPARE_BIT_B);

		/*
		 * Configure Compare match value for Timer1 to start count from it
		 * 0XFFFF so that value doesn't exceed 16_bits
		 */
		TIMER1_OUTPUT_COMPARE_REGISTER_A = ((Config_Ptr -> compare_value)) & 0XFFFF;

		/*
		 * Enable Timer1 compare match interrupt
		 */
		TIMER1_INTERRUPT_MASK_REGISTER = SET_BIT(TIMER1_INTERRUPT_MASK_REGISTER,TIMER1_OUTPUT_COMPARE_MATCH_INTERRUPT);

		break;
	}
}
/*
 * Description: Function to DeInit the timer to start again from beginning
 */
void Timer1_deInit(void)
{
	/*Clear all register in Timer1*/
	TIMER1_CONTROL_REGIRSTER_A         &= 0X00;
	TIMER1_CONTROL_REGIRSTER_B         &= 0X00;
	TIMER1_INITIAL_VALUE_REGISTER      &= 0X00;
	TIMER1_OUTPUT_COMPARE_REGISTER_A   &= 0X00;
	TIMER1_INTERRUPT_MASK_REGISTER     &= 0X00;
}
/*
 * Description: Function to set the Call Back function address.
 */
void Timer1_setCallBack(void(*a_ptr)(void))
{
	g_Timer1_callBackPtr = a_ptr;
}
