#ifndef _FSS25FL_H_
#define _FSS25FL_H_

#include "board.h"      // LPCOpen board support
#include "diskio.h"		// FatFs lower layer API

#define FAT_SECTOR_SIZE                     512
#define FLASH_SECTOR_SIZE                   4096

#define MOUNT_POINT                         ""

bool        S25FL_begin                     (FATFS *_fatFs);
int         S25FL_format                    (FATFS *_fatFs);
DSTATUS     S25FL_FatFs_DiskStatus          ( void );
DSTATUS     S25FL_FatFs_DiskInitialize      ( void );
DRESULT     S25FL_FatFs_DiskRead            (BYTE *buff, DWORD sector, UINT count);
#if !FF_FS_READONLY
DRESULT     S25FL_FatFs_DiskWrite           (const BYTE *buff, DWORD sector, UINT count);
#endif
DRESULT     S25FL_FatFs_DiskIoCtl           (BYTE cmd, void *buff);


#endif  //_FSS25FL_H_