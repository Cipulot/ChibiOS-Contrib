/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2007        */
/*-----------------------------------------------------------------------*/
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/

#include "hal.h"
#include "ffconf.h"
#include "ff.h"
#include "diskio.h"
#include "usbh/dev/msd.h"
#include "fatfs_devices.h"

#if HAL_USE_MMC_SPI && HAL_USE_SDC
#error "cannot specify both MMC_SPI and FATFSDEV_MMC drivers"
#endif

// sanity check for no FAT option selected
// why is the FAT sources being pulled into the build?
#if !HAL_USE_MMC_SPI && !HAL_USE_SDC & !HAL_USBH_USE_MSD
#error "MMC_SPI, FATFSDEV_MMC or USBH_MSD driver must be specified. None was."
#endif

#if !defined(FATFS_HAL_DEVICE)
#if HAL_USE_MMC_SPI
#define FATFS_HAL_DEVICE MMCD1
#else
#define FATFS_HAL_DEVICE SDCD1
#endif
#endif

#if HAL_USE_MMC_SPI
extern MMCDriver FATFS_HAL_DEVICE;
#endif
#if HAL_USE_SDC
extern SDCDriver FATFS_HAL_DEVICE;
#endif
#if HAL_USBH_USE_MSD
#endif

#if HAL_USE_RTC
extern RTCDriver RTCD1;
#endif


/*-----------------------------------------------------------------------*/
/* Initialize a Drive                                                    */

DSTATUS disk_initialize (
    BYTE pdrv         /* Physical drive number (0..) */
)
{
  DSTATUS stat;

  switch (pdrv) {
#if HAL_USE_MMC_SPI
    case FATFSDEV_MMC:
      stat = 0;
      /* It is initialized externally, just reads the status.*/
      if (blkGetDriverState(&FATFS_HAL_DEVICE) != BLK_READY)
        stat |= STA_NOINIT;
      if (mmcIsWriteProtected(&FATFS_HAL_DEVICE))
        stat |=  STA_PROTECT;
      return stat;
#elif HAL_USE_SDC
    case FATFSDEV_MMC:
      stat = 0;
      /* It is initialized externally, just reads the status.*/
      if (blkGetDriverState(&FATFS_HAL_DEVICE) != BLK_READY)
        stat |= STA_NOINIT;
      if (blkIsWriteProtected(&FATFS_HAL_DEVICE))
        stat |= STA_PROTECT;
      return stat;
#endif
#if HAL_USBH_USE_MSD
    case FATFSDEV_MSD:
      stat = 0;
      /* It is initialized externally, just reads the status.*/
      if (blkGetDriverState(&MSBLKD[0]) != BLK_READY)
        stat |= STA_NOINIT;
      return stat;
#endif
  }
  return STA_NOINIT;
}

/*-----------------------------------------------------------------------*/
/* Return Disk Status                                                    */

DSTATUS disk_status (
    BYTE pdrv         /* Physical drive number (0..) */
)
{
  DSTATUS stat;

  switch (pdrv) {
#if HAL_USE_MMC_SPI
  case FATFSDEV_MMC:
    stat = 0;
    /* It is initialized externally, just reads the status.*/
    if (blkGetDriverState(&FATFS_HAL_DEVICE) != BLK_READY)
      stat |= STA_NOINIT;
    if (mmcIsWriteProtected(&FATFS_HAL_DEVICE))
      stat |= STA_PROTECT;
    return stat;
#elif HAL_USE_SDC
  case FATFSDEV_MMC:
    stat = 0;
    /* It is initialized externally, just reads the status.*/
    if (blkGetDriverState(&FATFS_HAL_DEVICE) != BLK_READY)
      stat |= STA_NOINIT;
    if (blkIsWriteProtected(&FATFS_HAL_DEVICE))
      stat |= STA_PROTECT;
    return stat;
#endif
#if HAL_USBH_USE_MSD
    case FATFSDEV_MSD:
      stat = 0;
      /* It is initialized externally, just reads the status.*/
      if (blkGetDriverState(&MSBLKD[0]) != BLK_READY)
        stat |= STA_NOINIT;
      return stat;
#endif
  }
  return STA_NOINIT;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */

DRESULT disk_read (
    BYTE pdrv,        /* Physical drive number (0..) */
    BYTE *buff,       /* Data buffer to store read data */
    DWORD sector,     /* Sector address (LBA) */
    UINT count        /* Number of sectors to read (1..255) */
)
{
  switch (pdrv) {
#if HAL_USE_MMC_SPI
  case FATFSDEV_MMC:
    if (blkGetDriverState(&FATFS_HAL_DEVICE) != BLK_READY)
      return RES_NOTRDY;
    if (mmcStartSequentialRead(&FATFS_HAL_DEVICE, sector))
      return RES_ERROR;
    while (count > 0) {
      if (mmcSequentialRead(&FATFS_HAL_DEVICE, buff))
        return RES_ERROR;
      buff += MMCSD_BLOCK_SIZE;
      count--;
    }
    if (mmcStopSequentialRead(&FATFS_HAL_DEVICE))
        return RES_ERROR;
    return RES_OK;
#elif HAL_USE_SDC
  case FATFSDEV_MMC:
    if (blkGetDriverState(&FATFS_HAL_DEVICE) != BLK_READY)
      return RES_NOTRDY;
    if (blkRead(&FATFS_HAL_DEVICE, sector, buff, count))
      return RES_ERROR;
    return RES_OK;
#endif
#if HAL_USBH_USE_MSD
    case FATFSDEV_MSD:
    /* It is initialized externally, just reads the status.*/
    if (blkGetDriverState(&MSBLKD[0]) != BLK_READY)
      return RES_NOTRDY;
    if (usbhmsdLUNRead(&MSBLKD[0], sector, buff, count))
      return RES_ERROR;
    return RES_OK;
#endif
  }
  return RES_PARERR;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */

#if !FF_FS_READONLY
DRESULT disk_write (
    BYTE pdrv,        /* Physical drive number (0..) */
    const BYTE *buff, /* Data to be written */
    DWORD sector,     /* Sector address (LBA) */
    UINT count        /* Number of sectors to write (1..255) */
)
{
  switch (pdrv) {
#if HAL_USE_MMC_SPI
  case FATFSDEV_MMC:
    if (blkGetDriverState(&FATFS_HAL_DEVICE) != BLK_READY)
        return RES_NOTRDY;
    if (mmcIsWriteProtected(&FATFS_HAL_DEVICE))
        return RES_WRPRT;
    if (mmcStartSequentialWrite(&FATFS_HAL_DEVICE, sector))
        return RES_ERROR;

    while (count > 0) {
        // invalidate cache on buffer
        cacheBufferFlush(buff, MMCSD_BLOCK_SIZE);

        if (mmcSequentialWrite(&FATFS_HAL_DEVICE, buff))
            return RES_ERROR;
        buff += MMCSD_BLOCK_SIZE;
        count--;
    }
    if (mmcStopSequentialWrite(&FATFS_HAL_DEVICE))
        return RES_ERROR;
    return RES_OK;
#elif HAL_USE_SDC
  case FATFSDEV_MMC:
    if (blkGetDriverState(&FATFS_HAL_DEVICE) != BLK_READY)
      return RES_NOTRDY;

    // invalidate cache on buffer
    cacheBufferFlush(buff, count * MMCSD_BLOCK_SIZE);

    if (blkWrite(&FATFS_HAL_DEVICE, sector, buff, count))
        return RES_ERROR;

    return RES_OK;
#endif
#if HAL_USBH_USE_MSD
  case FATFSDEV_MSD:
    /* It is initialized externally, just reads the status.*/
    if (blkGetDriverState(&MSBLKD[0]) != BLK_READY)
      return RES_NOTRDY;

    // invalidate cache on buffer
    cacheBufferFlush(buff, count * MSBLKD[0].info.blk_size);

    if (usbhmsdLUNWrite(&MSBLKD[0], sector, buff, count))
      return RES_ERROR;
    return RES_OK;
#endif
  }
  return RES_PARERR;
}
#endif /* _FS_READONLY */

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */

DRESULT disk_ioctl (
    BYTE pdrv,        /* Physical drive number (0..) */
    BYTE cmd,         /* Control code */
    void *buff        /* Buffer to send/receive control data */
)
{
  (void)buff;

  switch (pdrv) {
#if HAL_USE_MMC_SPI
  case FATFSDEV_MMC:
    switch (cmd) {
    case CTRL_SYNC:
        return RES_OK;
#if FF_MAX_SS > FF_MIN_SS
    case GET_SECTOR_SIZE:
        *((WORD *)buff) = MMCSD_BLOCK_SIZE;
        return RES_OK;
#endif
#if FF_USE_TRIM
    case CTRL_TRIM:
        mmcErase(&FATFS_HAL_DEVICE, *((DWORD *)buff), *((DWORD *)buff + 1));
        return RES_OK;
#endif
    default:
        return RES_PARERR;
    }
#elif HAL_USE_SDC
  case FATFSDEV_MMC:
    BlockDeviceInfo bdi;

    switch (cmd) {
      case CTRL_SYNC:
          return RES_OK;
      case GET_SECTOR_COUNT:
        if (blkGetInfo(&FATFS_HAL_DEVICE, &bdi)) {
          return RES_ERROR;
        }
        *((DWORD *)buff) = bdi.blk_num;
        return RES_OK;
#if FF_MAX_SS > FF_MIN_SS
      case GET_SECTOR_SIZE:
        if (blkGetInfo(&FATFS_HAL_DEVICE, &bdi)) {
          return RES_ERROR;
        }
        *((WORD *)buff) = bdi.blk_size;
        return RES_OK;
#endif
#if FF_USE_TRIM
      case GET_BLOCK_SIZE:
        /* unsupported */
        break;
      case CTRL_TRIM:
        /* unsupported */
        break;
#endif
      default:
          return RES_PARERR;
    }
#endif
#if HAL_USBH_USE_MSD
    case FATFSDEV_MSD:
      switch (cmd) {
        case CTRL_SYNC:
            return RES_OK;
        case GET_SECTOR_COUNT:
            *((DWORD *)buff) = MSBLKD[0].info.blk_num;
            return RES_OK;
#if FF_MAX_SS > FF_MIN_SS
        case GET_SECTOR_SIZE:
            *((WORD *)buff) = MSBLKD[0].info.blk_size;
            return RES_OK;
#endif
#if FF_USE_TRIM
#error "unimplemented yet!"
//    case CTRL_TRIM:
//      ....
//      return RES_OK;
#endif
        default:
            return RES_PARERR;
      }
#endif
  }
  return RES_PARERR;
}

DWORD get_fattime(void) {
#if HAL_USE_RTC
    RTCDateTime timespec;

    rtcGetTime(&RTCD1, &timespec);
    return rtcConvertDateTimeToFAT(&timespec);
#else
    return ((uint32_t)0 | (1 << 16)) | (1 << 21); /* wrong but valid time */
#endif
}
