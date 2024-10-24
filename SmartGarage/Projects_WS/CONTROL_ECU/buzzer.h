 /******************************************************************************
 *
 * Module: BUZZER
 *
 * File Name: buzzer.h
 *
 * Description: header file for the BUZZER driver
 *
 * Author: Shehab Kishta
 *
 *******************************************************************************/
#ifndef BUZZER_H_
#define BUZZER_H_

#include "std_types.h"
#include "common_macros.h"
#include <avr/io.h>
#include "util/delay.h"
#include "gpio.h"

/*******************************************************************************
 *                                Definitions                                  *
 *******************************************************************************/

#define BUZZER_PORT                 PORTA_ID
#define BUZZER_PIN                  PIN7_ID


/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/
/*
 * Description
 * Function to Initialize BUZZER Driver.
 */
void Buzzer_init(void);
/*
 * Description
 * Function to Activate the BUZZER.
 */
void Buzzer_on(void);
/*
 * Description
 * Function to DeActivate the BUZZER.
 */
void Buzzer_off(void);


#endif /* BUZZER_H_ */
