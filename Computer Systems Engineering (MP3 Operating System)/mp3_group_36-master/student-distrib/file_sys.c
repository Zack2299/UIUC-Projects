
#include "file_sys.h"

/*
file_sys_init
    DESCRIPTION:    Initializes the paging bits.
    INPUTS:         mbi -- ptr to struct that holds start addr of file system
    OUTPUTS:        
    RETURN VALUE:   -1 -- invalid ptr
                    
    EFFECT:         Initializes the file sys
*/
int file_sys_init(module_t* mbi) {
    printf("mod start: %d\n", (uint32_t) mbi->mod_start);
    /* init file descriptor table*/

    /* grab starting mem addr and set it globally */
    
    blocks_start = (boot_block_t*)(mbi->mod_start);
    inode_start = (inode_t*)(blocks_start + 1);
    data_block_start = (data_block_t*) (blocks_start + blocks_start->num_inodes + 1);

    /* invalid mem addr */
    if (blocks_start == NULL) {
        return -1;
    }
    /* valid mem addr */
    
    return 0;
}

/*
file_sys_open
    DESCRIPTION:    Initializes the paging bits.
    INPUTS:          
    OUTPUTS:
    RETURN VALUE:
    EFFECT: 
*/
int file_sys_open(const uint8_t* filename) {
    return 0;
}

/*
file_sys_close
    DESCRIPTION:    Initializes the paging bits.
    INPUTS:          
    OUTPUTS:
    RETURN VALUE:
    EFFECT: 
*/
int file_sys_close(int32_t fd) {
    return 0;
}

/*
file_sys_read
    DESCRIPTION:    Initializes the paging bits.
    INPUTS:          
    OUTPUTS:
    RETURN VALUE:
    EFFECT: 
*/
int file_sys_read(int32_t fd, void* buf, int32_t nbytes) {
    // inode_t inode_for_file = file_descriptors[fd].inode;
    // copy count byte of data from datablocks of inode
    if(buf == NULL || nbytes < 0)
        return -1;
    int cur_process = get_pcb_idx();
    pcb_t* curPCB = get_pcb(cur_process);
    int bytes_read = read_data(curPCB->file_desc_table[fd].inode, curPCB->file_desc_table[fd].file_pos, (uint8_t*)buf, nbytes);
    
    curPCB->file_desc_table[fd].file_pos += bytes_read;
    return bytes_read;

}

/*
file_sys_write
    DESCRIPTION:    Initializes the paging bits.
    INPUTS:          
    OUTPUTS:
    RETURN VALUE:
    EFFECT: 
*/
int file_sys_write(int32_t fd, const void* buf, int32_t nbytes) {    // CP2: does nothing
    return -1;
}

