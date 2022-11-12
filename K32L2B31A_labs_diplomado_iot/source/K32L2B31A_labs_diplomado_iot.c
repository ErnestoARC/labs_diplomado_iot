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
enum{
	CMD_AT_ATI_Display_Product_Identification_Information=0,
	CMD_AT_AT_GMI_Request_Manufacturer_Identification,
	CMD_AT_AT_GMM_Request_TA_Model_Identification,
	CMD_AT_AT_GMR_Request_TA_Revision_Identification_of_Software_Release,
	CMD_AT_AT_CGMI_Request_Manufacturer_Identification,
};

typedef struct _iot_nodo_data{
	uint16_t data_sensor_luz_adc;
	uint16_t data_sensor_luz_lux;
	//------------------------------------
	uint8_t data_sensor_luz_voltaje;
	uint8_t data_sensor_temperatura;
	uint16_t data_sensor_humedad;
	//------------------------------------
	uint16_t data_sensor_presion_atmosferica;
	uint16_t reservado;
	//------------------------------------
} iot_nodo_data_t;
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

const char msg1[100]={'h','o','l','a','1',0x00};
const char msg2[]="hola2";
#define MSG3 "hola3"

const char* cmd_at[5]={
		"ATI\r\n",
		"AT+GMI\r\n",
		"AT+GMM\r\n",
		"AT+GMR\r\n",
		"AT+CGMI\r\n",
};

uint32_t msg_size;
iot_nodo_data_t datos_locales;

/*******************************************************************************
 * Private Source Code
 ******************************************************************************/
void ec25_print_data_raw(uint8_t *data_ptr, uint32_t data_size) {
	for (uint32_t i = 0; i < data_size; i++) {
		PRINTF("%c", *(data_ptr + i));
	}
}

void ec25_print_data_ascii_hex(uint8_t *data_ptr, uint32_t data_size) {
	for (uint32_t i = 0; i < data_size; i++) {
		PRINTF("0x%02x ", *(data_ptr + i));
	}
}

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

	datos_locales.data_sensor_luz_adc=ADC16_GetChannelConversionValue(ADC0_PERIPHERAL,ADC0_CH0_CONTROL_GROUP);
	datos_locales.data_sensor_luz_voltaje=2;
	datos_locales.data_sensor_luz_lux=3;
	datos_locales.data_sensor_temperatura=4;
	datos_locales.data_sensor_humedad=5;
	datos_locales.data_sensor_presion_atmosferica=6;

//	/*Captura dato ADC e imprime por consola*/
//	PRINTF("data_sensor_luz_adc: %d\r\n",	datos_locales.data_sensor_luz_adc);
//	/*Imprimir mensajes*/
//	PRINTF("%s\r\n",msg1);
//	msg_size=sizeof(msg1);
//	PRINTF("sizeof:%d\r\n",msg_size);
//	msg_size=strlen(msg1);
//	PRINTF("strlen:%d\r\n",msg_size);
//
//	PRINTF("%s\r\n",msg2);
//	msg_size=sizeof(msg2);
//	PRINTF("sizeof:%d\r\n",msg_size);
//	msg_size=strlen(msg2);
//	PRINTF("strlen:%d\r\n",msg_size);
//
//	PRINTF("%s\r\n",MSG3);
//	msg_size=sizeof(MSG3);
//	PRINTF("sizeof:%d\r\n",msg_size);
//	msg_size=strlen(MSG3);
//	PRINTF("strlen:%d\r\n",msg_size);
//
//	PRINTF("%s",cmd_at[CMD_AT_AT_GMM_Request_TA_Model_Identification]);


	/* Enter an infinite loop, just incrementing a counter. */
	while (1) {
		i++;
		if(leer_bandera_nuevo_dato()!=0){
			escribir_bandera_nuevo_dato(0);
			datos_locales.data_sensor_luz_adc=ADC16_GetChannelConversionValue(ADC0_PERIPHERAL,ADC0_CH0_CONTROL_GROUP);
			ec25_print_data_ascii_hex((uint8_t *)(&datos_locales),sizeof(datos_locales));
		}
		/* 'Dummy' NOP to allow source level single stepping of
		 tight while() loop */
		__asm volatile ("nop");
	}
	return 0;
}



