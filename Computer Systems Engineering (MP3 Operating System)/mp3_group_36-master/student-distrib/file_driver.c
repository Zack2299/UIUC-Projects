
#include "file_sys.h"

/*
read_dentry_by_name
    DESCRIPTION:    Grabs directory entry's ptr and sets argument to ptr
    INPUTS:         file_name -- String to find 
                    dir_entry -- Struct ptr to set
    OUTPUTS:        n/a
    RETURN VALUE:   -1 -- Fail
                    0 -- Success
    EFFECT:         Sets struct ptr
*/
uint32_t read_dentry_by_name(const uint8_t* file_name, dir_entry_t* dir_entry) {
    /* initialize */
    dir_entry_t* ret_val;

    /* call helper to find */
    ret_val = traverse_dentry_by_name(file_name);

    /* never found */
    if (ret_val == NULL) {
        return -1;
    }

    /* return -1 if file name is too big*/
    if (strlen((int8_t*)file_name) > MAX_FILE_NAME_SIZE) {
        strcpy((int8_t*)(dir_entry->file_name), (int8_t*)(ret_val->file_name));
        dir_entry->file_type = ret_val->file_type;
        dir_entry->inode = ret_val->inode;
        dir_entry->file_name[MAX_FILE_NAME_SIZE] = '\0';
        return -1;
    }

    /* found */

    /* deep copy */
    strcpy((int8_t*)(dir_entry->file_name), (int8_t*)(ret_val->file_name));
    dir_entry->file_type = ret_val->file_type;
    dir_entry->inode = ret_val->inode;

    /* return 0 */
    return 0;
}

/*
traverse_dentry_by_name
    DESCRIPTION:    Traverses and finds specific directory entry based off file name
    INPUTS:         file_name -- String to reference and search for
    OUTPUTS:        n/a
    RETURN VALUE:   dir_entry_file_name -- Ptr to struct in 64B directory entries
                    NULL -- Failed to find matching file
    EFFECT: 
*/
dir_entry_t* traverse_dentry_by_name(const uint8_t* file_name) {
    /* initialize */
    int i;

    /* 
        skip the first 64B to get to the file name of size 32B 
            ptr to beginning of boot block    
    */
    dir_entry_t* dir_entry_file_name = ((dir_entry_t*)blocks_start);

    /* 
        increment ptr to beginning of dir entries 
            ptr = ptr + 64B    
    */
    dir_entry_file_name++;

    /* traverse the dir entries */
    for (i = 0; i < 63; i++) {
        /* found struct containing name */
        if (strncmp((char*)file_name, (char*)dir_entry_file_name, MAX_FILE_NAME_SIZE) == 0) {
            /* return ptr to specific directory entry */
            return dir_entry_file_name;
        }

        /* not found */
        
        /* increment ptr */
        dir_entry_file_name++;
    }

    /* never found */
    return NULL;
}

/*
read_directory
    DESCRIPTION:    Grabs directory entry's ptr and sets argument to ptr
    INPUTS:         index -- Index value (ptr offset) into the boot block's 64B directory entries 
                    dir_entry -- struct ptr to set
    OUTPUTS:        n/a
    RETURN VALUE:   -1 -- fail
                    0 -- success
    EFFECT:         Sets struct ptr
*/
int32_t read_directory(int32_t fd, void* buf, int32_t nbytes) {
    /* initialize */
    dir_entry_t dentry;   
    int cur_process = get_pcb_idx();
    pcb_t* cur_pcb_ptr = get_pcb(cur_process);
    int32_t counter = cur_pcb_ptr->file_desc_table[fd].file_pos;

    if( read_dentry_by_index(counter, &dentry) == -1){
        return 0;
    }
    strncpy((int8_t*)buf, (int8_t*)&(dentry.file_name), MAX_FILE_NAME_SIZE);
    int32_t bytes_read = strlen((int8_t*)&(dentry.file_name));

    counter++;  
    cur_pcb_ptr->file_desc_table[fd].file_pos = counter;


    if(bytes_read > MAX_FILE_NAME_SIZE){
        return MAX_FILE_NAME_SIZE;
    }
    return bytes_read;

}

/*
open_directory
    DESCRIPTION:    
    INPUTS:         filename -- input
    OUTPUTS:    
    RETURN VALUE:
    EFFECT:         n/a
*/
int32_t open_directory(const uint8_t* filename){
    return 0;
}

/*
close_directory
    DESCRIPTION:    
    INPUTS:         fd -- input
    OUTPUTS:    
    RETURN VALUE:
    EFFECT:         n/a
*/
int32_t close_directory(int32_t fd){
    // return -1;
    return 0;
}

/*
write_directory
    DESCRIPTION:    
    INPUTS:         fd -- input
                    buf -- input
                    nbytes -- input
    OUTPUTS:    
    RETURN VALUE:
    EFFECT:         n/a
*/
int32_t write_directory(int32_t fd, const void* buf, int32_t nbytes){
    return 0;
}

/*
read_dentry_by_index
    DESCRIPTION:    Grabs directory entry's ptr and sets argument to ptr
    INPUTS:         index -- Index value (ptr offset) into the boot block's 64B directory entries 
                    dir_entry -- struct ptr to set
    OUTPUTS:        n/a
    RETURN VALUE:   -1 -- fail
                    0 -- success
    EFFECT:         Sets struct ptr
*/
uint32_t read_dentry_by_index(const uint32_t index, dir_entry_t* dir_entry) {
    /* initialize */
    dir_entry_t* ret_val;

    /* call helper to find */
    ret_val = traverse_dentry_by_index(index);

    /* never found */
    if (ret_val == NULL) {
        return -1;
    }

    /* found */

    /* deep copy */
    strcpy((int8_t*)(dir_entry->file_name), (int8_t*)(ret_val->file_name));
    dir_entry->file_type = ret_val->file_type;
    dir_entry->inode = ret_val->inode;  

    /* return 0 */
    return 0;
}

/*
traverse_dentry_by_index
    DESCRIPTION:    Traverses and finds specific directory entry based off index
    INPUTS:         index -- Index to offset ptr by
    OUTPUTS:        n/a
    RETURN VALUE:   dir_entry_t_contents -- Ptr to struct in 64B directory entries
                    NULL -- Failed to find matching file
    EFFECT: 
*/
dir_entry_t* traverse_dentry_by_index(const uint32_t index) {
    /* check index boundaries */
    if (index < 0 || index > 62) {  // 62 because there's 63 files but 62 excluding '.'
        return NULL;
    }

    /* index boundaries valid */

    /* 
        skip the first 64B to get to the file name of size 32B 
            ptr to beginning of boot block    
    */
    dir_entry_t* dir_entry_file_name = ((dir_entry_t*)blocks_start);

    /* 
        increment ptr to beginning of dir entries 
            ptr = ptr + 64B    
    */
    dir_entry_file_name++;

    /* save directory struct of specific index to tmp */
    dir_entry_t* dir_entry_t_contents = dir_entry_file_name + index;

    /* return contents of found struct */
    return dir_entry_t_contents;
}


/*
read_data
    DESCRIPTION:    Reads data from specific inode/data block and sends to *buf
    INPUTS:         inode -- Specific inode number that will contain the data
                    offset -- Offset to start extracting data
                    buf -- Buffer to contain data
                    length -- How much data to grab from start 
    OUTPUTS:
    RETURN VALUE:   -1 -- Fail
                    0 -- Success
    EFFECT: 
*/
uint32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length) {
    /*
        make it compatible w/...
            open, close, read
        makes it easier for CP 3 
    */

    /* initialize */

    /* invalid inode number */
    if (inode < 0 || inode > 62) {  // 62 because 62 excluding '.'
        return -1;
    }

    /* valid inode number */

    /* FILL IN ZACK */
    /* OKAY JOSH */
    inode_t* inode_ptr = inode_start + inode;
    int inode_length = inode_ptr->len_in_B;

    if (inode_length <= offset)
        return 0;

    uint32_t* inode_data_block_num = ((uint32_t*)inode_ptr) + 1; // ptr to 0th data block # of inode
    inode_data_block_num += offset/FOUR_KB; // get correct data block we should start reading from based on offset
                                            // divide by 4096 because there are 4096 bytes per data block
    // get first data block to read from
    data_block_t* data_block_ptr = data_block_start + *inode_data_block_num;

    /*
        problem here
        edit: FIXED, I'M SO GOATED -Jose
    */
    int i;
    int bytes_read_in_curr_block;
    int total_bytes_read = 0;
    bytes_read_in_curr_block = offset % FOUR_KB;
    buf[0] = data_block_ptr->data_block[bytes_read_in_curr_block];
    total_bytes_read++;
    for (i = 1; i < length; i++) {
        if (total_bytes_read + offset > inode_length)
            break;

        if (bytes_read_in_curr_block >= FOUR_KB-1) { // counter fix is here, added the minus one
            // get next data block num and update data_block_ptr to point to that new data block
            inode_data_block_num++;
            data_block_ptr = data_block_start + *inode_data_block_num;
            // new data block so reset bytes read in current block
            bytes_read_in_curr_block = 0;
        } else {
            bytes_read_in_curr_block++;
        }
        total_bytes_read++;
        // copy the data over from the curr data block into the buffer
        buf[i] = data_block_ptr->data_block[bytes_read_in_curr_block];
    }

    return total_bytes_read;
}
