/*
 *  main.c
 *
 *  Created on: 17-09-2021
 *  Author: Martin Rios - jrios@fi.uba.ar
 * 
 */

#include "main.h"
#include "S25FL_CIAA_port.h"
#include "fsS25FL.h"
#include "ff.h"
#include <string.h>
#include "my_uart.h"

void initHW ();
FRESULT scan_files (char* path);
static void showMainMenu();
static void showMenu(const char *menuText, const char *menuFooter, const char **options, uint8_t nrOptions);

static FATFS fatFs;
static FIL fp;             // <-- File object needed for each open file

int main (void)  
{
    s25fl_t s25flDriverStruct;
    uint8_t i;
    UINT wbytes, br;
    TCHAR fpath[32];
    char inputText[MAX_INPUT_TEXT];

    static stateMenu_t stateMenu = START;
	static stateSubmenu_t stateSubmenu = WAITING_INPUT;
    uint8_t len, menuOption = 0;
    char outputLine[MAX_TEXT];

	boardInit(); // Inicializar y configurar la plataforma	
	initHW(); // Se inicializa el hardware especifico para este proyecto

    // Se inicializa el driver de la memoria Flash
    s25flDriverStruct.chip_select_ctrl = chipSelect_CIAA_port;
    s25flDriverStruct.spi_write_fnc = spiWrite_CIAA_port;
    s25flDriverStruct.spi_writeByte_fnc = spiWriteByte_CIAA_port;
    s25flDriverStruct.spi_read_fnc = spiRead_CIAA_port;
    s25flDriverStruct.spi_read_register = spiReadRegister_CIAA_port;
    s25flDriverStruct.delay_fnc = delay_CIAA_port;
    s25flDriverStruct.memory_size = S64MB;

    UART_clearTerminal();
    UART_cursorHome();

    if(S25FL_InitDriver(s25flDriverStruct))
        UART_WriteLine("Driver inicializado."); 
    else
    {
        UART_WriteLine("Error al inicializar el driver.");
        while(1);
    }

    while(1)
    {
        switch(stateMenu)
        {
            case START: // Estado inicial
                if(S25FL_begin(&fatFs))
                {
                    UART_WriteLine("Sistema de archivos inicializado.");
                    delay(2000);
                }
                else
                {
                    UART_WriteLine("Error al iniciar el Sistema de archivos FAT.");
                    while(1);
                }

                showMainMenu();
                stateMenu = MAIN_MENU;
                break;  

            case MAIN_MENU: // Menu principal
                if(UART_Available())
                {
                    menuOption = UART_readOption();
                    sprintf(outputLine, "%d\r\n", menuOption);

                    switch((optionMainMenu_t)menuOption)
                    {
                        case OPTION_FORMAT:
                        	showMenu(formatOptionText, mainMenuFooter, ConfirmOptions, sizeof(ConfirmOptions)/sizeof(*ConfirmOptions));
                            UART_setCursorPosition(OPTIONS_START_Y_POS,OPTIONS_START_X_POS);
                            UART_WriteLine(formatWaitText1);
                            UART_WriteLine(formatWaitText2);
                            stateMenu = FORMAT;
                            break;

                        case OPTION_WRITE_FILE:
                            showMenu(writeFileOptionText, NULL, NULL, 4);
                            UART_setCursorPosition(OPTIONS_START_Y_POS,OPTIONS_START_X_POS);
                            stateMenu = WRITE_FILE;
                            break;

                        case OPTION_READ_FILE:
                            showMenu(readFileOptionText, NULL, NULL, 1);
                            UART_setCursorPosition(OPTIONS_START_Y_POS,OPTIONS_START_X_POS);
                                                        
                            sprintf(fpath, MOUNT_POINT FILE_PATH);
                            UART_Write("Leyendo contenido del archivo: ");
                            UART_Write(fpath);
                            UART_WriteLine("");
                            if( f_open( &fp, fpath, FA_READ ) == FR_OK )
                            {
                                UART_setCursorPosition(OPTIONS_START_Y_POS+3,OPTIONS_START_X_POS);
                                UART_WriteLine("********** Contenido del archivo: **********");
                                while (!f_eof( &fp ) ) 
                                {
                                    int rbytes;                                    
                                    f_read ( &fp , inputText, MAX_INPUT_TEXT, &rbytes);
                                    UART_Write(inputText);
                                    gpioWrite( LEDG, ON );
                                }
                                UART_WriteLine("********************************************");
                            } 
                            else
                            {
                                UART_WriteLine("Error: No se pudo leer el archivo."); 
                                gpioWrite( LEDR, ON );
                            }
                            f_close(&fp);
                            fflush(stdout);    
                            UART_WriteLine("Ingrese un numero y presione ENTER para volver al menu principal...");                           
                            stateMenu = READ_FILE;                            
                            break;
                        
                        case OPTION_DELETE_FILE:
                            showMenu(deleteFileOptionText, mainMenuFooter, ConfirmOptions, sizeof(ConfirmOptions)/sizeof(*ConfirmOptions));
                            //UART_setCursorPosition(OPTIONS_START_Y_POS,OPTIONS_START_X_POS);
                            stateMenu = DELETE_FILE;                        
                            break;
                        
                        case OPTION_SCAN_FILES:
                            showMenu(scanFilesOptionText, NULL, NULL, 4);
                            UART_setCursorPosition(OPTIONS_START_Y_POS,OPTIONS_START_X_POS);
                            UART_WriteLine("Escaneando la memoria en busqueda de archivos...\r\n");
                            FRESULT res = scan_files("/");
                            if(res != FR_OK) UART_WriteLine("Error: No se pudo escanear la memoria.");
                            UART_WriteLine("Ingrese un numero y presione ENTER para volver al menu principal...");
                            stateMenu = SCAN_FILES;                        
                            break;  

                        default:
                            UART_sendTerminalCommand(CLEAR_LINE);
                            UART_WriteLine(invalidOption);
                            break;                                                                                                           
                    }
                }
                break;

            case FORMAT:
                gpioWrite(LEDR, HIGH);
                if(S25FL_format(&fatFs) != 0)
                {
                    while(1)
                    {   
                        delay(250);
                        gpioWrite(LEDR, ON);
                        delay(250);
                        gpioWrite(LEDR, OFF);
                    }
                }
                gpioWrite(LEDR, LOW); 
                showMainMenu();      
                stateMenu = MAIN_MENU;    
                break;

            case WRITE_FILE:                
                switch(stateSubmenu)
                {
                    case WAITING_INPUT: // Esperando que se ingrese el texto
                        if(UART_Available())
                        {
                            len = UART_ReadLine(inputText, MAX_INPUT_TEXT);
                            if(len < MAX_INPUT_TEXT - 1)  // Se muestra lo ingresado
                            {
                                UART_setCursorPosition(OPTIONS_START_Y_POS,OPTIONS_START_X_POS);
                                UART_Write(TextInput);
                                UART_WriteLine(inputText);  // Se imprime lo que se ingreso por la UART                                

                                // Se guarda lo ingresado en el archivo
                                sprintf(fpath, MOUNT_POINT FILE_PATH);
                                if( f_open( &fp, fpath, FA_WRITE | FA_OPEN_APPEND ) == FR_OK )
                                {
                                    inputText[len] = '\r';
                                    inputText[len+1] = '\n';
                                    f_write( &fp, inputText, len+2, &wbytes );
                                    UART_WriteLine("");
                                    if( wbytes == len+2 ) UART_WriteLine("Se escribieron los datos con exito!");
                                    else UART_WriteLine("Error: No se pudieron escribir todos los datos.");
                                    f_close(&fp);            
                                }
                                UART_WriteLine("");
                                UART_Write(subMenuFooter);
                                stateSubmenu = WAITING_OPTION;
                            }
                            else // Se muestra que hubo un error
                            {
                                UART_sendTerminalCommand(CLEAR_LINE);
                                UART_sendTerminalCommand(MOVE_NEXT_LINE);
                                UART_sendTerminalCommand(CLEAR_LINE);
                                UART_WriteLine(errorText);
                                UART_moveCursorNUp(2);
                            }                            
                        }
                        break;

                    case WAITING_OPTION:  // Esperando que se ingrese la opcion
                        if(UART_Available())
                        {
                            menuOption = UART_readOption();
                            if(menuOption != INVALID_OPTION) // Si es una opcion valida se procede a ejecutar la opcion
                            {
                                switch((optionConfirm_t)menuOption)
                                {
                                case OPTION_YES:
                                    UART_clearTerminal();
                                    showMainMenu();
                                    stateSubmenu = WAITING_INPUT;
                                    stateMenu = MAIN_MENU;
                                    break;
                                case OPTION_NO:
                                    UART_sendTerminalCommand(CLEAR_LINE);
                                    UART_moveCursorNUp(1);
                                    UART_sendTerminalCommand(CLEAR_LINE);
                                    stateSubmenu = WAITING_INPUT;
                                    break;
                                }
                            }
                        }
                        break;
                }
                break;

            case READ_FILE:                   
                if(UART_Available())
                {
                    menuOption = UART_readOption();
                    if(menuOption != INVALID_OPTION)
                    { 
                        gpioWrite( LEDR, OFF );
                        gpioWrite( LEDG, OFF );
                        showMainMenu();
                        stateMenu = MAIN_MENU;
                    }
                }     
                break;

            case DELETE_FILE:
                if(UART_Available())
                {
                    menuOption = UART_readOption();
                    if(menuOption != INVALID_OPTION)
                    {
                        switch((optionConfirm_t)menuOption)
                        {
                            case OPTION_YES:
                                UART_WriteLine("");
                                sprintf(fpath, MOUNT_POINT FILE_PATH);
                                if(f_unlink(fpath) == FR_OK) UART_WriteLine("Archivo eliminado con exito!");
                                else UART_WriteLine("Error: No se pudo eliminar el archivo.");
                                UART_WriteLine("Ingrese 1 + ENTER para salir...");
                                break;

                            case OPTION_NO:
                                showMainMenu();
                                stateMenu = MAIN_MENU;
                                break;
                        }
                    }
                }      
                break;      

            case SCAN_FILES:
                if(UART_Available())
                {
                    menuOption = UART_readOption();
                    if(menuOption != INVALID_OPTION)
                    { 
                        showMainMenu();
                        stateMenu = MAIN_MENU;
                    }
                }                       
                break;    

            default:
                stateMenu = START;
                break;                                                                      
        }      

        sleepUntilNextInterrupt();
    }
}

/**************************************************************************/
/*! 
    @brief      Inicializa el hardware especifico para la placa.
*/
/**************************************************************************/
void initHW ()
{
    UART_Init(UART_USB);
    if(spiInit(SPI0))   uartWriteString(UART_USB, "Spi inicializado correctamente.\r\n");
    gpioInit( MEMORY_CS, GPIO_OUTPUT );
    gpioWrite(MEMORY_CS, HIGH);
}

/**************************************************************************/
/*! 
    @brief      Escanea la memoria en busqueda de archivos.

    @param[in]  path
                El directorio donde comenzar a buscar los archivos. 
    @return     El código de resultado de la operacion.
*/
/**************************************************************************/
FRESULT scan_files (char* path)
{
   FRESULT res;
   DIR dir;
   UINT i;
   static FILINFO fno;
   char outputStr[MAX_TEXT];

   res = f_opendir(&dir, path);                       /* Open the directory */
   if (res == FR_OK) {
      for (;;) {
         res = f_readdir(&dir, &fno);                   /* Read a directory item */
         if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
         if (fno.fattrib & AM_DIR) {                    /* It is a directory */
            i = strlen(path);
            sprintf(&path[i], "/%s\r\n", fno.fname);
            res = scan_files(path);                    /* Enter the directory */
            if (res != FR_OK) break;
            path[i] = 0;
         } else {                                       /* It is a file. */
            sprintf(outputStr, "%s/%s", path, fno.fname);
            UART_WriteLine(outputStr);
         }
      }
      f_closedir(&dir);
   }

   return res;
}

/**************************************************************************/
/*!
 * @brief   Muestra el menu principal en la terminal serie
 */
/**************************************************************************/
static void showMainMenu()
{
	showMenu(mainMenuHeader, mainMenuFooter, OptionsMenu,  sizeof(OptionsMenu)/sizeof(*OptionsMenu));
}

/**************************************************************************/
/*!
 * @brief   Muestra en la terminal serie un menu con sus opciones

 * @param   menuText Texto del titulo del menu
 * @param	*menuFooter Texto de las indicaciones para comandar el menu
 * @param	**options Opciones que posee el menu
 * @param	nrOptions Cantidad de opciones que posee el menu
 */
/**************************************************************************/
static void showMenu(const char *menuText, const char *menuFooter, const char **options, uint8_t nrOptions)
{
	uint8_t i;

	UART_clearTerminal();
	UART_cursorHome();
	UART_WriteLine("");
	UART_WriteLine(menuHeader);
	UART_WriteLine(menuText);
	UART_WriteLine(menuHeader);
	UART_WriteLine("");
	if(options != NULL)  UART_ShowOptions(options, nrOptions);
	else
	{
		for(i = 0; i < nrOptions; i++)	UART_WriteLine(""); // agrego espacio para mostrar la informacion
	}
	UART_WriteLine("");
	UART_WriteLine(menuHeader);
	if(menuFooter != NULL) UART_Write(menuFooter);
}