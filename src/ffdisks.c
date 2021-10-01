#include "ff.h"
#include "fsS25FL.h"


// Definitions required by FatFs according to ffconf.h
#if !FF_FS_READONLY && !FF_FS_NORTC
DWORD get_fattime (void)
{
	/* Get local time */
    RTC_TIME_T rtcTime;
	Chip_RTC_GetFullTime (LPC_RTC, &rtcTime);

	/* Pack date and time into a DWORD variable */
	return (  (DWORD) (rtcTime.time[RTC_TIMETYPE_YEAR] - 1980)  << 25)
		   | ((DWORD)  rtcTime.time[RTC_TIMETYPE_MONTH]         << 21)
		   | ((DWORD)  rtcTime.time[RTC_TIMETYPE_DAYOFMONTH]    << 16)
		   | ((DWORD)  rtcTime.time[RTC_TIMETYPE_HOUR]          << 11)
		   | ((DWORD)  rtcTime.time[RTC_TIMETYPE_MINUTE]        << 5)
		   | ((DWORD)  rtcTime.time[RTC_TIMETYPE_SECOND]        >> 1);
}
#endif

#if FF_MULTI_PARTITION
PARTITION VolToPart[FF_VOLUMES] = 
{
    {0, 1},             // "0:" ==> Physical drive 0, 1st partition (MMC:)
    {1, 1},             // "1:" ==> Physical drive 1, 1st partition (USB:)
};
#endif


DSTATUS disk_status (
	BYTE pdrv           /* Physical drive nmuber to identify the drive */
)
{    
    return S25FL_FatFs_DiskStatus ();
}


DSTATUS disk_initialize (
	BYTE pdrv           /* Physical drive nmuber to identify the drive */
)
{
    return S25FL_FatFs_DiskInitialize ();
}


DRESULT disk_read (
	BYTE pdrv,          /* Physical drive nmuber to identify the drive */
	BYTE *buff,         /* Data buffer to store read data */
	DWORD sector,       /* Start sector in LBA */
	UINT count          /* Number of sectors to read */
)
{
	if (!buff || !count) 
    {
		return RES_PARERR;
	}
    else
    {
        return S25FL_FatFs_DiskRead (buff, sector, count);
    }
}


DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	if (!buff || !count)
    {
		return RES_PARERR;
	}
    else
    {
        return S25FL_FatFs_DiskWrite (buff, sector, count);
    }
}


DRESULT disk_ioctl (
	BYTE pdrv,          /* Physical drive nmuber (0..) */
	BYTE cmd,           /* Control code */
	void *buff          /* Buffer to send/receive control data */
)
{   
    return S25FL_FatFs_DiskIoCtl (cmd, buff);
}
