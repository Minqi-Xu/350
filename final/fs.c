#include "disk.h"
#include "fs.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

#define min(a,b) (((a) < (b)) ? (a) : (b))

#ifdef DEBUG
#define DEBUG_PRINT(fmt, args...)    fprintf(stderr, fmt, ## args)
#else
#define DEBUG_PRINT(fmt, args...)    /* Don't do anything in release builds */
#endif

// Debug file system -----------------------------------------------------------

void fs_debug(Disk *disk)
{
    if (disk == 0)
        return;

    Block block;

    // Read Superblock
    disk_read(disk, 0, block.Data);

    uint32_t magic_num = block.Super.MagicNumber;
    uint32_t num_blocks = block.Super.Blocks;
    uint32_t num_inodeBlocks = block.Super.InodeBlocks;
    uint32_t num_inodes = block.Super.Inodes;

    if (magic_num != MAGIC_NUMBER)
    {
        printf("Magic number is valid: %c\n", magic_num);
        return;
    }

    printf("SuperBlock:\n");
    printf("    magic number is valid\n");
    printf("    %u blocks\n", num_blocks);
    printf("    %u inode blocks\n", num_inodeBlocks);
    printf("    %u inodes\n", num_inodes);

    uint32_t expected_num_inodeBlocks = round((float)num_blocks / 10);

    if (expected_num_inodeBlocks != num_inodeBlocks)
    {
        printf("SuperBlock declairs %u InodeBlocks but expect %u InodeBlocks!\n", num_inodeBlocks, expected_num_inodeBlocks);
    }

    uint32_t expect_num_inodes = num_inodeBlocks * INODES_PER_BLOCK;
    if (expect_num_inodes != num_inodes)
    {
        printf("SuperBlock declairs %u Inodes but expect %u Inodes!\n", num_inodes, expect_num_inodes);
    }

    // FIXME: Read Inode blocks
    uint32_t count = 0;
    for(uint32_t i = 1; i <= num_inodeBlocks; i++) {
        disk_read(disk, i, block.Data);
        for(uint32_t j = 0; j < INODES_PER_BLOCK; j++) {
            if(count >= num_inodes)
                break;
            if(block.Inodes[j].Valid == 0) {
                count++;
                continue;
            }
            printf("Inode %u:\n", count);
            printf("    size: %u bytes\n", block.Inodes[j].Size);
            printf("    direct blocks:");
            for(int k = 0; k < POINTERS_PER_INODE; k++) {
                if(k * 4096 < block.Inodes[j].Size)
                    printf(" %u", block.Inodes[j].Direct[k]);
                else
                    break;
            }
            printf("\n");
            if(block.Inodes[j].Size > 4096 * 5) {
                printf("    indirect block: %u\n", block.Inodes[j].Indirect);
                uint32_t indirect = block.Inodes[j].Indirect;
                Block temp_block;
                disk_read(disk, indirect, temp_block.Data);
                uint32_t inode_size = block.Inodes[j].Size;
                uint32_t indir_used_datablock = (inode_size - 4096 * 5) / 4096;
                if(inode_size % 4096 != 0)
                    indir_used_datablock++;
                printf("    indirect data blocks:");
                for(uint32_t k = 0; k < indir_used_datablock; k++) {
                    printf(" %u", temp_block.Pointers[k]);
                }
                printf("\n");
            }
            count++;
        }

        if(count > num_inodes)
            break;
    }


}

// Format file system ----------------------------------------------------------

bool fs_format(Disk *disk)
{
    if(disk_mounted(disk)) {
        return false;
    }
    // Write superblock
    uint32_t num_blocks = disk->Blocks;
    uint32_t num_inodeBlocks = round((float)num_blocks / 10);
    Block block;
    block.Super.Blocks = num_blocks;
    block.Super.InodeBlocks = num_inodeBlocks;
    block.Super.MagicNumber = MAGIC_NUMBER;
    block.Super.Inodes = num_inodeBlocks * INODES_PER_BLOCK;
    disk_write(disk, 0, block.Data);

    // Clear all other blocks
    for(uint32_t j = 0; j < INODES_PER_BLOCK; j++) {
        block.Inodes[j].Valid = 0;
        block.Inodes[j].Indirect = 0;
        block.Inodes[j].Size = 0;
        for(uint32_t k = 0; k < POINTERS_PER_INODE; k++) {
            block.Inodes[j].Direct[k] = 0;
        }
    }
    for(uint32_t i = 1; i <= num_inodeBlocks; i++) {
        disk_write(disk, i, block.Data);
    }

    return true;
}

// FileSystem constructor
FileSystem *new_fs()
{
    FileSystem *fs = malloc(sizeof(FileSystem));
    return fs;
}

// FileSystem destructor
void free_fs(FileSystem *fs)
{
    // FIXME: free resources and allocated memory in FileSystem
    free(fs->bitmap);
    free(fs);
}

// Mount file system -----------------------------------------------------------

bool fs_mount(FileSystem *fs, Disk *disk)
{
    if(disk == NULL) {
        return false;
    }

    if(fs == NULL) {
        return false;
    }

    if(fs->disk != NULL && disk_mounted(fs->disk)) {
        return false;
    }

    // Read superblock
    Block block;
    disk_read(disk, 0, block.Data);
    if(block.Super.MagicNumber != MAGIC_NUMBER) {
        return false;
    }
    uint32_t num_blocks = block.Super.Blocks;
    uint32_t num_inodeBlocks = block.Super.InodeBlocks;
    uint32_t num_inodes = block.Super.Inodes;
    uint32_t expected_num_inodeBlocks = round((float)num_blocks / 10);

    if (expected_num_inodeBlocks != num_inodeBlocks)
    {
        return false;
    }

    uint32_t expect_num_inodes = num_inodeBlocks * INODES_PER_BLOCK;
    if (expect_num_inodes != num_inodes)
    {
        return false;
    }
    // Set device and mount
    disk_mount(disk);
    fs->disk = disk;

    // Copy metadata

    // Allocate free block bitmap
//    uint32_t num_blocks = block.Super.Blocks;
    fs->bitmap = malloc(sizeof(bool) * num_blocks);

    for(int i = 1; i < num_blocks; i++)
        fs->bitmap[i] = false;      // in bitmap, false means unused (i.e. 0 - unused)

    fs->bitmap[0] = true;       // super block is used.

    uint32_t num_inode_blocks = block.Super.InodeBlocks;
    uint32_t num_inode = block.Super.Inodes;
    uint32_t num_inode_blocks_used = num_inode / INODES_PER_BLOCK;

    if(num_inode % INODES_PER_BLOCK != 0)
        num_inode_blocks_used++;
    for(uint32_t i = 1; i <= num_inode_blocks; i++)
        fs->bitmap[i] = true;       // these blocks are already assigned to inode

    uint32_t count = 0; // count the number of inode
    for(uint32_t i = 1; i <= num_inode_blocks_used; i++) {
        disk_read(disk, i, block.Data);
        for(uint32_t j = 0; j < INODES_PER_BLOCK; j++) {
            if(count >= num_inode)
                break;
            if(block.Inodes[j].Valid == 0) {
                count++;
                continue;
            }
            uint32_t inode_size = block.Inodes[j].Size;
            for(uint32_t k = 0; k < POINTERS_PER_INODE; k++) {
                if(k * 4096 < block.Inodes[j].Size) {
                    if(block.Inodes[j].Direct[k] == 0) {
                        return false;
                    }
                    fs->bitmap[block.Inodes[j].Direct[k]] = true;
                }
                else
                    break;
            }
            if(inode_size > 4096 * 5) {
                if(block.Inodes[j].Indirect == 0) {
                    return false;
                }
                fs->bitmap[block.Inodes[j].Indirect] = true;
                uint32_t indirect = block.Inodes[j].Indirect;
                if(indirect == 0) {
                    return false;
                }
                Block temp_block;
                disk_read(disk, indirect, temp_block.Data);
                uint32_t indir_used_datablock = (inode_size - 4096 * 5) / 4096;
                if(inode_size % 4096 != 0)
                    indir_used_datablock++;
                for(uint32_t k = 0; k < indir_used_datablock; k++) {
                    if(temp_block.Pointers[k] == 0) {
                        return false;
                    }
                    fs->bitmap[temp_block.Pointers[k]] = true;
                }
            }
            count++;
        }
    }

    return true;
}

// Create inode ----------------------------------------------------------------

ssize_t fs_create(FileSystem *fs)
{
    // Locate free inode in inode table
    uint32_t inumber = 0;
    // Read superblock
    Block block;
    ssize_t rtval = -1;
    disk_read(fs->disk, 0, block.Data);
    uint32_t num_inode_blocks = block.Super.InodeBlocks;
    for(int i = 1; i <= num_inode_blocks; i++) {
        Block temp_block;
        disk_read(fs->disk, i, temp_block.Data);
        for(int j = 0; j < INODES_PER_BLOCK; j++) {
            if(temp_block.Inodes[j].Valid == 0) {
                rtval = inumber;
                temp_block.Inodes[j].Size = 0;
                temp_block.Inodes[j].Indirect = 0;
                for(int k = 0; k < POINTERS_PER_INODE; k++) {
                    temp_block.Inodes[j].Direct[k] = 0;
                }
                temp_block.Inodes[j].Valid = 1;
                disk_write(fs->disk, i, temp_block.Data);
            }
            if(rtval != -1) break;
            inumber++;
        }
        if(rtval != -1) break;
    }
    // Record inode if found

    return rtval;
}

// Optional: the following two helper functions may be useful.

// bool find_inode(FileSystem *fs, size_t inumber, Inode *inode)
// {
//     return true;
// }

// bool store_inode(FileSystem *fs, size_t inumber, Inode *inode)
// {
//     return true;
// }

// Remove inode ----------------------------------------------------------------

bool fs_remove(FileSystem *fs, size_t inumber)
{
    // Load inode information
    uint32_t block_num = inumber / INODES_PER_BLOCK + 1;
    uint32_t inode_loc = inumber % INODES_PER_BLOCK;
    Block block;
    disk_read(fs->disk, block_num, block.Data);
    uint32_t inode_size = block.Inodes[inode_loc].Size;

    if(block.Inodes[inode_loc].Valid == 0) {
        return false;
    }

    // Free direct blocks
    for(uint32_t i = 0; i < POINTERS_PER_INODE; i++) {
        if(i * 4096 < inode_size) {
            fs->bitmap[block.Inodes[inode_loc].Direct[i]] = false;
            block.Inodes[inode_loc].Direct[i] = 0;
        }
    }

    // Free indirect blocks
    if(inode_size > 4096 * 5) {
        fs->bitmap[block.Inodes[inode_loc].Indirect] = false;
        uint32_t indirect = block.Inodes[inode_loc].Indirect;
        Block temp_block;
        disk_read(fs->disk, indirect, temp_block.Data);
        uint32_t indir_used_datablock = (inode_size - 4096 * 5) / 4096;
        if(inode_size % 4096 != 0)
            indir_used_datablock++;
        for(uint32_t k = 0; k < indir_used_datablock; k++) {
            fs->bitmap[temp_block.Pointers[k]] = false;
            temp_block.Pointers[k] = 0;
        }
        disk_write(fs->disk, indirect, temp_block.Data);
    }
    block.Inodes[inode_loc].Indirect = 0;

    // Clear inode in inode table
    block.Inodes[inode_loc].Size = 0;
    block.Inodes[inode_loc].Valid = 0;

    disk_write(fs->disk, block_num, block.Data);

    return true;
}

// Inode stat ------------------------------------------------------------------

ssize_t fs_stat(FileSystem *fs, size_t inumber)
{
    // Load inode information
    uint32_t block_num = inumber / INODES_PER_BLOCK + 1;
    uint32_t inode_loc = inumber % INODES_PER_BLOCK;
    Block block;
    disk_read(fs->disk, block_num, block.Data);
    uint32_t inode_size = block.Inodes[inode_loc].Size;

    if(block.Inodes[inode_loc].Valid == 0) {
        return -1;
    }

    return inode_size;
}

// Read from inode -------------------------------------------------------------

ssize_t fs_read(FileSystem *fs, size_t inumber, char *data, size_t length, size_t offset)
{
    // Load inode information
    Block super;        // get the super block
    disk_read(fs->disk, 0, super.Data);
    if(inumber >= super.Super.Inodes) {
        return -1;
    }

    uint32_t block_num = inumber / INODES_PER_BLOCK + 1;
    uint32_t inode_loc = inumber % INODES_PER_BLOCK;
    Block block;        // this block contains the needed inode
    disk_read(fs->disk, block_num, block.Data);
    uint32_t inode_size = block.Inodes[inode_loc].Size;         // size of inode

    if(block.Inodes[inode_loc].Valid == 0) {
        return -1;
    }
    if(inode_size < offset) {
        return -1;
    }


    // Adjust length
    uint32_t true_len = min(length, inode_size - offset);
    uint32_t num_blocks = true_len / 4096;
    if(true_len % 4096 != 0) {
        num_blocks++;
    }



    // Read block and copy to data
    offset = offset / 4096;
    for(int i = 0; i < num_blocks; i++) {
        Block temp;
        if(i + offset < 5) {
            disk_read(fs->disk, block.Inodes[inode_loc].Direct[i+offset], temp.Data);
            memcpy(data + (i * 4096), temp.Data, min(4096, true_len - i * 4096));
        } else {
            disk_read(fs->disk, block.Inodes[inode_loc].Indirect, temp.Data);
            Block datas;
            disk_read(fs->disk, temp.Pointers[i + offset - 5], datas.Data);
            memcpy(data + (i * 4096), datas.Data, min(4096, true_len - i * 4096));
        }
    }

    return true_len;
}

// Optional: the following helper function may be useful.

// ssize_t fs_allocate_block(FileSystem *fs)
// {
//     return -1;
// }

// Write to inode --------------------------------------------------------------

ssize_t fs_write(FileSystem *fs, size_t inumber, char *data, size_t length, size_t offset)
{
    // Load inode
    Block super;
    disk_read(fs->disk, 0, super.Data);
    if(inumber >= super.Super.Inodes) {
//        printf("debug branch 1\n");
        return -1;
    }
    uint32_t block_num = inumber / INODES_PER_BLOCK + 1;
    uint32_t inode_loc = inumber % INODES_PER_BLOCK;
    Block block;
    disk_read(fs->disk, block_num, block.Data);
    uint32_t inode_size = block.Inodes[inode_loc].Size;

    if(block.Inodes[inode_loc].Valid == 0) {
//            printf("debug branch 2\n");
        return -1;
    }
    if(offset / 4096 > POINTERS_PER_BLOCK + POINTERS_PER_INODE - 1) {
//        printf("debug branch 3\n");
        return -1;
    }

    uint32_t original_block_used = inode_size / 4096;
    if(inode_size % 4096 != 0) {
        original_block_used++;
    }

    // Find the true length to write
    uint32_t true_len = min(length, 1029 * 4096 - offset);
    uint32_t num_blocks = true_len / 4096;
    if(true_len % 4096 != 0) {
        num_blocks++;
    }

    offset = offset / 4096;

    // Write block and copy to data
    bool have_space = false;
    bool flag = false;
    uint32_t count_discard = 0;
    uint32_t data_start = super.Super.InodeBlocks + 1;
    for(uint32_t i = data_start; i < super.Super.Blocks; i++) {
        if(fs->bitmap[i] == false) {
            have_space = true;
        }
    }
    for(uint32_t i = 0; i < num_blocks; i++) {
        Block temp;
        if(i + offset < 5) {
            if(i + offset + 1 <= original_block_used) {
                disk_read(fs->disk, block.Inodes[inode_loc].Direct[i+offset], temp.Data);
                memcpy(temp.Data, data + (i * 4096), min(4096, true_len - i * 4096));
                disk_write(fs->disk, block.Inodes[inode_loc].Direct[i+offset], temp.Data);
            } else {
                if(have_space == false) {
                    count_discard++;
                } else {
                    uint32_t ptr;
                    for(ptr = data_start; ptr < super.Super.Blocks; ptr++) {
                        if(fs->bitmap[ptr] == false) {
                            break;
                        }
                    }
                    block.Inodes[inode_loc].Direct[i+offset] = ptr;
                    fs->bitmap[ptr] = true;
                    disk_read(fs->disk, ptr, temp.Data);
                    memcpy(temp.Data, data + (i * 4096), min(4096, true_len - i * 4096));
                    disk_write(fs->disk, ptr, temp.Data);
                    disk_write(fs->disk, block_num, block.Data);
                    have_space = false;
                    for(uint32_t j = data_start; j < super.Super.Blocks; j++) {
                        if(fs->bitmap[j] == false) {
                            have_space = true;
                        }
                    }
                }
            }
        } else {
            if(inode_size <= 5 * 4096) {
                if(have_space == false) {
                    count_discard++;
                    continue;
                } else if(flag == false) {
                    uint32_t ptr;
                    for(ptr = data_start; ptr < super.Super.Blocks; ptr++) {
                        if(fs->bitmap[ptr] == false) {
                            break;
                        }
                    }
                    block.Inodes[inode_loc].Indirect = ptr;
                    fs->bitmap[ptr] = true;
                    disk_write(fs->disk, block_num, block.Data);
                    flag = true;
                    have_space = false;
                    for(uint32_t j = data_start; j < super.Super.Blocks; j++) {
                        if(fs->bitmap[j] == false) {
                            have_space = true;
                        }
                    }
                }
            }
            disk_read(fs->disk, block.Inodes[inode_loc].Indirect, temp.Data);
            if(i + offset <= original_block_used - 1) {
//                printf("debug branch 1\n");
                Block datas;
                disk_read(fs->disk, temp.Pointers[i + offset - 5], datas.Data);
                memcpy(datas.Data, data + (i * 4096), min(4096, true_len - i * 4096));
                disk_write(fs->disk, temp.Pointers[i + offset - 5], datas.Data);
            } else {
//                printf("debug branch 2\n");
                if(have_space == false) {
//                    printf("debug branch 3\n");
                    count_discard++;
                } else {
//                    printf("debug branch 4\n");
                    uint32_t ptr;
                    for(ptr = data_start; ptr < super.Super.Blocks; ptr++) {
                        if(fs->bitmap[ptr] == false) {
                            break;
                        }
                    }
//                    printf("ptr = %u, offset = %lu, i = %u\n", ptr, offset, i);
                    temp.Pointers[i + offset - 5] = ptr;
//                    printf("temp.Pointers[%lu] = %u\n", i+offset-5, temp.Pointers[i + offset - 5]);
                    fs->bitmap[ptr] = true;
                    Block datas;
                    disk_read(fs->disk, ptr, datas.Data);
                    memcpy(datas.Data, data + (i * 4096), min(4096, true_len - i * 4096));
                    disk_write(fs->disk, ptr, datas.Data);
                    disk_write(fs->disk, block.Inodes[inode_loc].Indirect,temp.Data);
                    disk_write(fs->disk, block_num, block.Data);
                    have_space = false;
                    for(uint32_t j = data_start; j < super.Super.Blocks; j++) {
                        if(fs->bitmap[j] == false) {
                            have_space = true;
                        }
                    }
                }
            }

        }
    }

    uint32_t total_num_blocks = num_blocks - count_discard;
    if(total_num_blocks != num_blocks) {
        block.Inodes[inode_loc].Size = (total_num_blocks + offset) * 4096;
        disk_write(fs->disk, block_num, block.Data);
        return total_num_blocks * 4096;
    }
    block.Inodes[inode_loc].Size = true_len + offset * 4096;
    disk_write(fs->disk, block_num, block.Data);
    return true_len;
}
