/**
 * @file    fat.c
 * @brief   FAT file system implementation.
 * @date    4 maj 2014
 * @author  Michal Ksiezopolski
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
 * @addtogroup FAT
 * @{
 */

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
/**
 * @brief Partition type
 */
typedef enum {
  PAR_TYPE_EMPTY = 0x00,    //!< PAR_TYPE_EMPTY
  PAR_TYPE_FAT12 = 0x01,    //!< PAR_TYPE_FAT12
  PAR_TYPE_FAT16_32M = 0x04,//!< PAR_TYPE_FAT16_32M
  PAR_TYPE_EXTENDED = 0x05, //!< PAR_TYPE_EXTENDED
  PAR_TYPE_FAT16 = 0x06,    //!< PAR_TYPE_FAT16
  PAR_TYPE_NTFS = 0x07,     //!< PAR_TYPE_NTFS
  PAR_TYPE_FAT32 = 0x0b,    //!< PAR_TYPE_FAT32

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
 * @brief Root directory entry (32 bytes long)
 */
typedef struct {
  uint8_t filename[8];        ///< Name of file
  uint8_t extension[3];       ///< Extension of file
  uint8_t attributes;         ///< Attributes. 0x01 - read only, 0x02 - hidden, 0x04 - system, 0x08 - volume ID, 0x10 - directory, 0x20 - archive
  uint8_t unused;             ///< Unused
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
 * @brief Long directory entry
 *
 * @details Long directory entries always immediately precede a
 * short directory entry. 1st long directory entry is the first one
 * preceding the short directory entry. Long directory characters
 * are 16-bit.
 * After the last character of the name there is a 0x0000 character. The
 * rest of the characters are padded with 0xffff.
 */
typedef struct {
  uint8_t order;          ///< Order of entry in sequence of long dir entries. (0x40 mask means last long dir entry)
  uint16_t name1[5];      ///< Part 1 of name
  uint8_t attributes;     ///< Attributes. 0x0f for long file
  uint8_t type;           ///<
  uint8_t checksum;       ///< Checksum of name in short dir entry.
  uint16_t name2[6];      ///< Part 2 of name
  uint16_t firstClusterL; ///< Always 0
  uint16_t name3[2];      ///< Part 3 of name
}__attribute((packed)) FAT_LongDirEntry;
/**
 * @brief Structure for getting date information of a file.
 */
typedef union {

  uint16_t date;
  struct {
    uint16_t day: 5;
    uint16_t month: 4;
    uint16_t year: 7;
  } fields;
} FAT_DateFormat;
/**
 * @brief Structure for getting date information of a file.
 */
typedef union {

  uint16_t time;
  struct {
    uint16_t seconds: 5;
    uint16_t minutes: 6;
    uint16_t hours: 5;
  } fields;
} FAT_TimeFormat;
/**
 * @brief Structure for keeping file information
 */
typedef struct {
  char filename[12];          ///< Zero ended file name and extension
  uint32_t firstCluster;      ///< First cluster of file
  uint32_t fileSize;          ///< Size of file
  uint8_t attributes;         ///< Attributes of file
  uint16_t lastModifiedTime;  ///< Last modified time of file
  uint16_t lastModifiedDate;  ///< Last modified date of file
  uint32_t rootDirEntry;      ///< Number of root dir entry for file
  int id;                     ///< File ID
  uint32_t wrPtr;             ///< Pointer to current write location
  uint32_t rdPtr;             ///< Pointer to current read location

} FAT_File;
/**
 * @brief Structure containing info about partition structure
 *
 * @details This is used by the application to store the relevant data
 * read from the partition table and the bootsector of the partition.
 *
 */
typedef struct {
  uint8_t partitionNumber;    ///< Number of the partition on disk (as in MBR)
  uint8_t type;               ///< Type of the partition - file system type
  uint32_t startAddress;      ///< Start address - LBA sector number
  uint32_t length;            ///< Length of partition in sectors
  uint32_t startFatSector;    ///< Sector where FAT start
  uint32_t rootDirSector;     ///< Sector where root directory starts
  uint32_t rootDirCluster;    ///< First cluster of root directory
  uint32_t dataStartSector;   ///< Sector where data starts
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
/**
 * @brief Physical layer callbacks.
 */
typedef struct {
  void (*phyInit)(void);
  uint8_t (*phyReadSectors)(uint8_t* buf, uint32_t sector, uint32_t count);
  uint8_t (*phyWriteSectors)(uint8_t* buf, uint32_t sector, uint32_t count);
} FAT_PhysicalCb;

#define FAT_MAX_DISKS     2   ///< Maximum number of mounted disks
#define MAX_OPENED_FILES  32  ///< Maximum number of opened files
#define FAT_LAST_CLUSTER  0x0fffffff ///< Last cluster in file

/**
 * @brief Opened files
 *
 * @details If a file ID is -1 then the file is not present.
 * To delete a file, just write -1 to its ID field.
 */
static FAT_File openedFiles[MAX_OPENED_FILES];
static FAT_DiskInfo mountedDisks[FAT_MAX_DISKS]; ///< Disk info for mounted disks
static uint8_t buf[512]; ///< Buffer for reading sectors
static FAT_PhysicalCb phyCallbacks; ///< Physical layer callbacks

static uint32_t FAT_Cluster2Sector(uint32_t cluster);
//static void FAT_ListRootDir(void);
static uint32_t FAT_GetEntryInFAT(uint32_t cluster);
static int FAT_FindFile(FAT_File* file);
static int FAT_GetNextId(void);
static int FAT_GetCluster(uint32_t firstCluster, uint32_t clusterOffset,
    uint32_t* clusterNumber);
static void FAT_UpdateRootEntry(int file);

/**
 * @brief Convenience function for reading sectors.
 *
 * @details It checks if the sector isn't in the buffer first
 * as a simple caching mechanism.
 *
 * @param sector Sector to read.
 */
static void FAT_ReadSector(uint32_t sector) {

  static uint32_t sectInBuffer = UINT32_MAX;

  // check if we already read the sector
  if (sectInBuffer == sector) {
    println("ReadSector: Sector already read");
    return;
  }

  sectInBuffer = sector;
  phyCallbacks.phyReadSectors(buf, sector, 1);
  println("ReadSector: Read sector %u", (unsigned int) sector);

}
/**
 * @brief Convenience function for reading sectors.
 *
 * @details It checks if the sector isn't in the buffer first
 * as a simple caching mechanism.
 *
 * @param sector Sector to read.
 */
static void FAT_WriteSector(uint32_t sector) {

  phyCallbacks.phyWriteSectors(buf, sector, 1);
  println("WriteSector: Written sector %u", (unsigned int) sector);

}
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

  // Read MBR - first sector (0)
  FAT_ReadSector(0);

  FAT_MBR* mbr = (FAT_MBR*)buf;
  if (mbr->signature != 0xaa55) {
    println("Invalid disk signature %04x", mbr->signature);
    return -1;
  }
  println("Found valid disk signature");

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
      println("Partition %d size is: %u", i, (unsigned int)mbr->partitionTable[i].size*512);

      mountedDisks[0].partitionInfo[i].partitionNumber = i;
      mountedDisks[0].partitionInfo[i].type = mbr->partitionTable[i].type;
      mountedDisks[0].partitionInfo[i].startAddress = mbr->partitionTable[i].partitionLBA;
      mountedDisks[0].partitionInfo[i].length = mbr->partitionTable[i].size;
    }
  }

  // Read boot sector of first partition
  FAT_ReadSector(mountedDisks[0].partitionInfo[0].startAddress);

  FAT32_BootSector* bootSector = (FAT32_BootSector*)buf;

  if (bootSector->signature != 0xaa55) {
    println("Invalid partition signature %04x", mbr->signature);
    return -2;
  }

  println("Found valid partition signature");

  // We already have length from partition table, so just display
//  println("Partition size is %d", (unsigned int)bootSector->totalSectors32);

  if (bootSector->totalSectors32 != mountedDisks[0].partitionInfo[0].length) {
    println("Error: Wrong partition size");
    while(1);
  }
  // reserved sectors are the sectors before the FAT including boot sector
//  println("Reserved sectors = %d", (unsigned int)bootSector->reservedSectors);

//  println("Bytes per sector %d", (unsigned int)bootSector->bytesPerSector);

  if (bootSector->bytesPerSector != 512) {
    // TODO Make library sector length independent
    println("Error: incompatible sector length");
    while(1);
  }
  // hidden sectors are the sectors on disk preceding partition
//  println("Hidden sectors %d", (unsigned int)bootSector->hiddenSectors);
  println("Sectors per cluster =  %d", (unsigned int)bootSector->sectorsPerCluster);
  println("Number of FATs =  %d", (unsigned int)bootSector->numberOfFATs);
  println("Sectors per FAT =  %d", (unsigned int)bootSector->sectorsPerFAT32);

  // The cluster where the root directory is at
  println("Root cluster = %d", (unsigned int)bootSector->rootCluster);
//  println("FSInfo structure is at sector %d", (unsigned int)bootSector->fsInfo);
//  println("Backup boot sector is at sector %d", (unsigned int)bootSector->backupBootSector);


  // Sector on disk where FAT is (from start of disk)
  uint32_t fatStart = mountedDisks[0].partitionInfo[0].startAddress +
      bootSector->reservedSectors;

  mountedDisks[0].partitionInfo[0].startFatSector = fatStart;
  println("FATs start at sector %d", (unsigned int)fatStart);

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

//  FAT_ListRootDir();

  // Set all IDs to free slot
  for (int i = 0; i < MAX_OPENED_FILES; i++) {
    openedFiles[i].id = -1;
  }

  return 0;
}
/**
 * @brief Opens a file.
 * @param filename Name of file
 * @return ID of file
 *
 * TODO Add parsing paths.
 * TODO Add long filenames
 */
int FAT_OpenFile(const char* filename) {

  FAT_File file;
  strcpy(file.filename, filename);
  println("%s: Opening file %s", __FUNCTION__, filename);

  int id = FAT_FindFile(&file);

  if (id != -1) {
    // copy file information structure
    openedFiles[id] = file;
  }

  return id;
}
/**
 * @brief Create new file
 * @param filename name of new file
 * @return File ID or -1 if file exists
 * TODO Finish this function
 */
int FAT_NewFile(const char* filename) {
  FAT_File file;
  strcpy(file.filename, filename);
  println("%s: Opening file %s", __FUNCTION__, filename);

  int id = FAT_FindFile(&file);

  // if file found
  if (id != -1) {
    // file already exists
    return -1;
  }
  return 0;
}
/**
 * @brief Close a file.
 * @param file ID of file
 * @return ID of closed file (won't be useful anymore) or -1 if error.
 */
int FAT_CloseFile(int file) {

  // if incorrect file ID
  if (file >= MAX_OPENED_FILES) {
    return -1;
  }
  // File not opened
  if (openedFiles[file].id == -1) {
    return -1; // EOF for not open file
  }
  // close file if no errors
  openedFiles[file].id = -1;
  return file;
}
/**
 * @brief Move the read pointer to new location in file
 * @param file File ID
 * @param newWrPtr New read pointer location
 * @return New read pointer location or -1 if error ocurred.
 */
int FAT_MoveRdPtr(int file, int newWrPtr) {

  // if incorrect file ID
  if (file >= MAX_OPENED_FILES) {
    return -1;
  }

  // File not opened
  if (openedFiles[file].id == -1) {
    return -1; // EOF for not open file
  }

  // Can't move beyond length of file for read
  if (newWrPtr > openedFiles[file].fileSize) {
    println("%s: EOF reached", __FUNCTION__);
    return -1;
  }

  // if no errors - move the read pointer
  openedFiles[file].rdPtr = newWrPtr;
  return newWrPtr;
}
/**
 * @brief Move the write pointer to new location in file.
 * @param file File ID
 * @param newWrPtr New write pointer location
 * @return New write pointer location or -1 if error ocurred.
 * FIXME Finish function
 */
int FAT_MoveWrPtr(int file, int newWrPtr) {
  // if incorrect file ID
  if (file >= MAX_OPENED_FILES) {
    return -1;
  }
  // File not opened
  if (openedFiles[file].id == -1) {
    return -1; // EOF for not open file
  }
  // TODO If new ptr value is larger than file size - zero pad
  // Can't move beyond length of file for read
//  if (newWrPtr > openedFiles[file].fileSize) {
//    println("EOF reached");
//    return -1;
//  }

  openedFiles[file].wrPtr = newWrPtr;
  return newWrPtr;
}
/**
 * @brief Reads contents of file.
 * @param id ID of opened file
 * @param data Buffer for storing data
 * @param count Number of bytes to read
 * @return Number of bytes read or -1 for EOF
 * TODO Also read next clusters
 */
int FAT_ReadFile(int file, uint8_t* data, int count) {

  println("%s", __FUNCTION__);

  // if incorrect file ID
  if (file >= MAX_OPENED_FILES) {
    println("Maximum number of files open");
    return -1;
  }

  // File not opened
  if (openedFiles[file].id == -1) {
    println("File not open");
    return -1; // EOF for not open file
  }
  // We have already reached EOF
  if (openedFiles[file].rdPtr >= openedFiles[file].fileSize) {
    println("EOF reached");
    return -1;
  }

  int len = 0; // number of bytes read

  // jump to sector where read pointer is at (counting from first sector)
  // each sector is 512 bytes long
  uint32_t sectorOffset = openedFiles[file].rdPtr / 512;

  // which cluster from start cluster is the sector at
  uint32_t clusterOffset = sectorOffset/
      mountedDisks[0].partitionInfo[0].sectorsPerCluster;
  // sector to read in the cluster
  sectorOffset = sectorOffset %
      mountedDisks[0].partitionInfo[0].sectorsPerCluster;

  // find the cluster number where the data is at
  uint32_t baseCluster = 0;
  FAT_GetCluster(openedFiles[file].firstCluster, clusterOffset,
      &baseCluster);
  uint32_t baseSector = FAT_Cluster2Sector(baseCluster);

  // add number of sectors in the cluster where data is at
  baseSector += sectorOffset;

  // read data sector
  FAT_ReadSector(baseSector);

  // start getting data from read pointer (in the current sector)
  uint8_t* ptr = buf + openedFiles[file].rdPtr % 512;

  println("%s: reading data", __FUNCTION__);
  for (int i = 0; i < count; i++) {

    data[i] = *ptr++;
    openedFiles[file].rdPtr++;
    len++;
    // check if EOF reached
    if (openedFiles[file].rdPtr >= openedFiles[file].fileSize) {
      println("%s: EOF reached", __FUNCTION__);
      break;
    }
    // if sector boundary reached
    if (openedFiles[file].rdPtr % 512 == 0) {
      println("%s: read new sector", __FUNCTION__);
      // increment sector counter
      sectorOffset++;
      // which sector in cluster is it
      sectorOffset = sectorOffset %
          mountedDisks[0].partitionInfo[0].sectorsPerCluster;

      // if first sector, then read new cluster
      if (sectorOffset == 0) {
        println("%s: jump to next cluster", __FUNCTION__);
        // change cluster to next
        FAT_GetCluster(baseCluster, 1, &baseCluster);
      }
      baseSector = FAT_Cluster2Sector(baseCluster) + sectorOffset;
      FAT_ReadSector(baseSector);
      ptr = buf;
    }
  }

  return len;
}
/**
 * @brief Writes data to a file
 * @param file ID of file, to which we write data.
 * @param data Data to write
 * @param count Number of bytes to write
 * @return Number of bytes written.
 * TODO Make this cross sector and cluster boundaries
 * FIXME For now we can write only up to EOF
 */
int FAT_WriteFile(int file, const uint8_t* data, int count) {

  println("%s", __FUNCTION__);

  // if incorrect file ID
  if (file >= MAX_OPENED_FILES) {
    println("Maximum number of files open");
    return -1;
  }

  // File not opened
  if (openedFiles[file].id == -1) {
    println("File not open");
    return -1; // EOF for not open file
  }
  // We have already reached EOF
  // TODO Make this cross EOF - adding more data - change file size in root dir
  if (openedFiles[file].wrPtr >= openedFiles[file].fileSize) {
    println("%s: EOF reached", __FUNCTION__);

    // TODO Zero out the bytes between wrPtr and filesize
    // TODO If new cluster we need to add cluster info in FAT
  }

  int len = 0; // number of bytes written

  // jump to sector where write pointer is at (counting from first sector)
  // each sector is 512 bytes long
  uint32_t sectorOffset = openedFiles[file].wrPtr / 512;

  // which cluster from start cluster is the sector at
  uint32_t clusterOffset = sectorOffset/
      mountedDisks[0].partitionInfo[0].sectorsPerCluster;
  // sector to write in the cluster
  sectorOffset = sectorOffset %
      mountedDisks[0].partitionInfo[0].sectorsPerCluster;

  // find the cluster number where the data is at
  uint32_t baseCluster = 0;
  FAT_GetCluster(openedFiles[file].firstCluster, clusterOffset,
      &baseCluster);
  uint32_t baseSector = FAT_Cluster2Sector(baseCluster);

  // add number of sectors in the cluster where data is at
  baseSector += sectorOffset;

  // read data sector
  FAT_ReadSector(baseSector);

  // start writing data from write pointer (in the current sector)
  uint8_t* ptr = buf + openedFiles[file].wrPtr % 512;

  println("%s: writing data", __FUNCTION__);
  for (int i = 0; i < count; i++) {

    *ptr++ = data[i];
    openedFiles[file].wrPtr++;
    len++;

    // if sector boundary reached
    if (openedFiles[file].wrPtr % 512 == 0) {
      println("%s: new sector", __FUNCTION__);
      FAT_WriteSector(baseSector); // save data
//      FAT_UpdateRootEntry(file);
      // increment sector counter
      sectorOffset++;
      // which sector in cluster is it
      sectorOffset = sectorOffset %
          mountedDisks[0].partitionInfo[0].sectorsPerCluster;

      // if first sector, then read new cluster
      if (sectorOffset == 0) {
        println("%s: jump to next cluster", __FUNCTION__);

        if (openedFiles[file].wrPtr >= openedFiles[file].fileSize) {
          // TODO If new cluster then update FAT
          //        FAT_UpdateCluster();
        }

        // change cluster to next
        FAT_GetCluster(baseCluster, 1, &baseCluster);

      }
      baseSector = FAT_Cluster2Sector(baseCluster) + sectorOffset;
      FAT_ReadSector(baseSector);
      ptr = buf;
    }

    // check if EOF reached and update file size
    if (openedFiles[file].wrPtr >= openedFiles[file].fileSize) {
      // if writing to end of file - increment filesize
      openedFiles[file].fileSize = openedFiles[file].wrPtr;
    }
  }

  FAT_WriteSector(baseSector); // save data
  FAT_UpdateRootEntry(file);
  return len;

}
/**
 * @brief Updates the root directory entry of a given file.
 *
 * @details This function is called after a write to the file
 * in order to update the timestamp and the file length if
 * necessary.
 *
 * @param file File ID
 */
static void FAT_UpdateRootEntry(int file) {

  // cluster of root dir
  uint32_t currentCluster = mountedDisks[0].partitionInfo[0].rootDirCluster;

  // sector where root dir is at
  uint32_t sector = FAT_Cluster2Sector(currentCluster);

  // every root dir entry is 32 bytes
  // add sector offset of entry
  sector += openedFiles[file].rootDirEntry * sizeof(FAT_RootDirEntry) / 512;

  // read sector where entry is at

  FAT_ReadSector(sector);
  println("%s: Read sector %u", __FUNCTION__, (unsigned int)sector);

  // point to entry in the current sector
//  uint8_t* ptr = buf;
//  ptr += (openedFiles[file].rootDirEntry * sizeof(FAT_RootDirEntry)) % 512;

  FAT_RootDirEntry* dirEntry = (FAT_RootDirEntry*) buf;
  println("%s: Dir entry %u", __FUNCTION__,
      (unsigned int)openedFiles[file].rootDirEntry);
  dirEntry += openedFiles[file].rootDirEntry;

  char filename[12];
  char* namePtr = (char*)dirEntry->filename;
  // copy filename from directory entry
  for (int k = 0; k < 11; k++) {
    filename[k] = *namePtr++;
  }
  filename[11] = 0; // end string
  dirEntry->fileSize = openedFiles[file].fileSize;

  println("%s: Updating root entry for file: %s, size %u", __FUNCTION__,
      filename, (unsigned int)openedFiles[file].fileSize);

  FAT_WriteSector(sector);
}
/**
 * @brief Gets number of cluster clusterOffset in a file
 *
 * @details
 *
 * @param firstCluster First cluster of file
 * @param clusterOffset Cluster from start of file we want to find
 * @param clusterNumber The number of the searched cluster (function writes this)
 * @return Cluster from start of file we really found
 */
static int FAT_GetCluster(uint32_t firstCluster, uint32_t clusterOffset,
    uint32_t* clusterNumber) {

  uint32_t entry = firstCluster;
  int i;

  for (i = 0; i < clusterOffset; i++) {
    entry = FAT_GetEntryInFAT(entry);
    // last cluster reached before we reached clusterOffset
    if (entry == 0xFFFFFFFF) {
      *clusterNumber = entry; // return the entry
      return i;
    }
  }

  *clusterNumber = entry; // return the entry
  return clusterOffset;
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
static uint32_t FAT_Cluster2Sector(uint32_t cluster) {

  uint32_t sector = mountedDisks[0].partitionInfo[0].dataStartSector
      + (cluster - 2) * mountedDisks[0].partitionInfo[0].sectorsPerCluster;

  return sector;
}
/**
 * @brief Gets FAT entry for given cluster
 * @param cluster Cluster number
 * @return FAT entry for given cluster
 */
static uint32_t FAT_GetEntryInFAT(uint32_t cluster) {

  // Calculate the sector where the FAT entry for the cluster is located at.
  // Every entry is 4 bytes long. We divide the byte number where the entry
  // starts (cluster*4) by the number of bytes per sector, which gives
  // the sector number of the entry
  uint32_t sector = mountedDisks[0].partitionInfo[0].startFatSector +
      cluster*4/mountedDisks[0].partitionInfo[0].bytesPerSector;

  println("%s: FAT entry is at sector %d", __FUNCTION__, (unsigned int)sector);

//  uint8_t buf[512]; // buffer for sector data

  // read sector where FAT entry is at
//  phyCallbacks.phyReadSectors(buf, sector, 1);
  FAT_ReadSector(sector);

  // the byte number of the entry in the given sector is the remainder
  // of the previous calculation
  uint8_t offset = (cluster*4) % mountedDisks[0].partitionInfo[0].bytesPerSector;

  // the 4-byte entry is at offset
  uint32_t* ret = (uint32_t*)(buf+offset);

  println("%s: Fat entry is %08x", __FUNCTION__, (unsigned int)*ret);

  return *ret;
}
/**
 * @brief Finds a given file in a directory.
 * @param file Name of the file
 * @return ID of file or -1 if not found.
 * TODO Search for files also in subdirectories of the root directory.
 */
static int FAT_FindFile(FAT_File* file) {

  println("%s: Searching for file %s", __FUNCTION__, file->filename);

  uint32_t i = 0, j = 0, k = 0;

  FAT_RootDirEntry* dirEntry = 0; // the directory entry
  // current cluster of root dir
  uint32_t currentCluster = mountedDisks[0].partitionInfo[0].rootDirCluster;
  // current sector of root dir
  uint32_t currentSector;

  char* ptr; // for copying filename
  char filename[12];

  // do until file is found or we reach last entry in the root directory
  while(1) {

    // there are 16 entries per sector
    // Read new sector every 16 entries
    if ((i%16) == 0) {
      println("%s: read new sector", __FUNCTION__);
      // TODO Also change cluster
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
      FAT_ReadSector(currentSector);

      // FIXME This may be needed for terminal
//      TIMER_Delay(1000);

      // first entry in buffer for new sector
      dirEntry = (FAT_RootDirEntry*)buf;
      // go to next sector
      j++;
    }

//    println("Comparing file %u", (unsigned int)i);
    i++; // update counter to next entry.

    if (dirEntry->filename[0] == 0x00) {
      // last root dir entry
      println("%s: Last entry reached. File not found", __FUNCTION__);
      return -1;
    }

    if (dirEntry->filename[0] == 0xe5) {
//      println("Empty file");
      dirEntry++;
      continue;
    }
    if (dirEntry->attributes == 0x0f) {
//      println("Long file");
      dirEntry++;
      continue;
    }

    ptr = (char*)dirEntry->filename;
    // copy filename from directory entry
    for (k=0; k<11; k++) {
      filename[k] = *ptr++;
    }
    filename[11] = 0; // end string
//    println("File name %s", filename);
    if (!strcmp(filename, file->filename)) {

      // get all the relevant information about the file
      file->firstCluster = (((uint32_t)(dirEntry->firstClusterH))<<16) |
          (uint32_t)dirEntry->firstClusterL;
      file->fileSize = dirEntry->fileSize;
      file->attributes = dirEntry->attributes;
      file->lastModifiedTime = dirEntry->lastModifiedTime;
      file->lastModifiedDate = dirEntry->lastModifiedDate;
      file->id = FAT_GetNextId();
      file->rootDirEntry = i-1;
      println("%s, File root dir entry = %u", __FUNCTION__,
          (unsigned int)file->rootDirEntry);

      FAT_DateFormat date;
      date.date = file->lastModifiedDate;

      FAT_TimeFormat time;
      time.time = file->lastModifiedTime;

      file->rdPtr = 0; // start reading from 1st byte
      file->wrPtr = 0; // start writing from 1st byte

      println("%s: Found file %s of size %u, ID = %u!!!",
          __FUNCTION__, file->filename, (unsigned int)file->fileSize,
          (unsigned int)file->id);
      println("%s: File created on %02u.%02u.%04u at %02u:%02u:%02u",
          __FUNCTION__, date.fields.day,date.fields.month, date.fields.year+1980,
          time.fields.hours, time.fields.minutes, time.fields.seconds*2);

      FAT_LongDirEntry *longEntry = (FAT_LongDirEntry *)(dirEntry - 1);

      uint16_t longFilename[14];

      uint16_t* namePtr = longFilename;

      for (int i = 0; i < 5; i++) {
        *namePtr++ = longEntry->name1[i];
      }
      for (int i = 0; i < 6; i++) {
        *namePtr++ = longEntry->name2[i];
      }
      for (int i = 0; i < 2; i++) {
        *namePtr++ = longEntry->name3[i];
      }
      *namePtr = 0;
      println("%s: Long file name", __FUNCTION__);
      hexdump16C(longFilename, 14);

      return file->id;
    }
    dirEntry++;
  }

  return -1;
}
/**
 * @brief Finds next free ID of file.
 * @return File ID or -1 if no free left.
 */
static int FAT_GetNextId(void) {

  for (int i = 0; i < MAX_OPENED_FILES; i++) {
    // search for free file entry
    if (openedFiles[i].id == -1) {
      return i;
    }
  }

  // if no free
  return -1;
}
/**
 * @brief Lists files in root directory of volume
 * TODO Finish this function. Used for tests for now
 */
//static void FAT_ListRootDir(void) {
//
//  println("Root dir");
//
//  // read first sector of root dir
//  phyCallbacks.phyReadSectors(buf,
//      mountedDisks[0].partitionInfo[0].rootDirSector, 1);
//
//  FAT_RootDirEntry* dirEntry = (FAT_RootDirEntry*)buf;
//
//  char filename[12];
//  char* ptr;
//  int i, j;
//
//  // 16 entries in one sector
//  for (i = 0; i< 16; i++) {
//
//    println("FILE %d:", i);
//
//    ptr = (char*)dirEntry->filename;
//
//    // check if file is empty
//    if (dirEntry->filename[0] == 0x00 || dirEntry->filename[0] == 0xe5) {
//      dirEntry++;
//      continue;
//    }
//    if (dirEntry->attributes == 0x0f) {
//      println("Long file");
//      dirEntry++;
//      continue;
//    }
//
//    for (j=0; j<11; j++) {
//      filename[j] = *ptr++;
//    }
//    filename[11] = 0;
//
//    println("Filename %s, attributes 0x%02x, file size %d",
//        filename, (unsigned int)dirEntry->attributes,
//        (unsigned int)dirEntry->fileSize);
//
//    if (!strcmp(filename, "HELLO   TXT")) {
//      println("Found file %s!!!!", filename);
//      uint8_t buf2[512];
//      uint32_t cluster = (((uint32_t)(dirEntry->firstClusterH))<<16) |
//          (uint32_t)dirEntry->firstClusterL;
//      println("File is at cluster %d", (unsigned int)cluster);
//      println("File is at sector %d", (unsigned int)FAT_Cluster2Sector(cluster));
//      println("File size is %d",(unsigned int)dirEntry->fileSize);
//
//      phyCallbacks.phyReadSectors(buf2, FAT_Cluster2Sector(cluster), 1);
//
//      hexdump(buf2, 16);
//      println("%s",buf2);
//
//      FAT_GetEntryInFAT(cluster);
//    }
//
//    dirEntry++;
//  }
//
//}

/**
 * @}
 */
