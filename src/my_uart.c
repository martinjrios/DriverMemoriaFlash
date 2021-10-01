/*=====avl_uart.c============================================================
 * Author: Martin Rios <jrios@fi.uba.ar>
 * Date: 2021/08/10
 * Version: 1.0
 *===========================================================================*/

#include "../inc/my_uart.h"
#include "sapi.h"
#include "stdlib.h"
#include "ctype.h"

static uartMap_t menuUart;

/*!
 * @brief   Inicializa la UART
 * @param   _uart
 * 			La UART que se va a utilizar para manejar el menu
 */
void UART_Init(uartMap_t _uart)
{
	menuUart = _uart;
	uartInit ( menuUart, MENU_UART_BAUDRATE );
}

/*!
 * @brief   Lee la opcion ingresada desde la terminal
 * @return  El valor numerico de la opcion ingresada
 */
uint8_t UART_readOption()
{
	char optionAscii;
	char inputBuffer[MAX_INPUT_BUFFER];
	uint8_t optionNumber = INVALID_OPTION, len;

	len = UART_ReadLine(inputBuffer, MAX_INPUT_BUFFER);

	if(len > 0)
	{
		if(!isalpha(inputBuffer[0])) optionNumber = atoi(inputBuffer);  // Se comprueba que lo ingresado sea un numero
		else optionNumber = INVALID_OPTION;
	}
	else optionNumber = INVALID_OPTION; // Si no se ingreso un valor se toma como opcion invalida
	
	return optionNumber;
}

/*!
 * @brief   Lee una linea desde la UART y la almacena en un buffer
 * @param	readBuffer
 * 			El buffer para almacenar la linea leida
 * @param	maxLength
 * 			La longitud maxima del buffer
 * @return  La cantidad de bytes leidos sin contar el caracter de final de linea
 */
uint8_t UART_ReadLine(char *readBuffer, uint8_t maxLength)
{
	uint8_t byteRead, byteCount = 0, i;
	delay_t uartTimeout;

	if(maxLength < 1) return FALSE;  // Si la longitud del buffer es 0 se sale ya que no tendra espacio para almacenar algun caracter

	for(i = 0; i < maxLength; i++)  readBuffer[i] = '\0'; // limpio el str

	delayInit ( &uartTimeout, UART_TIMEOUT);

	while(byteCount < maxLength)
	{
		if(uartReadByte(menuUart, &byteRead))
		{
			if('\n' == byteRead || '\r' == byteRead || '\0' == byteRead) break; // Si se detecta alguno de los caracteres delimitadores de linea, se sale
			*readBuffer++ = (char) byteRead;
			byteCount++;
		}
		else if(delayRead(&uartTimeout)) break;  // Si no se encontro el caracter delimitador y se cumplio el tiempo maximo, se sale
	}

	if(byteCount < maxLength) readBuffer[byteCount] = '\0'; // Coloco el \0 para finalizar el string
	else readBuffer[byteCount-1] = '\0'; // Si se ingreso la cantidad maxima de caracteres sin un delimitador de linea, se Coloca el \0 para finalizar el string en la ultima posicion

	delay(1);  // Espero 1 ms para que se termine de cargar en el buffer de la UART lo que haya quedado pendiente de lectura
	uartRxFlush(menuUart); // Limpio lo que haya quedado en el buffer

	return byteCount;
}

/*!
 * @brief   Escribe una cadena en la UART con la terminacion \r\n
 * @param	line
 * 			La cadena a escribir
 */
void UART_WriteLine(const char *line)
{
	uartWriteString(menuUart, line);
	uartWriteString(menuUart, "\r\n");
}

/*!
 * @brief   Escribe una cadena en la UART
 * @param	str
 * 			La cadena a escribir
 */
void UART_Write(const char *str)
{
	uartWriteString(menuUart, str);
}

/*!
 * @brief   Detecta si hay algun byte sin leer en el buffer de la UART
 * @return  TRUE si hay algun byte disponible para leer desde la UART
 */
bool_t UART_Available()
{
	return(uartRxReady(menuUart));
}

/*!
 * @brief   Despliega en la UART las opciones de un menu
 * @param	menuOptions
 * 			El arreglo con los textos de las opciones del menu
 * @param	lastOptionIndex
 * 			La cantidad de opciones que tiene el menu
 */
void UART_ShowOptions(const char **menuOptions, uint8_t lastOptionIndex)
{
	uint8_t i;
	char strOptionIndex[MAX_STR_OPTION_INDEX];

	for(i = 0; i < lastOptionIndex; i++)
	{
		sprintf(strOptionIndex, "[%d]-", i);
		uartWriteString( menuUart, strOptionIndex);
		UART_WriteLine(menuOptions[i]);
	}
}

/*!
 * @brief   Envia un comando VT100 a la terminal
 * @param	command
 * 			El texto del codigo del comando VT100
 */
void UART_sendTerminalCommand(const char *command)
{
	uartWriteByte(menuUart, 27); // envio el caracter ESC
	uartWriteString(menuUart, command); // envio el comando
}

/*!
 * @brief   Borra la pantalla de la terminal
 */
void UART_clearTerminal()
{
	UART_sendTerminalCommand(CLEAR_SCREEN);
}

/*!
 * @brief   Posiciona el cursor de la terminal en el inicio de la pantalla
 */
void UART_cursorHome()
{
	UART_sendTerminalCommand(CURSOR_HOME);
}

/*!
 * @brief   Coloca el cursor de la terminal en una determinada posicion
 * @param	v
 * 			El numero de linea para la posicion vertical del cursor
 * @param	h
 * 			El numero de linea para la posicion horizontal del cursor
 */
void UART_setCursorPosition(uint8_t v, uint8_t h)
{
	char command[11];

	sprintf(command, "[%d;%df", v, h);
	UART_sendTerminalCommand(command);
}

/*!
 * @brief   Desplaza el cursor de la terminal en una cierta cantidad de lineas hacia abajo
 * @param	n
 * 			La cantidad de lineas a desplazarse
 */
void UART_moveCursorNDown(uint8_t n)
{
	char command[11];

	sprintf(command, "[%dB", n);
	UART_sendTerminalCommand(command);
}

/*!
 * @brief   Desplaza el cursor de la terminal en una cierta cantidad de lineas hacia arriba
 * @param	n
 * 			La cantidad de lineas a desplazarse
 */
void UART_moveCursorNUp(uint8_t n)
{
	char command[11];

	sprintf(command, "[%dA", n);
	UART_sendTerminalCommand(command);
}
