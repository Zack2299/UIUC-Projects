
#include "sys_call.h"
#include "sys_call_asm.h"
#include "paging.h"
#include "lib.h"
#include "paging_asm.h"

#define _132MB  0x8400000 - 4
#define USER_CS  0x0023
#define USER_DS 0x002B
#define MAX_PROCESSES 6
uint8_t processBitmap[MAX_PROCESSES] = {0,0,0,0,0,0};
int newProcess = -1;
int scheduled_terminal = 2;
pcb_t* currentProcess = NULL;
int currrentProcessNum = -1;

/*
halt
    DESCRIPTION:    Halts the current process and returns to parent.
    INPUTS:         status -- status of process 
    OUTPUTS:    
    RETURN VALUE:
    EFFECT:         Stops the current process.
*/
int32_t halt (uint8_t status) {
    cli();
    /* initialize */
    int i;
    /* clear the process' position in the bitmap */
    process_free(currrentProcessNum);

    /* the current process is a base shell */
    if(currentProcess->parent_id == -1){
        sti();
        execute((const uint8_t*)"shell");
        return 0;
    }
    /* grab the pcb of the parent */
    pcb_t* parentPCB = get_pcb(currentProcess->parent_id);

    /*clearing file descriptor table*/
    for(i = 0; i <= FD_END; i++){
        currentProcess->file_desc_table[i].file_operations_table_ptr = NULL;
        currentProcess->file_desc_table[i].inode = -1;
        currentProcess->file_desc_table[i].file_pos = 0;
        currentProcess->file_desc_table[i].flags = 0;
    }
    /* clear args */
    memset((void*)currentProcess->args, '\0', strlen((const int8_t*)currentProcess->args));

    /*preparing assembly*/
    tss.ss0 = KERNEL_DS;
    tss.esp0 = ((uint32_t)parentPCB) + EIGHT_KB - 4; // -4 because of stack convention

    /*closing a process*/
    currentProcess->active = 0;
    paging_tear_down(parentPCB->pid);

    /* checking to see if fish is running on a terminal, if so, don't teardown */
    process_info_t* allTerminals = ret_multi_process();

    if(allTerminals[0].fish == 0 && allTerminals[1].fish == 0 && allTerminals[2].fish == 0){
        user_vidmem_teardown();
    }
    /* reset fish flag */
    running_terminal->fish = 0;
    /* save ebp/esp of parent */
    uint32_t ebp = parentPCB->saved_ebp;
    uint32_t esp = parentPCB->saved_esp;
    /*updating global variables*/
    currrentProcessNum = parentPCB->pid;
    currentProcess = parentPCB;
    running_terminal->cur_process = parentPCB;

    /*go to asm*/
    halt_asm(ebp, esp, status);
    sti();
    return 0;
}

/*
schedule
    DESCRIPTION:    Schedules the processes that are running.
    INPUTS:         
    OUTPUTS:
    RETURN:
    SIDE-EFFECT:    Changes the running process. Alludes to multi-core.
*/
void schedule() {
    cli();
    /*if a process has been ran, save info to stack*/
    if(currrentProcessNum != -1){
        uint32_t interrupted_ebp;
        uint32_t interrupted_esp;

        /* save current ebp/esp */
        asm volatile(
            "movl %%esp, %0 ;"
            "movl %%ebp, %1 ;"
            : "=r" (interrupted_esp), "=r" (interrupted_ebp)
        );
        /* save ebp/esp into current process */
        currentProcess->saved_ebp = interrupted_ebp;
        currentProcess->saved_esp = interrupted_esp;
    }
    /*update the running terminal*/
    int nextTermNum = ((running_terminal->process_id) + 1)% 3;
    int offset = 0;
    if(nextTermNum == 0)
        offset = -2;
    else
        offset = 1;
    running_terminal = (running_terminal + offset);
    
    /*if no process is being exeuted on next terminal, run shell*/
    uint32_t ebp;
    uint32_t esp;
    if(running_terminal->cur_process == NULL){
        clear();
        execute((const uint8_t*)"shell");
    }
    /*if there is, set up the context switch for it*/
    else{
        currentProcess = running_terminal->cur_process;
        currrentProcessNum = running_terminal->cur_process->pid;
        paging_set_up(running_terminal->cur_process->pid);
        ebp = running_terminal->cur_process->saved_ebp;
        esp = running_terminal->cur_process->saved_esp;
        asm volatile(
            "movl %0, %%esp ;"
            "movl %1, %%ebp ;"
            :
            : "r" (esp), "r" (ebp)
        );
        tss.ss0 = KERNEL_DS;
        tss.esp0 = ((uint32_t)(running_terminal->cur_process)) + EIGHT_KB - 4;
    }
    sti();
    /* return */
    asm volatile(
        "leave  ;"
        "ret    ;"
    );
}

/* 
get_scheduled_term
    DESCRIPTION:    Returns the scheduled terminal's index.
    INPUT:
    OUTPUT:
    RETURN:         scheduled_terminal
    SIDE-EFFECT:    Returns the index of the scheduled terminal.
*/
int get_scheduled_term(){
    return scheduled_terminal;
}

/*
process_allocate
    DESCIPTION: Find an empty bitmap space and allocate the process to it
    INPUTS: 
    OUTPUTS:
    RETURN:     bitmap_pos: the allocated bitmap position for success,
                -1: for fail
 */
int32_t process_allocate(void){

    /* bitmap position */
    int bitmap_pos = 0;
    while(processBitmap[bitmap_pos] == 1){
        bitmap_pos++;
        /* return -1 for no room */
        if(bitmap_pos == 6){        // 6 because there are 6 possible processes
            return -1;
        }
    }
    /* return pid */
    processBitmap[bitmap_pos] = 1;
    return bitmap_pos;
}

/*  process_free(int32_t pid)
 *  DESCIPTION: free the process in the bitmap based on its pid
 *  INPUT: int32_t pid - process index
 *  OUTPUT: none
 *  RETURN: the allocated bitmap position for success,
 *          -1 for fail
 */
int32_t process_free(int32_t pid){
    
    /* fail if double free */
    if(processBitmap[pid] == 0){
        return -1;
    }

    /* return 0 for success */
    processBitmap[pid] = 0;
    return 0;   
}

/*
execute
    DESCRIPTION:    Runs the specified process.
    INPUTS:         command -- Name of process to execute 
    OUTPUTS:    
    RETURN VALUE:
    EFFECT:         Runs the specified process
*/
int32_t execute (const uint8_t* command) {
    cli();
    if(currrentProcessNum < 2){
        change_background_color(running_terminal->process_id);
    }
    if (command == NULL) {
        sti();
        return -1;
    }    
    // 1 - PARSE ARGS
    int i;
    int j = 0;
    uint32_t command_len = strlen((int8_t*)command);
    uint8_t filename[MAX_FILE_NAME_SIZE];
    uint8_t args[128];

    /* clear args */
    memset((void*)args, '\0', strlen((const int8_t*)args));

    //variables used to parse 
    int command_end = 0;
    int char_found = 0;
    int arg_end = 0;
    int charcount = 0;

    /* grab the command and args */
    for (i = 0; i < command_len; i++) {
        //if you see a letter and we haven't read the full command
        if (command[i] != ' ' && command_end == 0) {
            filename[charcount++] = command[i];
            char_found = 1;
        }
        //if we see a space after reading the command
        else if (command[i] == ' ' && char_found == 1 && command_end == 0) {
            j = -1;
            command_end = 1;
            continue;
        }
        // if we see another letter after reading the command
        else if (command[i] != ' ' && char_found == 1 && command_end == 1) {
            j++;
            args[j] = command[i];
            arg_end = j + 1;
        }
        //skip all spaces in between
        else {
            continue;
        }
    } 
    //add null terminating char to the end
    filename[charcount] = '\0';
    args[arg_end] = '\0';

    // used to check if there were arguments present and the command wasnt grep or cat
    int grepCheck = strncmp((const int8_t*)"grep", (const int8_t*)filename, strlen((const int8_t*)filename));
    int catCheck = strncmp((const int8_t*)"cat", (const int8_t*)filename, strlen((const int8_t*)filename));
    int argCheck = strlen((const int8_t*)args);

    // fail if args are present and command is not grep or cat
    if ((grepCheck != 0 && catCheck != 0) && argCheck != 0) {
        return -1;
    }

    // if command is too long fail
    if (strlen((const int8_t*)filename) > 32) {
        return -1;
    }

    // 2 - TYPE/ENTRY POINT
    uint8_t magic_num[4];       // there are 4 magic numbers to consider
    uint8_t entry_point[4];     // there are 4 entry points to consider
    dir_entry_t temp_dentry;
    if (read_dentry_by_name(filename, &temp_dentry) == -1)
        return -1;
    read_data(temp_dentry.inode, 0, magic_num, 4);  // read 4 for the magic numbers

    // check that it's an executable - 0: 0x7f; 1: 0x45; 2: 0x4c; 3: 0x46
    if (magic_num[0] != MAGIC_NUM_1 || magic_num[1] != MAGIC_NUM_2 ||
            magic_num[2] != MAGIC_NUM_3 || magic_num[3] != MAGIC_NUM_4) {
        sti();
        return -1;
    }

    // we've validated everything so increment current process num b/c we're starting a new process
    int32_t possibleProcess = process_allocate();
    if(possibleProcess == -1){
        sti();
        return -1;
    }

    /* update the newest process index with its page */
    newProcess = possibleProcess;
    // 3 - PAGING
    paging_set_up(newProcess);

    pcb_t* pcb_ptr = get_pcb(newProcess);

    // set up stdin and stdout in fdt --> has to do with terminal (need help from Jose)
    int terminal_num = current_terminal_num();

    /* the first three shells */
    if(newProcess < 3){
        pcb_ptr->pid = newProcess;
        pcb_ptr->parent_id = -1;
        pcb_ptr->terminal_id = newProcess;
    }

    /* not the first three shells */
    else{
        pcb_ptr->pid = newProcess;
        pcb_ptr->terminal_id = terminal_num;
        pcb_ptr->parent_id = currrentProcessNum;
    }

    /*initalizing the file descriptor table*/
    for (i = FD_START; i <= FD_END; i++) {
        /*stdin location*/
        if (i == 0){
            pcb_ptr->file_desc_table[i].file_operations_table_ptr = &stdin_fp; //stdin_fp
            pcb_ptr->file_desc_table[i].inode = 0;
            pcb_ptr->file_desc_table[i].file_pos = 0;
            pcb_ptr->file_desc_table[i].flags = 1;
        }
        /*stdout location*/
        else if (i == 1){
            pcb_ptr->file_desc_table[i].file_operations_table_ptr = &stdout_fp; //stdout_fp
            pcb_ptr->file_desc_table[i].inode = 0;
            pcb_ptr->file_desc_table[i].file_pos = 0;
            pcb_ptr->file_desc_table[i].flags = 1;
        }
        else{
            pcb_ptr->file_desc_table[i].file_operations_table_ptr = NULL;
            pcb_ptr->file_desc_table[i].inode = -1;
            pcb_ptr->file_desc_table[i].file_pos = 0;
            pcb_ptr->file_desc_table[i].flags = 0;
        }
    }

    // 4 - FILE TO PAGING
    // copy file data into page
    
    uint32_t file_data_len = (((inode_t*)(inode_start + temp_dentry.inode))->len_in_B);
    read_data(temp_dentry.inode, 0, (uint8_t*)(_128MB_PLUS_OFFSET), file_data_len);//4194304

    // 5 - SET UP STACK
    //filling out the rest of the pcb struct
   
    pcb_ptr->active = 1;
    strncpy((int8_t*)(pcb_ptr->args), (int8_t*)args, strlen((int8_t*)args));

    // set TSS
    tss.ss0 = KERNEL_DS;
    tss.esp0 = ((uint32_t) pcb_ptr) + EIGHT_KB - 4;        // -4 because of stack convention

    // user_ds <- USER_DS need to push to stack and also update one of the values 
    // user_esp <- should be set to 132MB (bottom )
    // user_cs <- USER_CS
    // prog_eip <- entry point (parameter)

    // push iret context to stack
    // call iret --> done
    /*Read from byte 24 to 27, 4 bytes*/
    read_data(temp_dentry.inode, 24, entry_point, 4);   // read 4 because of the 4 entry points
    
    uint32_t eip = (uint32_t)(*((uint32_t*) entry_point));

    uint32_t ds = USER_DS;
    uint32_t cs = USER_CS;
    uint32_t esp = _132MB;
    /*checking if fish was just executed*/
    int fishCheck = strncmp((const int8_t*)"fish", (const int8_t*)filename, strlen((const int8_t*)filename));
    if(fishCheck == 0){
        running_terminal->fish = 1;
    }
    /*saving process we stopped */
    if (currrentProcessNum != -1) {
        pcb_t* activeProcess = get_pcb(currrentProcessNum);
        uint32_t saved_esp;
        uint32_t saved_ebp;
        asm volatile(
            "movl %%esp, %0 ;"
            "movl %%ebp, %1 ;"
            : "=r" (saved_esp), "=r" (saved_ebp)
            
        );
        activeProcess->saved_ebp = saved_ebp;
        activeProcess->saved_esp = saved_esp;
    }
    /*updating globals*/
    currrentProcessNum = newProcess;
    currentProcess = pcb_ptr;
    running_terminal->cur_process = currentProcess;
    
    sti();
    //push_iret_context(eip);
    asm volatile (
        "pushl %0;"
        "pushl %1;"
        "pushfl;"
        "popl %%eax;"
        "orl $0x200, %%eax;"        
        "pushl %%eax;"
        "pushl %2;"
        "pushl %3;"
        "iret;"
        :
        :"r"(ds), "r"(esp), "r"(cs), "r"(eip)
        :"%eax", "%cc");
    // the hex 200 is to clear eax    

    return 0; // won't ever reach this cuz iret
}

/*
read
    DESCRIPTION:    Dispatches the appropriate read
    INPUTS:         fd -- FD index to find
                    buf -- Read from
                    nbytes -- Length to read
    OUTPUTS:    
    RETURN VALUE:
    EFFECT:         Routes to appropriate read
*/
int32_t read(int32_t fd, void* buf, int32_t nbytes) {

    /*Please check all the following functions with a TA*/
    /*
        They makes sense to me since these functions are just suppose to be dispatchers
        Just check to see if the entry exists and modify accordingly then go to actual function
    */
    if(buf == NULL || nbytes < 0 || (fd < FD_START || fd > FD_END))
        return -1;
    pcb_t* curPCB = running_terminal->cur_process;
    if(curPCB->file_desc_table[fd].flags == 0)
        return -1; //Can't write to function that doesn't exist

    return curPCB->file_desc_table[fd].file_operations_table_ptr->read(fd, buf, nbytes);
}

/*
write
    DESCRIPTION:    Dispatches the appropriate write
    INPUTS:         fd -- FD index to find
                    buf -- write to
                    nbytes -- Length to read
    OUTPUTS:    
    RETURN VALUE:
    EFFECT:         Routes to appropriate write
*/
int32_t write (int32_t fd, const void* buf, int32_t nbytes) {
    /*Please check with TA*/

    /*All conditions for errors, double check with TA if there's more*/
    if(buf == NULL || nbytes < 0 || (fd < FD_START || fd > FD_END))
        return -1;
    pcb_t* curPCB = running_terminal->cur_process;
    if(curPCB->file_desc_table[fd].inode == -1)
        return -1; //Can't write to function that doesn

    return curPCB->file_desc_table[fd].file_operations_table_ptr->write(fd, buf, nbytes);
}

/*
open
    DESCRIPTION:    Dispatches the appropriate open
    INPUTS:         fd -- FD index to find
                    buf -- open from
                    nbytes -- Length to open
    OUTPUTS:    
    RETURN VALUE:
    EFFECT:         Routes to appropriate read
*/
int32_t open (const uint8_t* filename) {
    int i;
    if(strlen((int8_t*)filename) == 0)
        return -1;

    dir_entry_t dentry_for_file;
    if (read_dentry_by_name((const uint8_t*)filename, &dentry_for_file) == -1)
        return -1;
    pcb_t* curPCB = running_terminal->cur_process;

    // find an open spot in the table (skipping stdin and stdout)
    /*^^^ correct */
    /*Plus 2 bc I made FD_START 0*/
    for(i = FD_START + 2; i <= FD_END; i++) {
        if (curPCB->file_desc_table[i].inode == -1) {
            /*Mark as occupied*/
            curPCB->file_desc_table[i].flags = 1;
            curPCB->file_desc_table[i].inode = dentry_for_file.inode;

            /*Check this line with TA*/
            curPCB->file_desc_table[i].file_pos = 0;

            /*Storing the correct jumptable based on file type*/
            if(dentry_for_file.file_type == 0){
                curPCB->file_desc_table[i].file_operations_table_ptr = &rtc_fp;
                curPCB->file_desc_table[i].file_operations_table_ptr->open(filename);
            }
            else if(dentry_for_file.file_type == 1)
                curPCB->file_desc_table[i].file_operations_table_ptr = &dir_fp;
            else
                curPCB->file_desc_table[i].file_operations_table_ptr = &file_fp;
            
            return i; // returning index where it was stored
        }
    }
    return -1; //if you can't find an open spot return -1
}

/*
close
    DESCRIPTION:    Dispatches the appropriate close
    INPUTS:         fd -- FD index to find
    OUTPUTS:    
    RETURN VALUE:
    EFFECT:         Routes to appropriate close
*/
int32_t close (int32_t fd) {

    /*Please get this checked out with a TA but I'm pretty sure this is all you need to do.*/
    if (fd < FD_START+2 || fd > FD_END)
    if (fd < FD_START + 2 || fd > FD_END)
        return -1;
    
    pcb_t* curPCB = running_terminal->cur_process;
    if(curPCB->file_desc_table[fd].inode == -1 || curPCB->file_desc_table[fd].flags == 0)

    if(curPCB->file_desc_table[fd].inode == -1 || curPCB->file_desc_table[fd].flags == 0)
        return -1; // inode tell us if the space is occupied or not. You can't close a unoccupied spot

    // set this spot as open
    curPCB->file_desc_table[fd].flags = 0;
    curPCB->file_desc_table[fd].file_pos = 0;
    curPCB->file_desc_table[fd].inode = -1;

    return curPCB->file_desc_table->file_operations_table_ptr->close(fd);
}

/*
getargs
    DESCRIPTION:    Dispatches the appropriate close
    INPUTS:         buf -- Buffer to hold data
                    nbytes -- Length to read
    OUTPUTS:    
    RETURN VALUE:
    EFFECT:         Routes to appropriate close
*/
int32_t getargs (uint8_t* buf, int32_t nbytes) {
    if(buf == NULL || nbytes < 0)
        return -1;
    pcb_t* pcbPtr = running_terminal->cur_process;
    strncpy((int8_t*)buf, (const int8_t*)pcbPtr->args, nbytes);
    
    //if no arguments present, fail
    if(*buf == NULL)
        return -1;
    return 0; // checkpoint 4
}

/*
vidmap
    DESCRIPTION:    Dispatches the appropriate close
    INPUTS:         screen_start
    OUTPUTS:    
    RETURN VALUE:
    EFFECT:         Routes to appropriate close
*/
int32_t vidmap (uint8_t** screen_start) {
    /*Don't forget to tear the paging down in HALT!!!*/
    /*Someone else do it please*/
    if(screen_start == NULL)
        return -1;
    /*checks to see if pointer is inside user mem space*/
    uint32_t addr = (uint32_t)screen_start;
    if(addr < _128MB || addr > _128MB + FOUR_MB)
        return -1;
    /*setting up paging for user_vid_mem*/
    user_vidmem_setup();
    /*make double pointer point to page directory entry point*/
    *screen_start = (uint8_t*)0x8800000;
    /*double check if we return this or not*/
    return (uint32_t)screen_start; // checkpoint 4
}

/*
set_handler
    DESCRIPTION:    Dispatches the appropriate close
    INPUTS:         signum
                    handler_address
    OUTPUTS:    
    RETURN VALUE:
    EFFECT:         Routes to appropriate close
*/
int32_t set_handler (int32_t signum, void* handler_address) {
    return 0;
}

/*
sigreturn
    DESCRIPTION:    Dispatches the appropriate close
    INPUTS:    
    OUTPUTS:    
    RETURN VALUE:
    EFFECT:         Routes to appropriate close
*/
int32_t sigreturn (void) {
    return 0;
}

/*
get_pcb
    DESCRIPTION:    Calculates the PCB struct's address.
    INPUTS:         cur_process_num -- Offset for address logic
    OUTPUTS:    
    RETURN VALUE:
    EFFECT:         Grabs ptr to address to PCB struct
*/
pcb_t* get_pcb(int getPcbCurrentProcessNum) {
    return (pcb_t*) (EIGHT_MB - EIGHT_KB * (getPcbCurrentProcessNum + 1)); // 8kb is 0x2000
}

/*
funcPointers_int
    DESCRIPTION:    Initializes the function pointers
    INPUTS:         
    OUTPUTS:    
    RETURN VALUE:
    EFFECT:         Initializes the function pointers
*/
void funcPointers_int(void){
    stdin_fp.open = terminal_open;
    stdin_fp.close = terminal_close;
    stdin_fp.read = terminal_read;
    stdin_fp.write = ret_neg_1;

    stdout_fp.open = terminal_open;
    stdout_fp.close = terminal_close;
    stdout_fp.read = ret_neg_1;
    stdout_fp.write = terminal_write;

    rtc_fp.open = rtc_open;
    rtc_fp.close = rtc_close;
    rtc_fp.read = rtc_read;
    rtc_fp.write = rtc_write;

    file_fp.open = file_sys_open;
    file_fp.close = file_sys_close;
    file_fp.read = file_sys_read;
    file_fp.write = file_sys_write;

    /*fill these out with directory open, close, etc, bc I couldn't find them*/
    dir_fp.open = open_directory;
    dir_fp.close = close_directory;
    dir_fp.read = read_directory;
    dir_fp.write = write_directory;  
}

/*
get_pcb_idx
    DESCRIPTION:    Grabs the current process's ID.
    INPUTS:         
    OUTPUTS:    
    RETURN VALUE:   cur_process_num -- Int that is the ID
    EFFECT:         Calculates ID for process.
*/
int get_pcb_idx(){
    return currrentProcessNum;
}

/*
ret_neg_1
    DESCRIPTION:    Returns -1
    INPUTS:         
    OUTPUTS:    
    RETURN VALUE:   -1
    EFFECT:         
*/
int ret_neg_1() {
    return -1;
}
