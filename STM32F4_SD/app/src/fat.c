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
 * @brief FAT 16 and 12 partition boot sector
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
} __attribute((packed)) FAT16_BootSector;

/**
 * @brief FAT 32 partition boot sector
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

  uint32_t  sectorsPerFAT32;   ///<
  uint16_t  fags;            ///<
  uint16_t  fsVersion;     ///<
  uint32_t  rootCluster;          ///<
  uint16_t  fsInfo;   ///<
  uint16_t  backupBootSector;     ///<
  uint8_t   reserved[12];
  uint8_t   driveNumber;       ///< 0x00 - floppy disk, 0x80 - hard disk
  uint8_t   reserved1;
  uint8_t   bootSignature;     ///< Extended boot signature - 0x29 - indicates the following 3 fields are present
  uint32_t  volumeID;          ///< Volume serial number
  uint8_t   volumeLabel[11];   ///< Volume label
  uint8_t   filesystem[8];     ///< File system name - string
  uint8_t   bootcode[420];     ///< FIXME
  uint16_t  signature;         ///< Boot signature 0xaa55
} __attribute((packed)) FAT32_BootSector;

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
  uint8_t type;
  uint32_t startAddress;
  uint32_t length;
  uint32_t startFatSector;

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
  }
  dprint("Valid disk signature\r\n");


  hexdump((uint8_t*)mbr->partitionTable, sizeof(FAT_PartitionTableEntry)*4);

  mountedDisks[0].diskID = 0;

  int i;

  for (i=0; i<4; i++) {
    if (mbr->partitionTable[i].type == 0 ) {
      dprint("Found empty partition\r\n");
    } else {
      dprint("Partition %d type is: %02x\r\n", i, mbr->partitionTable[i].type);
      dprint("Partition %d start sector is: %u\r\n", i, (unsigned int)mbr->partitionTable[i].partitionLBA);
      dprint("Partition %d size is: %u\r\n", i, (unsigned int)mbr->partitionTable[i].size);

      mountedDisks[0].partitionInfo[i].partitionNumber = i;
      mountedDisks[0].partitionInfo[i].type = mbr->partitionTable[i].type;
      mountedDisks[0].partitionInfo[i].startAddress = mbr->partitionTable[i].partitionLBA;
      mountedDisks[0].partitionInfo[i].length = mbr->partitionTable[i].size;
    }
  }

  // Read boot sector of first partition
  phyCallbacks.phyReadSectors(buf, mountedDisks[0].partitionInfo[0].startAddress, 1);
  hexdump(buf, 512);

  FAT32_BootSector* bootSector = (FAT32_BootSector*)buf;

  if (bootSector->signature != 0xaa55) {
    dprint("Invalid partition signature %04x\r\n", mbr->signature);
    return -2;
  }

  dprint("Valid partition signature\r\n");

  dprint("Partition size is %d\r\n", (unsigned int)bootSector->totalSectors32);
  dprint("Reserved sectors = %d\r\n", (unsigned int)bootSector->reservedSectors);
  dprint("Bytes per sector %d\r\n", (unsigned int)bootSector->bytesPerSector);
  dprint("Hidden sectors %d\r\n", (unsigned int)bootSector->hiddenSectors);
  dprint("Sectors per cluster =  %d\r\n", (unsigned int)bootSector->sectorsPerCluster);
  dprint("Number of FATs =  %d\r\n", (unsigned int)bootSector->numberOfFATs);
  dprint("Sectors per FAT =  %d\r\n", (unsigned int)bootSector->sectorsPerFAT32);

  uint32_t rootDirSector = mountedDisks[0].partitionInfo[0].startAddress +
      bootSector->reservedSectors + 2*bootSector->sectorsPerFAT32;


  return 0;
}
