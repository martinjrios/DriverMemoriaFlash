#include "fsS25FL.h"
#include "S25FL.h"
#include <stdlib.h>
#include <string.h>

static uint32_t _fatSectorCount();
static uint32_t _fatSectorAddress(uint32_t sector);
static uint32_t _flashSectorBase(uint32_t address);
static uint32_t _flashSectorOffset(uint32_t address);

/**************************************************************************/
/*! 
    @brief      Inicializa el sistema de archivos en la memoria flash.

    @param[in]  _fatFs
                Puntero a la estructura del sistema de archivos.
    @return     True si pudo montar correctamente el sistema de archivos.
*/
/**************************************************************************/
bool S25FL_begin(FATFS *_fatFs)
{
    FRESULT r = f_mount(_fatFs, MOUNT_POINT, 1);  // Se monta el sistema de archivos
    if (r != FR_OK)
    {
        return false;
    }
    return true;
}

/**************************************************************************/
/*! 
    @brief      Formatea la memoria flash con el sistema de archivos FAT.

    @param[in]  _fatFs
                Puntero a la estructura del sistema de archivos.
    @return     0 si se pudo formatear correctamente. 
                -1 en caso de error.
*/
/**************************************************************************/
int S25FL_format(FATFS *_fatFs)
{
    FRESULT r;
    uint8_t buf[FAT_SECTOR_SIZE] = {0};          // buffer para un sector

    #if FF_MULTI_PARTITION
    // Partition the flash with 1 partition that takes the entire space.
    DWORD plist[] = {100, 0, 0, 0};  // 1 primary partition with 100% of space.
    r = f_fdisk(0, plist, buf);

    if (r != FR_OK) {
        return r;
    }
    #endif

    // Se genera el sistema de archivos
    r = f_mkfs(MOUNT_POINT, NULL, buf, sizeof(buf));
    if (r != FR_OK)
    {
        return r;
    }

    // Se comprueba que el sistema de archivos se puede montar
    if (!S25FL_begin(_fatFs))
    {
        return -1;
    }

    return 0;
}

/**************************************************************************/
/*! 
    @brief      Inicializa el dispositivo de almacenamiento.

    @return     Siempre 0 ya que se va a usar un solo dispositivo.
*/
/**************************************************************************/
DSTATUS S25FL_FatFs_DiskInitialize ( void )
{
    return 0;
}


/**************************************************************************/
/*! 
    @brief      Obtiene el estado actual del dispositivo.

    @return     Estado del dispositivo.
*/
/**************************************************************************/
DSTATUS S25FL_FatFs_DiskStatus ( void )
{
    return(S25FL_readStatus());
}

/**************************************************************************/
/*! 
    @brief      Lee los datos almacenados en un sector o varios sectores del dispositivo.

    @param[out] buff
                El buffer donde se almacenaran los datos leidos.
    @param[in]  sector
                El numero de sector donde comenzar la lectura.
    @param[in]  count
                La cantidad de sectores a leer.

    @return     DRESULT (ver diskio.h)
*/
/**************************************************************************/
DRESULT S25FL_FatFs_DiskRead (BYTE *buff, DWORD sector, UINT count)
{
    // Se convierte el numero de sector del sistema FAT al correspondiente de la flash
    // y luego se leen la cantidad de sectores en el buffer provisto
    uint32_t address = _fatSectorAddress(sector);
    if (S25FL_readBuffer(address, buff, count*FAT_SECTOR_SIZE) <= 0)
    {
        return RES_ERROR;
    }
    return RES_OK;    
}

/**************************************************************************/
/*! 
    @brief      Escribe los datos en un sector o varios sectores del dispositivo.

    @param[out] buff
                El buffer con los datos a escribir.
    @param[in]  sector
                El numero de sector donde comenzar la escritura.
    @param[in]  count
                La cantidad de sectores a escribir.

    @return     DRESULT (ver diskio.h)
*/
/**************************************************************************/
DRESULT S25FL_FatFs_DiskWrite (const BYTE *buff, DWORD sector, UINT count)
{
    uint8_t *_flashSectorBuffer;
    _flashSectorBuffer = (uint8_t*)malloc(FLASH_SECTOR_SIZE);

    if (_flashSectorBuffer == NULL) {
        return RES_ERROR;
    }

    // Se itera sobre cada sector FAT y luego se lo actualiza.
    // Se trata de hacer una iteracion inteligente, minimizando la cantidad
    // de escrituras en los sectores de la flash, al combinar varias escrituras
    // en sectores FAT contiguos en un solo ciclo de escritura/actualizacion
    // del loop.
    for (int i=0; i < count; )
    {
        // Se determina la direccion de inicio de la flash correspondiente a este sector FAT  
        uint32_t address = _fatSectorAddress(sector+i);
        uint32_t sectorStart = _flashSectorBase(address);

        // Se calculan cuantos sectores FAT pueden ser escritos en este sector de la flash
        int available = ((sectorStart + FLASH_SECTOR_SIZE) - address)/FAT_SECTOR_SIZE;

        // Se determinan la cantidad de sectores FAT a escribir para llenar el
        // sector de la flash, basado en la cantidad que quedan para escribir
        int countToWrite = MIN(count-i, available);

        // Se lee el sector entero y se lo guarda en RAM
        if (S25FL_readBuffer(sectorStart, _flashSectorBuffer, FLASH_SECTOR_SIZE) != FLASH_SECTOR_SIZE)
        {
            // Error, no se pudo leer el sector antes de realizar la escritura
            return RES_ERROR;
        }

        // Se modifica la parte del sector apropiada con el nuevo bloque de datos
        uint16_t blockOffset = _flashSectorOffset(address);
        memcpy(_flashSectorBuffer+blockOffset, buff+(i*FAT_SECTOR_SIZE), countToWrite*FAT_SECTOR_SIZE);

        // Se borra el sector.
        // El numero de sector a borrar se determina tomando la direccion base de
        // inicio de ese sector y se la divide por el tamaño del sector.
        if (!S25FL_eraseSector(sectorStart/FLASH_SECTOR_SIZE))
        {
            // Error, no se pudo borrar el sector.
            return RES_ERROR;
        }

        // Se escrite el sector en la flash con los datos actualizados
        if (S25FL_writeBuffer(sectorStart, _flashSectorBuffer, FLASH_SECTOR_SIZE) != FLASH_SECTOR_SIZE)
        {
            // Error, no se pudo escribir el sector
            return RES_ERROR;
        }

        // Se incrementa el contador de acuerdo a la cantidad de sectores FAT que fueron escritos
        i += countToWrite;
    }

    if (_flashSectorBuffer != NULL) free(_flashSectorBuffer);  // Se libera la memoria tomada para el buffer del sector
    
    return RES_OK;
}

/**************************************************************************/
/*! 
    @brief      Controla diversas caracteristicas del dispositivo.

    @param[in]  cmd
                El comando de control.
    @param[in]  buff
                El puntero al parametro depende del codigo de comando. 
                No tiene importancia si el comando no tiene parametros para pasar.  
    @return     DRESULT (ver diskio.h)
*/
/**************************************************************************/
DRESULT S25FL_FatFs_DiskIoCtl (BYTE cmd, void *buff)
{
    //   http://elm-chan.org/fsw/ff/en/dioctl.html
    switch(cmd) 
    {
        case CTRL_SYNC:
        // No se hace nada ya que no hay nada para sincronizar en la flash
        break;
        case GET_SECTOR_COUNT:
        {
            // Obtiene la cantidad de sectores FAT disponibles
            DWORD* count = (DWORD*)buff;
            *count = _fatSectorCount();
            break;
        }
        case GET_SECTOR_SIZE:
        {
            // Obtiene el tamaño del sector FAT
            WORD* count = (WORD*)buff;
            *count = FAT_SECTOR_SIZE;
            break;
        }
        case GET_BLOCK_SIZE:
        {
            // Devuelve el numero de sectores FAT por cada sector flash
            // Se utiliza para alinear los datos para un borrado eficiente
            DWORD* count = (DWORD*)buff;
            *count = FLASH_SECTOR_SIZE/FAT_SECTOR_SIZE;
            break;
        }
        case CTRL_TRIM:
        // No soportado por el momento
        break;
    }
    return RES_OK;
}

/**************************************************************************/
/*! 
    @brief      Obtiene la cantidad de sectores FAT totales en la flash.

    @return     La cantidad de sectores FAT de la flash.
*/
/**************************************************************************/
static uint32_t _fatSectorCount()
{
    return (S25FL_pageSize()*S25FL_numPages())/FAT_SECTOR_SIZE;
}

/**************************************************************************/
/*! 
    @brief      Obtiene la direccion en memoria flash correspondiente al
                sector FAT especificado.

    @param[in]  sector
                El numero de sector FAT.
    @return     La direccion de memoria del sector en la flash.
*/
/**************************************************************************/
static uint32_t _fatSectorAddress(uint32_t sector)
{
    return sector*FAT_SECTOR_SIZE;
}

/**************************************************************************/
/*! 
    @brief      Obtiene la direccion de comienzo del sector flash para la direccion
                flash especificada.

    @param[in]  address
                La direccion en memoria flash.
    @return     La direccion donde comienza el sector flash.
*/
/**************************************************************************/
static uint32_t _flashSectorBase(uint32_t address)
{
    // Los 12 bits superiores de la direccion dan la direccion de comienzo
    // del sector.
    return address & 0xFFF000;
}

/**************************************************************************/
/*! 
    @brief      Obtiene el corrimiento en bytes respecto a la direccion de
                comienzo del sector para direccion especificada.

    @param[in]  address
                La direccion en memoria flash.
    @return     El corrimiento en bytes.
*/
/**************************************************************************/
static uint32_t _flashSectorOffset(uint32_t address)
{
    // Los 12 bits inferiores de la direccion dan el offset respecto al
    // inicio del sector.
    return address & 0x000FFF;
}
