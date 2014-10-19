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
#include <string.h>

#ifndef DEBUG
  #define DEBUG
#endif

#ifdef DEBUG
  #define print(str, args...) printf(""str"%s",##args,"")
  #define println(str, args...) printf("FAT--> "str"%s",##args,"\r\n")
#else
  #define print(str, args...) (void)0
  #define println(str, args...) (void)0
#endif

/**
 * @brief Partition table entry structure.
 *
 * @details The partition table is included in the first sector of the physical drive
 * and is used for identifying the partitions present on the disk.
 */
typedef struct {
  uint8_t   activeFlag;     ///< Active flag, 0x80 - bootable, 0x00 - inactive
  uint8_t   startCHS[3];    ///< CHS address of first absolute sector in partition
  uint8_t   type;           ///< Partition type
  uint8_t   stopCHS[3];     ///< CHS address of last absolute sector in partition
  uint32_t  partitionLBA;   ///< LBA of first absolute sector in partition
  uint32_t  size;           ///< Number of sectors in partition
} __attribute((packed)) FAT_PartitionTableEntry;


#define FAT_LAST_CLUSTER 0x0fffffff

typedef enum {
  PAR_TYPE_EMPTY = 0x00,
  PAR_TYPE_FAT12 = 0x01,
  PAR_TYPE_FAT16_32M = 0x04,
  PAR_TYPE_EXTENDED = 0x05,
  PAR_TYPE_FAT16 = 0x06,
  PAR_TYPE_NTFS = 0x07,
  PAR_TYPE_FAT32 = 0x0b,

} FAT_PartitonType;
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
  uint16_t  rootEntries;       ///< Number of 32-byte entries in root directory for FAT12/16. For FAT32 this is 0.
  uint16_t  totalSectors16;    ///< Total count of sectors on volume. For FAT32 this is 0.
  uint8_t   mediaType;         ///< 0xf8 - non-removable media, 0xf0 - removable media
  uint16_t  sectorsPerFAT;     ///< Number of sectors occupied by one FAT. For FAT32 this is 0.
  uint16_t  sectorsPerTrack;   ///< Number of sectors per track. Only for INT 0x13 on PC.
  uint16_t  headsPerCylinder;  ///< Number of heads per cylinder. Only for INT 0x13 on PC.
  uint32_t  hiddenSectors;     ///< Number of hidden sectors preceding this partition
  uint32_t  totalSectors32;    ///< New 32-bit count of sectors on volume (optional in FAT12/16)

  uint8_t   driveNumber;       ///< 0x00 - floppy disk, 0x80 - hard disk
  uint8_t   unused;            ///< Unused - always 0
  uint8_t   bootSignature;     ///< Extended boot signature - 0x29 - indicates the following 3 fields are present
  uint32_t  volumeID;          ///< Volume serial number
  uint8_t   volumeLabel[11];   ///< Volume label
  uint8_t   filesystem[8];     ///< File system name - string
  uint8_t   bootcode[448];     ///< Bootloader code
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
  uint16_t  rootEntries;       ///< Number of 32-byte entries in root directory for FAT12/16. For FAT32 this is 0.
  uint16_t  totalSectors16;    ///< Total count of sectors on volume. For FAT32 this is 0.
  uint8_t   mediaType;         ///< 0xf8 - non-removable media, 0xf0 - removable media
  uint16_t  sectorsPerFAT;     ///< Number of sectors occupied by one FAT. For FAT32 this is 0.
  uint16_t  sectorsPerTrack;   ///< Number of sectors per track. Only for INT 0x13 on PC.
  uint16_t  headsPerCylinder;  ///< Number of heads per cylinder. Only for INT 0x13 on PC.
  uint32_t  hiddenSectors;     ///< Number of hidden sectors preceding this partition
  uint32_t  totalSectors32;    ///< New 32-bit count of sectors on volume (optional in FAT12/16)

  uint32_t  sectorsPerFAT32;   ///< Number of sectors occupied by one FAT.
  uint16_t  flags;             ///<
  uint16_t  fsVersion;         ///< Version number. High byte - major revision. Low byte - minor revision.
  uint32_t  rootCluster;       ///< Cluster number of the first cluster of the root directory - usually 2
  uint16_t  fsInfo;            ///< Sector number of the FSINFO structure in the reserved area of the FAT32 volume - usually 1.
  uint16_t  backupBootSector;  ///< Sector number in reserved area with a copy of the boot records
  uint8_t   reserved[12];
  uint8_t   driveNumber;       ///< 0x00 - floppy disk, 0x80 - hard disk
  uint8_t   reserved1;
  uint8_t   bootSignature;     ///< Extended boot signature - 0x29 - indicates the following 3 fields are present
  uint32_t  volumeID;          ///< Volume serial number
  uint8_t   volumeLabel[11];   ///< Volume label
  uint8_t   filesystem[8];     ///< File system name - string
  uint8_t   bootcode[420];     ///< Bootloader code
  uint16_t  signature;         ///< Boot signature 0xaa55
} __attribute((packed)) FAT32_BootSector;

/**
 * @brief Root directory entry
 */
typedef struct {
  uint8_t filename[8];        ///< Name of file
  uint8_t extension[3];       ///< Extension of file
  uint8_t attributes;         ///< Attributes. 0x01 - read only, 0x02 - hidden, 0x04 - system, 0x08 - volume ID, 0x10 - directory, 0x20 - archive
  uint8_t unused;
  uint8_t createTimeTS;       ///< Creation time in tenths of a second
  uint16_t creationTime;      ///< Creation time - hours, minutes and seconds
  uint16_t creationDate;      ///< Year, month, day of file creation
  uint16_t lastAccess;        ///< Last access date - same format as creation date
  uint16_t firstClusterH;     ///< High 16 bits of cluster. Always 0 for FAT12/16
  uint16_t lastModifiedTime;  ///< Time of last modification
  uint16_t lastModifiedDate;  ///< Date of last modification
  uint16_t firstClusterL;     ///< Low 16 bits of cluster
  uint32_t fileSize;          ///< Size of file in bytes.
} __attribute((packed)) FAT_RootDirEntry;

/**
 * @brief Structure for keeping file information
 */
typedef struct {
  char filename[12]; ///< Zero ended file name and extension
  uint32_t firstCluster;
  uint32_t fileSize;
  uint8_t attributes;
  uint16_t lastModifiedTime;
  uint16_t lastModifiedDate;

} FAT_File;

/**
 * @brief Structure containing info about partition structure
 *
 * @details This is used by the application to store the relevant data
 * read from the partition table and the bootsector of the partition.
 *
 */
typedef struct {
  uint8_t partitionNumber;  ///< Number of the partition on disk (as in MBR)
  uint8_t type;             ///< Type of the partition - file system type
  uint32_t startAddress;    ///< Start address - LBA sector number
  uint32_t length;          ///< Length of partition in sectors
  uint32_t startFatSector;  ///< Sector where FAT start
  uint32_t rootDirSector;   ///< Sector where root directory starts
  uint32_t rootDirCluster;  ///< First cluster of root directory
  uint32_t dataStartSector; ///< Sector where data starts
  uint32_t sectorsPerCluster; ///< Number of sectors per cluster
  uint32_t bytesPerSector;    ///< Number of bytes per sector
} FAT_PartitionInfo;

/**
 * @brief Structure containing info about disk structure
 */
typedef struct {
  uint8_t diskID;
  FAT_PartitionInfo partitionInfo[4];
} FAT_DiskInfo;

#define FAT_MAX_DISKS 2

static FAT_DiskInfo mountedDisks[FAT_MAX_DISKS];

/**
 * @brief Physical layer callbacks.
 */
typedef struct {
  void (*phyInit)(void);
  uint8_t (*phyReadSectors)(uint8_t* buf, uint32_t sector, uint32_t count);
  uint8_t (*phyWriteSectors)(uint8_t* buf, uint32_t sector, uint32_t count);
} FAT_PhysicalCb;

static FAT_PhysicalCb phyCallbacks;



uint32_t FAT_Cluster2Sector(uint32_t cluster);
void FAT_ListRootDir(void);
uint32_t FAT_GetEntryInFAT(uint32_t cluster);
uint8_t FAT_FindFile(FAT_File* file);

/**
 * @brief Initialize FAT file system
 * @param phyInit Physical drive initialization function
 * @param phyReadSectors Read sectors function
 * @param phyWriteSectors Write sectors function
 * @return
 */
int8_t FAT_Init(void (*phyInit)(void),
    uint8_t (*phyReadSectors)(uint8_t* buf, uint32_t sector, uint32_t count),
    uint8_t (*phyWriteSectors)(uint8_t* buf, uint32_t sector, uint32_t count)) {

  phyCallbacks.phyInit = phyInit;
  phyCallbacks.phyReadSectors = phyReadSectors;
  phyCallbacks.phyWriteSectors = phyWriteSectors;

  // initialize physical layer
  phyCallbacks.phyInit();

  uint8_t buf[512]; // buffer for read sectors

  // Read MBR
  phyCallbacks.phyReadSectors(buf, 0, 1);

  FAT_MBR* mbr = (FAT_MBR*)buf;
  if (mbr->signature != 0xaa55) {
    println("Invalid disk signature %04x", mbr->signature);
    return -1;
  }
  println("Valid disk signature");

  // dump partition table
//  hexdump((uint8_t*)mbr->partitionTable, sizeof(FAT_PartitionTableEntry)*4);

  // TODO Extend to more than one disk
  mountedDisks[0].diskID = 0;

  // 4 partition table entries
  for (int i = 0; i < 4; i++) {
    if (mbr->partitionTable[i].type == 0 ) {
      println("Found empty partition");
    } else {
      println("Partition %d type is: %02x", i, mbr->partitionTable[i].type);
      if (mbr->partitionTable[i].type == PAR_TYPE_FAT32) {
        println("FAT32 partition found");
      }
      println("Partition %d start sector is: %u", i, (unsigned int)mbr->partitionTable[i].partitionLBA);
      println("Partition %d size is: %u", i, (unsigned int)mbr->partitionTable[i].size);

      mountedDisks[0].partitionInfo[i].partitionNumber = i;
      mountedDisks[0].partitionInfo[i].type = mbr->partitionTable[i].type;
      mountedDisks[0].partitionInfo[i].startAddress = mbr->partitionTable[i].partitionLBA;
      mountedDisks[0].partitionInfo[i].length = mbr->partitionTable[i].size;
    }
  }

  // Read boot sector of first partition
  phyCallbacks.phyReadSectors(buf, mountedDisks[0].partitionInfo[0].startAddress, 1);
//  hexdump(buf, 512);

  FAT32_BootSector* bootSector = (FAT32_BootSector*)buf;

  if (bootSector->signature != 0xaa55) {
    println("Invalid partition signature %04x", mbr->signature);
    return -2;
  }

  println("Valid partition signature");

  // We already have length from partition table
  println("Partition size is %d", (unsigned int)bootSector->totalSectors32);

  if (bootSector->totalSectors32 != mountedDisks[0].partitionInfo[0].length) {
    println("Error: Wrong partition size");
    while(1);
  }
  // reserved sectors are the sectors before the FAT including boot sector
  println("Reserved sectors = %d", (unsigned int)bootSector->reservedSectors);

  println("Bytes per sector %d", (unsigned int)bootSector->bytesPerSector);

  if (bootSector->bytesPerSector != 512) {
    // TODO Make library sector length independent
    println("Error: incompatible sector length");
    while(1);
  }
  // hidden sectors are the sectors on disk preceding partition
  println("Hidden sectors %d", (unsigned int)bootSector->hiddenSectors);
  println("Sectors per cluster =  %d", (unsigned int)bootSector->sectorsPerCluster);
  println("Number of FATs =  %d", (unsigned int)bootSector->numberOfFATs);
  println("Sectors per FAT =  %d", (unsigned int)bootSector->sectorsPerFAT32);

  // The cluster where the root directory is at
  println("Root cluster = %d", (unsigned int)bootSector->rootCluster);
  println("FSInfo structure is at sector %d", (unsigned int)bootSector->fsInfo);
  println("Backup boot sector is at sector %d", (unsigned int)bootSector->backupBootSector);


  // Sector on disk where FAT is (from start of disk)
  uint32_t fatStart = mountedDisks[0].partitionInfo[0].startAddress +
      bootSector->reservedSectors;

  mountedDisks[0].partitionInfo[0].startFatSector = fatStart;


  // Sector on disk where data clusters start
  // Cluster count start from 2
  // So this sector is where cluster 2 is allocated on disk
  uint32_t clusterStart = fatStart + bootSector->numberOfFATs *
      bootSector->sectorsPerFAT32;

  mountedDisks[0].partitionInfo[0].dataStartSector = clusterStart;

  uint32_t sectorsPerCluster = bootSector->sectorsPerCluster;

  // needed for mapping clusters to sectors
  mountedDisks[0].partitionInfo[0].sectorsPerCluster = sectorsPerCluster;

  mountedDisks[0].partitionInfo[0].bytesPerSector = bootSector->bytesPerSector;

  uint32_t rootCluster = bootSector->rootCluster;

  mountedDisks[0].partitionInfo[0].rootDirSector = FAT_Cluster2Sector(rootCluster);
  mountedDisks[0].partitionInfo[0].rootDirCluster = bootSector->rootCluster;

  println("FATs start at sector %d", (unsigned int)fatStart);

//  phyCallbacks.phyReadSectors(buf, fatStart, 1);
//  hexdump(buf, 512);

  FAT_ListRootDir();


  FAT_File file;

  strcpy(file.filename, "HELLO   TXT");
  FAT_FindFile(&file);
  strcpy(file.filename, "HAMLET  TXT");
  FAT_FindFile(&file);
  strcpy(file.filename, "LOL     TXT");
  FAT_FindFile(&file);

  return 0;
}

/**
 * @brief Converts cluster number to sector number from start of drive
 *
 * @details Two first clusters are reserved (-2 term in the equation).
 *
 * @param cluster Cluster number
 *
 * @return Sector number counting from the start of the drive.
 */
uint32_t FAT_Cluster2Sector(uint32_t cluster) {

  uint32_t sector = mountedDisks[0].partitionInfo[0].dataStartSector
      + (cluster - 2) * mountedDisks[0].partitionInfo[0].sectorsPerCluster;

  return sector;
}

/**
 * @brief Gets FAT entry for given cluster
 * @param cluster Cluster number
 * @return FAT entry for given cluster
 */
uint32_t FAT_GetEntryInFAT(uint32_t cluster) {

  // Calculate the sector where the FAT entry for the cluster is located at.
  // Every entry is 4 bytes long. We divide the byte number where the entry
  // starts (cluster*4) by the number of bytes per sector, which gives
  // the sector number of the entry
  uint32_t sector = mountedDisks[0].partitionInfo[0].startFatSector +
      cluster*4/mountedDisks[0].partitionInfo[0].bytesPerSector;

  println("FAT entry is at sector %d", (unsigned int)sector);

  uint8_t buf[512]; // buffer for sector data

  phyCallbacks.phyReadSectors(buf, sector, 1);

  // the byte number of the entry in the given sector is the remainder
  // of the previous calculation
  uint8_t offset = (cluster*4) % mountedDisks[0].partitionInfo[0].bytesPerSector;

  // the 4-byte entry is at offset
  uint32_t* ret = (uint32_t*)(buf+offset);

  println("Fat entry is %08x", (unsigned int)*ret);

  return *ret;
}

/**
 * @brief Finds a given file in a directory.
 * @param file Name of the file
 * @return
 * TODO Search for files also in subdirectories of the root directory.
 */
uint8_t FAT_FindFile(FAT_File* file) {

  uint8_t buf[512]; // buffer for sectors

  uint32_t i = 0, j = 0, k = 0;

  FAT_RootDirEntry* dirEntry = 0; // the directory entry
  // current cluster of root dir
  uint32_t currentCluster = mountedDisks[0].partitionInfo[0].rootDirCluster;
  // current sector of root dir
  uint32_t currentSector;

  char* ptr; // for copying filename
  char filename[12];

  // do until file is found or we reach last entry in the roor directory
  while(1) {

    // there are 16 entries per sector
    // Read new sector every 16 entries
    if ((i&0x0000000f) == 0) {

      println("Find file: read new sector");
      // if whole cluster read - find next cluster
//      if ((i%mountedDisks[0].partitionInfo[0].sectorsPerCluster) == 0) {
//        if (i != 0) { // for i == 0 we take first cluster of root dir from boot sector
//          // for other values search the FAT for next cluster
//
//          // new cluster number is in the entry for the current cluster
//          currentCluster = FAT_GetEntryInFAT(currentCluster);
//          // if last cluster then stop
//          if (currentCluster == FAT_LAST_CLUSTER) {
//            println("Last cluster reached. File not found");
//            return 1;
//          }
//        }
//        j = 0; // zero out sector counter at every new cluster
//      }

      // currently read sector is based on the current cluster
      // and the counter j, which updates every 16 entries
      currentSector = FAT_Cluster2Sector(currentCluster) + j;
      // read new sector every 16 entries
      phyCallbacks.phyReadSectors(buf, currentSector, 1);

      TIMER_Delay(1000);

      println("Read sector %u", (unsigned int) currentSector);

      // first entry in buffer for new sector
      dirEntry = (FAT_RootDirEntry*)buf;
      // go to next sector
      j++;
    }

    println("Comparing file %u", (unsigned int)i);
    i++; // update counter to next entry.

    if (dirEntry->filename[0] == 0x00) {
      // last root dir entry
      println("Last entry reached. File not found");
      return 1;
    }

    if (dirEntry->filename[0] == 0xe5) {
      println("Empty file");
      dirEntry++;
      continue;
    }
    if (dirEntry->attributes == 0x0f) {
      println("Long file");
      dirEntry++;
      continue;
    }

    ptr = (char*)dirEntry->filename;
    // copy filename from directory entry
    for (k=0; k<11; k++) {
      filename[k] = *ptr++;
    }
    filename[11] = 0; // end string
    if (!strcmp(filename, file->filename)) {
      println("Found file %s!!!!", file->filename);
      // get all the relevant information about the file

      file->firstCluster = (((uint32_t)(dirEntry->firstClusterH))<<16) |
          (uint32_t)dirEntry->firstClusterL;
      file->fileSize = dirEntry->fileSize;
      file->attributes = dirEntry->attributes;
      file->lastModifiedTime = dirEntry->lastModifiedTime;
      file->lastModifiedDate = dirEntry->lastModifiedDate;

      return 0;
    }
    dirEntry++;
  }

  return 1;
}


/**
 * @brief Lists files in root directory of volume
 * TODO Finish this function
 */
void FAT_ListRootDir(void) {

  println("Root dir");

  uint8_t buf[512];

  // read first sector of root dir
  phyCallbacks.phyReadSectors(buf,
      mountedDisks[0].partitionInfo[0].rootDirSector, 1);

//  hexdump(buf, 512);

  FAT_RootDirEntry* dirEntry = (FAT_RootDirEntry*)buf;

  char filename[12];
  char* ptr;
  int i, j;

  // 16 entries in one sector
  for (i = 0; i< 16; i++) {

    println("FILE %d:", i);

    ptr = (char*)dirEntry->filename;

    // check if file is empty
    if (dirEntry->filename[0] == 0x00 || dirEntry->filename[0] == 0xe5) {
      println("Empty file");
      dirEntry++;
      continue;
    }
    if (dirEntry->attributes == 0x0f) {
      println("Long file");
      dirEntry++;
      continue;
    }

    for (j=0; j<11; j++) {
      filename[j] = *ptr++;
    }
    filename[11] = 0;

    println("Filename %s, attributes 0x%02x, file size %d",
        filename, (unsigned int)dirEntry->attributes,
        (unsigned int)dirEntry->fileSize);

    if (!strcmp(filename, "HELLO   TXT")) {
      println("Found file %s!!!!", filename);
      uint8_t buf2[512];
      uint32_t cluster = (((uint32_t)(dirEntry->firstClusterH))<<16) |
          (uint32_t)dirEntry->firstClusterL;
      println("File is at cluster %d", (unsigned int)cluster);
      println("File is at sector %d", (unsigned int)FAT_Cluster2Sector(cluster));
      println("File size is %d",(unsigned int)dirEntry->fileSize);

      phyCallbacks.phyReadSectors(buf2, FAT_Cluster2Sector(cluster), 1);

      hexdump(buf2, 512);
      println("%s",buf2);

      FAT_GetEntryInFAT(cluster);
    }

    dirEntry++;
  }

}
