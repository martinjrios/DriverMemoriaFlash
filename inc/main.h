#ifndef __MAIN_H__
#define __MAIN_H__

#define FILE_PATH       "/log.txt"

#define OPTIONS_START_Y_POS		6
#define OPTIONS_START_X_POS		1

#define MAX_TEXT                32
#define MAX_INPUT_TEXT          255

typedef enum
{
	START,
	MAIN_MENU,
    FORMAT,
	WRITE_FILE,
	READ_FILE,
	DELETE_FILE,
	SCAN_FILES,
}stateMenu_t;

typedef enum
{
    OPTION_READ_FILE = 0,
	OPTION_WRITE_FILE,
	OPTION_DELETE_FILE,
	OPTION_SCAN_FILES,
    OPTION_FORMAT,
}optionMainMenu_t;

typedef enum
{
	OPTION_YES = 0,
	OPTION_NO,
}optionConfirm_t;

typedef enum
{
	WAITING_INPUT,
	WAITING_OPTION,
}stateSubmenu_t;

static const char menuHeader[] =            "+================================================================+";
static const char mainMenuHeader[] =        "|                               MENU                             |";
static const char mainMenuFooter[] =        "Ingrese el numero de una de las opciones y presione ENTER: ";
static const char subMenuFooter[] =         "Presione 0 para volver al menu principal o 1 para igresar nuevamente el texto: ";
static const char invalidOption[] =         "Opcion invalida!";
static const char formatOptionText[] =      "                   DESEA FORMATEAR LA MEMORIA?                    ";
static const char writeFileOptionText[] =   "Ingrese el texto a guardar en el archivo y presione ENTER: ";
static const char readFileOptionText[] =    "                      CONTENIDO DEL ARCHIVO:                      ";
static const char deleteFileOptionText[] =  "                    DESEA ELIMINAR EL ARCHIVO?                    ";
static const char scanFilesOptionText[] =   "                     CONTENIDO DE LA MEMORIA:                     ";
static const char formatWaitText1[] =       "Formateando la memoria Flash...";
static const char formatWaitText2[] =       "Esto puede demorar algunos minutos. Por favor espere...";
static const char errorText[] =             "Ha ocurrido un error. Intente nuevamente...";
static const char TextInput[] =             "Se ha ingresado: ";

static const char *OptionsMenu[] =
{
		"LEER ARCHIVO",
		"ESCRIBIR ARCHIVO",		
		"ELIMINAR ARCHIVO",
		"ESCANEAR ARCHIVOS EN MEMORIA",
        "FORMATEAR MEMORIA FLASH",
};

static const char *ConfirmOptions[] =
{
		"SI",
		"NO",
};

#endif   /*__MAIN_H__*/