/*!@file : K32L2B31A_labs_diplomado_iot.c
 * @author  Ernesto Andres Rincon Cruz
 * @version 0.00
 * @date    05/10/2022
 * @brief   Funcion principal main
 * @details
 *			v0.000	Proyecto base creado usando MCUXpresso
 *
 *
 */
/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "K32L2B31A.h"

#include "fsl_debug_console.h"
#include "fsl_adc16.h"
#include "lpuart0.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Private Prototypes
 ******************************************************************************/

/*******************************************************************************
 * External vars
 ******************************************************************************/


/*******************************************************************************
 * Local vars
 ******************************************************************************/
/* Force the counter to be placed into memory. */
volatile static uint8_t i = 0 ;
uint16_t dato_adc;
/*******************************************************************************
 * Private Source Code
 ******************************************************************************/
int main(void) {
	/* Init board hardware. */
	BOARD_InitBootPins();
	BOARD_InitBootClocks();
	BOARD_InitBootPeripherals();
#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
	/* Init FSL debug console. */
	BOARD_InitDebugConsole();
#endif

	/*Configurar canal ADC por donde se desea lectura
	 * dar señal de start al ADC */
	ADC16_SetChannelConfig(ADC0_PERIPHERAL, ADC0_CH0_CONTROL_GROUP,	&ADC0_channelsConfig[0]);

	/*Esperar que el ADC finalice el ADC*/
	while (0U == (kADC16_ChannelConversionDoneFlag	& ADC16_GetChannelStatusFlags(ADC0_PERIPHERAL,	ADC0_CH0_CONTROL_GROUP))) {
	}

	dato_adc=ADC16_GetChannelConversionValue(ADC0_PERIPHERAL,ADC0_CH0_CONTROL_GROUP);
	/*Captura dato ADC e imprime por consola*/
	PRINTF("ADC Value: %d\r\n",	dato_adc);

	/* Enter an infinite loop, just incrementing a counter. */
	while (1) {
		i++;
		if(leer_bandera_nuevo_dato()!=0){
			escribir_bandera_nuevo_dato(0);
			PRINTF("ADC Value: %c\r\n",	leer_dato());
		}
		/* 'Dummy' NOP to allow source level single stepping of
		 tight while() loop */
		__asm volatile ("nop");
	}
	return 0;
}



