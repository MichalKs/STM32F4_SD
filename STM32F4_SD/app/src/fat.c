/**
 * @file:   fat.c
 * @brief:  FAT file system implementation.
 * @date:   4 maj 2014
 * @author: Michal Ksiezopolski
 * 
 * @verbatim
 * Copyright (c) 2014 Michal Ksiezopolski.
 * All rights reserved. This program and the 
 * accompanying materials are made available 
 * under the terms of the GNU Public License 
 * v3.0 which accompanies this distribution, 
 * and is available at 
 * http://www.gnu.org/licenses/gpl.html
 * @endverbatim
 */

#include <fat.h>
#include <stdio.h>
#include <utils.h>
#define dprint(...) printf("FAT: "); printf(__VA_ARGS__)

/**
 * @brief Partition table entry structure.
 */
typedef struct {
  uint8_t   activeFlag;     ///< Active flag, 0x80 - bootable, 0x00 - inactive
  uint8_t   startCHS[3];    ///< CHS address of first absolute sector in partition
  uint8_t   type;           ///< Partition type
  uint8_t   stopCHS[3];     ///< CHS address of last absolute sector in partition
  uint32_t  partitionLBA;   ///< LBA of first absolute sector in partition
  uint32_t  size;           ///< Number of sectors in partition
} __attribute((packed)) FAT_PartitionTableEntry;

/**
 * @brief Master boot record structure.
 */
typedef struct {
  uint8_t bootcode[446];                      ///< Bootcode
  FAT_PartitionTableEntry partitionTable[4];  ///< Partition table
  uint16_t signature;                         ///< Signature 0xaa55
} __attribute((packed)) FAT_MBR;

/**
 * @brief FAT partition boot sector
 */
typedef struct {
  uint8_t   jmpcode[3];        ///< Jump instruction to boot code
  uint8_t   OEM_Name[8];       ///< String name
  uint16_t  bytesPerSector;    ///< Number of bytes per sector - may be 512, 1024, 2048 or 4096
  uint8_t   sectorsPerCluster; ///< Number of sectors per allocation unit (cluster). Legal values: 1,2,4,8,16,32,64 and 128 @warning Cluster size in bytes cannot be bigger than 32Kbytes
  uint16_t  reservedSectors;   ///< Number of reserved sectors. For FAT12 and FAT16 this value should be 1. For FAT32 usually 32
  uint8_t   numberOfFATs;      ///< Number of FAT data structures on partition (should be 2)
  uint16_t  rootEntries;       ///< Number of 32-byte entries in root directory
  uint16_t  totalSectors16;    ///< Total count of sectors on volume
  uint8_t   mediaType;         ///< 0xf8 - non-removable media, 0xf0 - removable media
  uint16_t  sectorsPerFAT;     ///< Number of sectors occupied by one FAT
  uint16_t  sectorsPerTrack;   ///< Number of sectors per track
  uint16_t  headsPerCylinder;  ///< Number of heads per cylinder
  uint32_t  hiddenSectors;     ///< Number of hidden sectors preceding this partition
  uint32_t  totalSectors32;    ///< New 32-bit count of sectors on volume (optional in FAT12/16)
  uint8_t   driveNumber;       ///< 0x00 - floppy disk, 0x80 - hard disk
  uint8_t   unused;            ///< Unused - always 0
  uint8_t   bootSignature;     ///< Extended boot signature - 0x29 - indicates the following 3 fields are present
  uint32_t  volumeID;          ///< Volume serial number
  uint8_t   volumeLabel[11];   ///< Volume label
  uint8_t   filesystem[8];     ///< File system name - string
  uint8_t   bootcode[448];     ///<
  uint16_t  signature;         ///< Boot signature 0xaa55
} __attribute((packed)) FAT_BootSector;

/**
 * Root directory entry
 */
typedef struct {
  uint8_t filename[8];
  uint8_t extension[3];
  uint8_t attributes;
  uint8_t unused;
  uint8_t createTimeMs;
  uint16_t creationTime;
  uint16_t creationDate;
  uint16_t lastAccess;
  uint16_t whatever;
  uint16_t lastModifiedTime;
  uint16_t lastModifiedDate;
  uint16_t firstCluster;
  uint32_t fileSize;
} __attribute((packed)) FAT_RootDirEntry;

/**
 * @brief Structure containing info about partition structure
 */
typedef struct {
  uint8_t partitionNumber;


} FAT_PartitionInfo;

/**
 * @brief Structure containing info about disk structure
 */
typedef struct {
  uint8_t diskID;
  FAT_PartitionInfo partitionInfo[4];
} FAT_DiskInfo;

#define FAT_MAX_DISKS 2

FAT_DiskInfo mountedDisks[FAT_MAX_DISKS];

/**
 * @brief Physical layer callbacks.
 */
typedef struct {
  void (*phyInit)(void);
  uint8_t (*phyReadSectors)(uint8_t* buf, uint32_t sector, uint32_t count);
  uint8_t (*phyWriteSectors)(uint8_t* buf, uint32_t sector, uint32_t count);
} FAT_PhysicalCb;

static FAT_PhysicalCb phyCallbacks;

int8_t FAT_Init(void (*phyInit)(void),
    uint8_t (*phyReadSectors)(uint8_t* buf, uint32_t sector, uint32_t count),
    uint8_t (*phyWriteSectors)(uint8_t* buf, uint32_t sector, uint32_t count)) {

  phyCallbacks.phyInit = phyInit;
  phyCallbacks.phyReadSectors = phyReadSectors;
  phyCallbacks.phyWriteSectors = phyWriteSectors;

  // initialize physical layer
  phyCallbacks.phyInit();

  uint8_t buf[512];

  // Read MBR
  phyCallbacks.phyReadSectors(buf, 0, 1);

  FAT_MBR* mbr = (FAT_MBR*)buf;
  if (mbr->signature != 0xaa55) {
    dprint("Invalid disk signature %04x\r\n", mbr->signature);
    return -1;
  } else {
    dprint("Valid disk signature\r\n");
  }

  hexdump(mbr->partitionTable, sizeof(FAT_PartitionTableEntry)*4);


  return 0;
}
