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
	FSM_ESTADO_INICIO=0,
	FSM_ESTADO_ESPERA_NUEVO_DATO_LPUART0,
	FSM_ESTADO_ANALIZA_NUEVO_DATO_LPUART0,
	FSM_ESTADO_ENVIAR_COMANDO_ATI,
	FSM_ESTADO_START_ADC,
	FSM_ESTADO_ESPERA_TIEMPO_RESULTADO,
	FSM_ESTADO_CAPTURA_RESULTADO_ADC,
	FSM_ESTADO_CAPTURA_IMPRIME_RESULTADO_ADC,
};



enum{
	CMD_AT_ATI_Display_Product_Identification_Information=0,
	CMD_AT_AT_GMI_Request_Manufacturer_Identification,
	CMD_AT_AT_GMM_Request_TA_Model_Identification,
	CMD_AT_AT_GMR_Request_TA_Revision_Identification_of_Software_Release,
	CMD_AT_AT_CGMI_Request_Manufacturer_Identification,
};

typedef struct _iot_nodo_data{
	uint16_t data_sensor_luz_adc;	//2 bytes
	uint16_t data_sensor_luz_lux;	//2 bytes
	//------------------------------------
	uint8_t data_sensor_luz_voltaje;// 1 byte
	uint8_t data_sensor_temperatura;// 1 byte
	uint16_t data_sensor_humedad;	//2 bytes
	//------------------------------------
	uint16_t data_sensor_presion_atmosferica;//2 bytes
	//------------------------------------
} iot_nodo_data_t;

typedef struct _sigfox_frame{
	uint32_t Preamble ;					//(4 bytes)
	uint16_t frame_synchronization;		// (2 bytes)
	uint32_t end_point_id;				// (4 bytes)
	uint8_t payload[12];				//Payload (0 to 12 bytes)
	uint16_t authentication;			// (var. length)
	uint16_t  fcs;						// (2 bytes, used as CRC)
} sigfox_frame_t;
/*******************************************************************************
 * Private Prototypes
 ******************************************************************************/

/*******************************************************************************
 * External vars
 ******************************************************************************/


/*******************************************************************************
 * Local vars
 ******************************************************************************/
uint8_t fst_estado_actual=FSM_ESTADO_INICIO;

/* Force the counter to be placed into memory. */
volatile static uint8_t i = 0 ;

const char msg1[100]={'h','o','l','a','1',0x00};
const char msg2[]="msg sigfox prueba";	//17 bytes
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
sigfox_frame_t sigfox_frame;

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

	/* Enter an infinite loop, just incrementing a counter. */
	while (1) {
		switch (fst_estado_actual) {
		case FSM_ESTADO_INICIO:
			/*Escribir condiciones iniciales para le ejecución de toda la FSM*/
			datos_locales.data_sensor_luz_adc = 1;
			datos_locales.data_sensor_luz_voltaje = 2;
			datos_locales.data_sensor_luz_lux = 3;
			datos_locales.data_sensor_temperatura = 4;
			datos_locales.data_sensor_humedad = 5;
			datos_locales.data_sensor_presion_atmosferica = 6;

			fst_estado_actual=FSM_ESTADO_ESPERA_NUEVO_DATO_LPUART0;
			break;

		case FSM_ESTADO_ESPERA_NUEVO_DATO_LPUART0:
			if(lpuart0_leer_bandera_nuevo_dato()!=0){
				lpuart0_escribir_bandera_nuevo_dato(0);
				fst_estado_actual=FSM_ESTADO_ANALIZA_NUEVO_DATO_LPUART0;
			}
			break;

		case FSM_ESTADO_ANALIZA_NUEVO_DATO_LPUART0:
			switch(lpuart0_leer_dato()){
			case 'l':
				fst_estado_actual=FSM_ESTADO_START_ADC;
				break;
			case 'm':
				fst_estado_actual=FSM_ESTADO_ENVIAR_COMANDO_ATI;
				break;
			case 'b':
				lpuart0_borrar_buffer();
				break;
			default://dato ilegal
				fst_estado_actual=FSM_ESTADO_INICIO;
				break;
			}
			break;

		case FSM_ESTADO_ENVIAR_COMANDO_ATI:
			PRINTF("%s",cmd_at[CMD_AT_ATI_Display_Product_Identification_Information]);
			fst_estado_actual=FSM_ESTADO_INICIO;
			break;

		case FSM_ESTADO_START_ADC:
			/*Genera señal de START para tomar dato ADC*/
			ADC16_SetChannelConfig(ADC0_PERIPHERAL, ADC0_CH0_CONTROL_GROUP, &ADC0_channelsConfig[0]);
			fst_estado_actual=FSM_ESTADO_ESPERA_TIEMPO_RESULTADO;
			break;

		case FSM_ESTADO_ESPERA_TIEMPO_RESULTADO:
			if (0U== (kADC16_ChannelConversionDoneFlag& ADC16_GetChannelStatusFlags(ADC0_PERIPHERAL,ADC0_CH0_CONTROL_GROUP))) {
				//No ha terminado de realizar la operacion del ADC
			} else {
				fst_estado_actual=FSM_ESTADO_CAPTURA_RESULTADO_ADC;
			}
			break;

		case FSM_ESTADO_CAPTURA_RESULTADO_ADC:
			datos_locales.data_sensor_luz_adc = ADC16_GetChannelConversionValue(ADC0_PERIPHERAL, ADC0_CH0_CONTROL_GROUP);
			fst_estado_actual=FSM_ESTADO_CAPTURA_IMPRIME_RESULTADO_ADC;
			break;

		case FSM_ESTADO_CAPTURA_IMPRIME_RESULTADO_ADC:
			PRINTF("data_sensor_luz_adc: %d\r\n",datos_locales.data_sensor_luz_adc);
			fst_estado_actual=FSM_ESTADO_INICIO;
			break;

		default:	//estado ilegal
			fst_estado_actual=FSM_ESTADO_INICIO;
			break;
		}
	}
	return 0;
}

//i++;
//if(leer_bandera_nuevo_dato()!=0){
//	escribir_bandera_nuevo_dato(0);
//	ADC16_SetChannelConfig(ADC0_PERIPHERAL, ADC0_CH0_CONTROL_GROUP,	&ADC0_channelsConfig[0]);
//	/*Esperar que el ADC finalice el ADC*/

//	datos_locales.data_sensor_luz_adc=ADC16_GetChannelConversionValue(ADC0_PERIPHERAL,ADC0_CH0_CONTROL_GROUP);
//
//	/*simulacion de tx data por sigfox*/
//	sigfox_frame.Preamble=0x2342;
//	sigfox_frame.frame_synchronization=0x4567;
//	sigfox_frame.end_point_id=0x45645767;
//	sigfox_frame.authentication=0x4564;
//	memcpy(&sigfox_frame.payload[0],(uint8_t *)(&datos_locales),sizeof(datos_locales));
//	sigfox_frame.fcs=0x1234;//CRC16
//
//	/*enviar frame a sigfox*/
//	ec25_print_data_raw((uint8_t *)(&sigfox_frame),sizeof(sigfox_frame));
//}
///* 'Dummy' NOP to allow source level single stepping of
// tight while() loop */
//__asm volatile ("nop");


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
