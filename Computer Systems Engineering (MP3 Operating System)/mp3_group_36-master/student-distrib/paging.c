#include "paging.h"
#include "paging_asm.h"

/* checkpoint 1 */
/*
paging_init
    DESCRIPTION:    Initializes the paging bits.
    INPUTS:         pde_ptr -- 
    OUTPUTS:
    RETURN VALUE:
    EFFECT: 
*/
void paging_init() {
    /* initialize */
    int i, j;

    /* set each page directory entry */
    for (i = 0; i < 1024; i++) {
        /* video mem 4kb */
        if (i == 0) {
            page_directory[i].page_dir_4kb.p = 1;
            page_directory[i].page_dir_4kb.r_w = 1;
            page_directory[i].page_dir_4kb.u_s = 0;
            page_directory[i].page_dir_4kb.pwt = 0;
            page_directory[i].page_dir_4kb.pcd = 0;
            page_directory[i].page_dir_4kb.a = 0;
            page_directory[i].page_dir_4kb.avl = 0;
            page_directory[i].page_dir_4kb.ps = 0;
            page_directory[i].page_dir_4kb.avail = 0;
            page_directory[i].page_dir_4kb.page_table_base_addr = (unsigned int)page_table >> 12;
        }
        
        /* kernel 4mb */
        else if (i == 1) {
            page_directory[i].page_dir_4mb.p = 1;
            page_directory[i].page_dir_4mb.r_w = 1; // changed from 1
            page_directory[i].page_dir_4mb.u_s = 0;
            page_directory[i].page_dir_4mb.pwt = 0;

            page_directory[i].page_dir_4mb.pcd = 0;
            page_directory[i].page_dir_4mb.a = 0;
            page_directory[i].page_dir_4mb.d = 0;
            page_directory[i].page_dir_4mb.ps = 1;

            page_directory[i].page_dir_4mb.g = 1;
            page_directory[i].page_dir_4mb.avl = 0;
            page_directory[i].page_dir_4mb.pat = 0;
            page_directory[i].page_dir_4mb.bits39_to_32 = 0;
            page_directory[i].page_dir_4mb.rsvd = 0;
            page_directory[i].page_dir_4mb.page_table_base_addr = 1;
        }

        /* 8mb to 4gb */
        else {
            page_directory[i].page_dir_4kb.p = 0;
            page_directory[i].page_dir_4kb.r_w = 0;
            page_directory[i].page_dir_4kb.u_s = 0;
            page_directory[i].page_dir_4kb.pwt = 0;
            page_directory[i].page_dir_4kb.pcd = 0;
            page_directory[i].page_dir_4kb.a = 0;
            page_directory[i].page_dir_4kb.avl = 0;
            page_directory[i].page_dir_4kb.ps = 0;
            page_directory[i].page_dir_4kb.avail = 0;
            page_directory[i].page_dir_4kb.page_table_base_addr = 0;
        }
    }
    for (j = 0; j < 1024; j++) {
        if (j == PAGE_TABLE_VIDEO_MEM_OFFSET) {
            page_table[j].page_table_4kb.p = 1;
            page_table[j].page_table_4kb.r_w = 1;
            page_table[j].page_table_4kb.u_s = 0;
            page_table[j].page_table_4kb.pwt = 0;
            page_table[j].page_table_4kb.pcd = 0;
            page_table[j].page_table_4kb.a = 0;
            page_table[j].page_table_4kb.d = 0;
            page_table[j].page_table_4kb.pat = 0;
            page_table[j].page_table_4kb.g = 0;
            page_table[j].page_table_4kb.avail = 0;
            page_table[PAGE_TABLE_VIDEO_MEM_OFFSET].page_table_4kb.page_base_addr = PAGE_TABLE_VIDEO_MEM_OFFSET;
        }
        // always pointing to video mem
        else if(j == PAGE_TABLE_VIDEO_MEM_OFFSET-1){
            page_table[j].page_table_4kb.p = 1;
            page_table[j].page_table_4kb.r_w = 1;
            page_table[j].page_table_4kb.u_s = 0;
            page_table[j].page_table_4kb.pwt = 0;
            page_table[j].page_table_4kb.pcd = 0;
            page_table[j].page_table_4kb.a = 0;
            page_table[j].page_table_4kb.d = 0;
            page_table[j].page_table_4kb.pat = 0;
            page_table[j].page_table_4kb.g = 0;
            page_table[j].page_table_4kb.avail = 0;
            page_table[j].page_table_4kb.page_base_addr = PAGE_TABLE_VIDEO_MEM_OFFSET;
        }

        else if(j == PAGE_TABLE_VIDEO_MEM_OFFSET+1){
            page_table[j].page_table_4kb.p = 1;
            page_table[j].page_table_4kb.r_w = 1;
            page_table[j].page_table_4kb.u_s = 0;
            page_table[j].page_table_4kb.pwt = 0;
            page_table[j].page_table_4kb.pcd = 0;
            page_table[j].page_table_4kb.a = 0;
            page_table[j].page_table_4kb.d = 0;
            page_table[j].page_table_4kb.pat = 0;
            page_table[j].page_table_4kb.g = 0;
            page_table[j].page_table_4kb.avail = 0;
            page_table[j].page_table_4kb.page_base_addr = j;
        }
        else if(j == PAGE_TABLE_VIDEO_MEM_OFFSET+2){
            page_table[j].page_table_4kb.p = 1;
            page_table[j].page_table_4kb.r_w = 1;
            page_table[j].page_table_4kb.u_s = 0;
            page_table[j].page_table_4kb.pwt = 0;
            page_table[j].page_table_4kb.pcd = 0;
            page_table[j].page_table_4kb.a = 0;
            page_table[j].page_table_4kb.d = 0;
            page_table[j].page_table_4kb.pat = 0;
            page_table[j].page_table_4kb.g = 0;
            page_table[j].page_table_4kb.avail = 0;
            page_table[j].page_table_4kb.page_base_addr = j;
        }
        else if(j == PAGE_TABLE_VIDEO_MEM_OFFSET+3){
            page_table[j].page_table_4kb.p = 1;
            page_table[j].page_table_4kb.r_w = 1;
            page_table[j].page_table_4kb.u_s = 0;
            page_table[j].page_table_4kb.pwt = 0;
            page_table[j].page_table_4kb.pcd = 0;
            page_table[j].page_table_4kb.a = 0;
            page_table[j].page_table_4kb.d = 0;
            page_table[j].page_table_4kb.pat = 0;
            page_table[j].page_table_4kb.g = 0;
            page_table[j].page_table_4kb.avail = 0;
            page_table[j].page_table_4kb.page_base_addr = j;
        }
        else {
            page_table[j].page_table_4kb.p = 0;
            page_table[j].page_table_4kb.r_w = 0;
            page_table[j].page_table_4kb.u_s = 0;
            page_table[j].page_table_4kb.pwt = 0;
            page_table[j].page_table_4kb.pcd = 0;
            page_table[j].page_table_4kb.a = 0;
            page_table[j].page_table_4kb.d = 0;
            page_table[j].page_table_4kb.pat = 0;
            page_table[j].page_table_4kb.g = 0;
            page_table[j].page_table_4kb.avail = 0;
            page_table[j].page_table_4kb.page_base_addr = 0;
        }
    }
    load_page_directory(page_directory);
}

/* checkpoint 3 */

/*
paging_set_up
    DESCRIPTION:    Sets up the 8MB pages for processes.
    INPUTS:         cur_process-num -- Offset for stack math
    OUTPUTS:
    RETURN VALUE:
    EFFECT: 
*/
void paging_set_up(uint32_t cur_process_num) {
    /* initialize */
    // int i;

    page_directory[_128MB_SHIFTED_R_22].page_dir_4mb.p = 1;
    page_directory[_128MB_SHIFTED_R_22].page_dir_4mb.r_w = 1;
    page_directory[_128MB_SHIFTED_R_22].page_dir_4mb.u_s = 1;
    page_directory[_128MB_SHIFTED_R_22].page_dir_4mb.pwt = 0;
    page_directory[_128MB_SHIFTED_R_22].page_dir_4mb.pcd = 0;
    page_directory[_128MB_SHIFTED_R_22].page_dir_4mb.a = 0;
    page_directory[_128MB_SHIFTED_R_22].page_dir_4mb.d = 0;
    page_directory[_128MB_SHIFTED_R_22].page_dir_4mb.ps = 1;
    page_directory[_128MB_SHIFTED_R_22].page_dir_4mb.g = 0;
    page_directory[_128MB_SHIFTED_R_22].page_dir_4mb.avl = 0;
    page_directory[_128MB_SHIFTED_R_22].page_dir_4mb.pat = 0;
    page_directory[_128MB_SHIFTED_R_22].page_dir_4mb.bits39_to_32 = 0;
    page_directory[_128MB_SHIFTED_R_22].page_dir_4mb.rsvd = 0;
    page_directory[_128MB_SHIFTED_R_22].page_dir_4mb.page_table_base_addr = (EIGHT_MB + cur_process_num * FOUR_MB) >> 22;   // shift 22 to align

    /* flush TLB */
    tlb_flush();
}





/*
paging_tear_down
    DESCRIPTION:    Tears down the 8MB pages for processes.
    INPUTS:         cur_process-num -- Offset for stack math
    OUTPUTS:
    RETURN VALUE:
    EFFECT: 
*/
void paging_tear_down(uint32_t cur_process_num) {
    page_directory[_128MB_SHIFTED_R_22].page_dir_4mb.page_table_base_addr = (EIGHT_MB + cur_process_num * FOUR_MB) >> 22;   // shift 22 to align
    tlb_flush();
}

/*
user_vidmem_setup
    DESCRIPTION:    Sets up video memory in the user space
    INPUTS:         
    OUTPUTS:
    RETURN VALUE:
    EFFECT: 
*/
void user_vidmem_setup() {
    int j;
    /*placing page table at 136MB VM, might need to be in a different spot, idk*/
    page_directory[_136MB_SHIFTED_R_22].page_dir_4kb.p = 1;
    page_directory[_136MB_SHIFTED_R_22].page_dir_4kb.u_s = 1;
    page_directory[_136MB_SHIFTED_R_22].page_dir_4kb.r_w = 1;
    page_directory[_136MB_SHIFTED_R_22].page_dir_4kb.page_table_base_addr = (unsigned int)vidmap_page_table >> 12;
    /*filling out vidmap page table, must give user-level access, present, rw, and u_s must be set to 1*/
    for (j = 0; j < 1024; j++) {
        if (j == 0) {
            vidmap_page_table[0].page_table_4kb.p = 1;
            vidmap_page_table[0].page_table_4kb.r_w = 1;
            vidmap_page_table[0].page_table_4kb.u_s = 1;
            vidmap_page_table[0].page_table_4kb.pwt = 0;
            vidmap_page_table[0].page_table_4kb.pcd = 0;
            vidmap_page_table[0].page_table_4kb.a = 0;
            vidmap_page_table[0].page_table_4kb.d = 0;
            vidmap_page_table[0].page_table_4kb.pat = 0;
            vidmap_page_table[0].page_table_4kb.g = 0;
            vidmap_page_table[0].page_table_4kb.avail = 0;
            vidmap_page_table[0].page_table_4kb.page_base_addr = PAGE_TABLE_VIDEO_MEM_OFFSET;
        }
        else{
            vidmap_page_table[j].page_table_4kb.p = 0;
            vidmap_page_table[j].page_table_4kb.r_w = 1;
            vidmap_page_table[j].page_table_4kb.u_s = 0;
            vidmap_page_table[j].page_table_4kb.pwt = 0;
            vidmap_page_table[j].page_table_4kb.pcd = 0;
            vidmap_page_table[j].page_table_4kb.a = 0;
            vidmap_page_table[j].page_table_4kb.d = 0;
            vidmap_page_table[j].page_table_4kb.pat = 0;
            vidmap_page_table[j].page_table_4kb.g = 0;
            vidmap_page_table[j].page_table_4kb.avail = 0;
            vidmap_page_table[j].page_table_4kb.page_base_addr = j;
        }
    }

    /* flush TLB */
    tlb_flush();
}


/*
user_vidmem_teardown
    DESCRIPTION:    Tears down video memory in the user space
    INPUTS:         
    OUTPUTS:
    RETURN VALUE:
    EFFECT: 
*/
void user_vidmem_teardown() {
    int j;
    /*resetting pde back to the original state*/
    page_directory[_136MB_SHIFTED_R_22].page_dir_4kb.p = 0;
    page_directory[_136MB_SHIFTED_R_22].page_dir_4kb.u_s = 0;
    page_directory[_136MB_SHIFTED_R_22].page_dir_4kb.r_w = 0;
    page_directory[_136MB_SHIFTED_R_22].page_dir_4kb.page_table_base_addr = 0;
    //resetting page back to original state
    for (j = 0; j < 1024; j++) {
        vidmap_page_table[j].page_table_4kb.p = 0;
        vidmap_page_table[j].page_table_4kb.r_w = 0;
        vidmap_page_table[j].page_table_4kb.u_s = 0;
        vidmap_page_table[j].page_table_4kb.pwt = 0;
        vidmap_page_table[j].page_table_4kb.pcd = 0;
        vidmap_page_table[j].page_table_4kb.a = 0;
        vidmap_page_table[j].page_table_4kb.d = 0;
        vidmap_page_table[j].page_table_4kb.pat = 0;
        vidmap_page_table[j].page_table_4kb.g = 0;
        vidmap_page_table[j].page_table_4kb.avail = 0;
        vidmap_page_table[j].page_table_4kb.page_base_addr = 0;
    }

    /* flush TLB */
    tlb_flush();
}

/*
We don't use this
*/
void terminal_setup(int old_terminal, int new_terminal) {

    int terminal_num = current_terminal_num();
    page_directory[_128MB_SHIFTED_R_22].page_dir_4mb.p = 1;
    page_directory[_128MB_SHIFTED_R_22].page_dir_4mb.r_w = 1;
    page_directory[_128MB_SHIFTED_R_22].page_dir_4mb.u_s = 1;
    page_directory[_128MB_SHIFTED_R_22].page_dir_4mb.pwt = 0;
    page_directory[_128MB_SHIFTED_R_22].page_dir_4mb.pcd = 0;
    page_directory[_128MB_SHIFTED_R_22].page_dir_4mb.a = 0;
    page_directory[_128MB_SHIFTED_R_22].page_dir_4mb.d = 0;
    page_directory[_128MB_SHIFTED_R_22].page_dir_4mb.ps = 1;
    page_directory[_128MB_SHIFTED_R_22].page_dir_4mb.g = 0;
    page_directory[_128MB_SHIFTED_R_22].page_dir_4mb.avl = 0;
    page_directory[_128MB_SHIFTED_R_22].page_dir_4mb.pat = 0;
    page_directory[_128MB_SHIFTED_R_22].page_dir_4mb.bits39_to_32 = 0;
    page_directory[_128MB_SHIFTED_R_22].page_dir_4mb.rsvd = 0;
    page_directory[_128MB_SHIFTED_R_22].page_dir_4mb.page_table_base_addr = (EIGHT_MB + terminal_num * FOUR_MB) >> 22;

    tlb_flush();
}

/*
video_mem_remap
    DESCRIPTION:    Makes it so that the visual terminal's programs get displayed
                    and other terminals do not get displayed (by changing their mapping)
    INPUTS:         nextTerminal
    OUTPUTS:
    RETURN:
    SIDE-EFFECT:    Changes video memory mapping of input terminal.
*/
void video_mem_remap(int nextTerminal){
    int visual_terminal = current_terminal_num();
    /*if the scheduled terminal is same as the one we're looking at, map one to one*/
    if(nextTerminal == visual_terminal){
        page_table[PAGE_TABLE_VIDEO_MEM_OFFSET].page_table_4kb.p = 1;
        page_table[PAGE_TABLE_VIDEO_MEM_OFFSET].page_table_4kb.r_w = 1;
        page_table[PAGE_TABLE_VIDEO_MEM_OFFSET].page_table_4kb.u_s = 0;
        page_table[PAGE_TABLE_VIDEO_MEM_OFFSET].page_table_4kb.pwt = 0;
        page_table[PAGE_TABLE_VIDEO_MEM_OFFSET].page_table_4kb.pcd = 0;
        page_table[PAGE_TABLE_VIDEO_MEM_OFFSET].page_table_4kb.a = 0;
        page_table[PAGE_TABLE_VIDEO_MEM_OFFSET].page_table_4kb.d = 0;
        page_table[PAGE_TABLE_VIDEO_MEM_OFFSET].page_table_4kb.pat = 0;
        page_table[PAGE_TABLE_VIDEO_MEM_OFFSET].page_table_4kb.g = 0;
        page_table[PAGE_TABLE_VIDEO_MEM_OFFSET].page_table_4kb.avail = 0;
        page_table[PAGE_TABLE_VIDEO_MEM_OFFSET].page_table_4kb.page_base_addr = PAGE_TABLE_VIDEO_MEM_OFFSET;

        
    }
    /*map to page buffer*/
    else{
        page_table[PAGE_TABLE_VIDEO_MEM_OFFSET].page_table_4kb.p = 1;
        page_table[PAGE_TABLE_VIDEO_MEM_OFFSET].page_table_4kb.r_w = 1;
        page_table[PAGE_TABLE_VIDEO_MEM_OFFSET].page_table_4kb.u_s = 0;
        page_table[PAGE_TABLE_VIDEO_MEM_OFFSET].page_table_4kb.pwt = 0;
        page_table[PAGE_TABLE_VIDEO_MEM_OFFSET].page_table_4kb.pcd = 0;
        page_table[PAGE_TABLE_VIDEO_MEM_OFFSET].page_table_4kb.a = 0;
        page_table[PAGE_TABLE_VIDEO_MEM_OFFSET].page_table_4kb.d = 0;
        page_table[PAGE_TABLE_VIDEO_MEM_OFFSET].page_table_4kb.pat = 0;
        page_table[PAGE_TABLE_VIDEO_MEM_OFFSET].page_table_4kb.g = 0;
        page_table[PAGE_TABLE_VIDEO_MEM_OFFSET].page_table_4kb.avail = 0;
        page_table[PAGE_TABLE_VIDEO_MEM_OFFSET].page_table_4kb.page_base_addr = (PAGE_TABLE_VIDEO_MEM_OFFSET + (nextTerminal + 1));

       
    }
    
    tlb_flush();
}

/*
fish_remap
    DESCRIPTION:    If the previous visual terminal was running fish, remap it so that
                    it no longer mapped to video memory. If the next visual terminal is running
                    fish, remap it to video memory
    INPUTS:         nextTerminal
    OUTPUTS:
    RETURN:
    SIDE-EFFECT:    Changes mapping of previous and next terminal.
*/
void fish_remap(int nextTerminal){
    /*grab visual terminal info and terminal we are going to */
    int visual_terminal = current_terminal_num();
    process_info_t* allTerminals = ret_multi_process();
    process_info_t* vTerminal  = &allTerminals[visual_terminal];
    process_info_t* next_terminal = & allTerminals[nextTerminal];
    /*if terminal we are leaving has fish running, map to it's terminal buffer*/ 
    if(vTerminal->fish){
        vidmap_page_table[0].page_table_4kb.page_base_addr = (PAGE_TABLE_VIDEO_MEM_OFFSET + (visual_terminal + 1));

    }
    /*if terminal we are going to has fish running, map to physical mem*/
    else if(next_terminal->fish){
        vidmap_page_table[0].page_table_4kb.page_base_addr = PAGE_TABLE_VIDEO_MEM_OFFSET;
    }
    else{}
    /*flush TLB*/
    tlb_flush();
}


