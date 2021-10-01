/*
 *  S25FL_CIAA_port.c
 *
 *  Created on: 17-09-2021
 *  Author: Martin Rios - jrios@fi.uba.ar
 * 
 */

#include "S25FL_CIAA_port.h"

/*************************************************************************************************
	 *  @brief Funcion para set/reset del chip enable
     *
	 *  @param		estado	Determina la accion a ser tomada con el pin CS.
	 *  @return     None.
***************************************************************************************************/
void chipSelect_CIAA_port(csState_t estado)  {

	switch(estado)  {

	case CS_ENABLE:
        gpioWrite ( MEMORY_CS, LOW );
		//Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT,3,0);
		break;

	case CS_DISABLE:
        gpioWrite ( MEMORY_CS, HIGH );
		//Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT,3,0);
		break;

	default:
		;
	}
}

/*************************************************************************************************
	 *  @brief Funcion para leer datos mediante SPI
     *
	 *  @param[out]	buffer
	 * 				Buffer donde se almacenaran los datos leidos.
	 *	@param[in]	buffersize
	 *				La cantidad de datos a leer.
	 *  @return     True si pudo leer correctamente del puerto, False en caso contrario.
***************************************************************************************************/
bool_t spiRead_CIAA_port(uint8_t* buffer, uint32_t bufferSize)
{
    if(spiRead(SPI0, buffer, bufferSize)) return true;
    else return false;
}

/**************************************************************************/
/*! 
    @brief      Lee un registro de 1 byte del dispositivo SPI.

    @param[in]  reg
                La direccion del registro a leer.             
    @return  	El contenido del registro
*/
/**************************************************************************/
uint8_t spiReadRegister_CIAA_port(uint8_t reg)
{
	Chip_SSP_DATA_SETUP_T xferConfig;
	uint8_t rxBuff[1];
	uint8_t* txBuff = &reg;
	bool res;

	xferConfig.tx_data = txBuff;
	xferConfig.tx_cnt  = 0;
	xferConfig.rx_data = rxBuff;
	xferConfig.rx_cnt  = 0;
	xferConfig.length  = 1;

	Chip_SSP_RWFrames_Blocking(LPC_SSP1,  &xferConfig);
	//Chip_SSP_Int_RWFrames8Bits( LPC_SSP1, &xferConfig );

	return rxBuff[0];
}

/**************************************************************************/
/*! 
    @brief      Escribe un array de datos al puerto SPI.

    @param[in]  buffer
				El array con los datos a escribir.
	@param[in]	bufferSize
				La cantidad de datos a escribir.            
*/
/**************************************************************************/
void spiWrite_CIAA_port(uint8_t* buffer, uint32_t bufferSize)
{
	Chip_SSP_DATA_SETUP_T xferConfig;

	xferConfig.tx_data = buffer;
	xferConfig.tx_cnt  = 0;
	xferConfig.rx_data = NULL;
	xferConfig.rx_cnt  = 0;
	xferConfig.length  = bufferSize;

	Chip_SSP_RWFrames_Blocking(LPC_SSP1,  &xferConfig);
	//Chip_SSP_Int_RWFrames8Bits( LPC_SSP1, &xferConfig );
	while(Chip_SSP_GetStatus(LPC_SSP1, SSP_STAT_BSY));
}

/**************************************************************************/
/*! 
    @brief      Escribe un byte al puerto SPI.

    @param[in]  data
				El dato a escribir.          
*/
/**************************************************************************/
void spiWriteByte_CIAA_port(uint8_t data)
{
	Chip_SSP_DATA_SETUP_T xferConfig;

	xferConfig.tx_data = &data;
	xferConfig.tx_cnt  = 0;
	xferConfig.rx_data = NULL;
	xferConfig.rx_cnt  = 0;
	xferConfig.length  = 1;

	//Chip_SSP_RWFrames_Blocking(LPC_SSP1,  &xferConfig);
	Chip_SSP_Int_RWFrames8Bits( LPC_SSP1, &xferConfig );
	while(Chip_SSP_GetStatus(LPC_SSP1, SSP_STAT_BSY));
}

/**************************************************************************/
/*! 
    @brief      Funcion para realizar un delay bloqueante.

    @param[in]  millisecs
				La cantidad de milisegundos del delay.          
*/
/**************************************************************************/
void delay_CIAA_port(uint32_t millisecs)
{
	delay((tick_t)millisecs);
}
