#include "terminal.h"

char charBuffer[BUFFER_SIZE] = {' '};
int visualTerminal = 0;
process_info_t multi_process[3];
process_info_t* running_terminal = multi_process;

/*
ret_multi_process
    DESCRIPTION:    returns pointer to array of terminal structs
    INPUTS:         
    OUTPUTS:
    RETURN:
    SIDE-EFFECT:    None 
*/
process_info_t* ret_multi_process(){
    return multi_process;
}


/*
*   terminal_open()
* Description: opens the terminal "Does nothing"
* Input: fd -- keeps track of which terminal is open
*        buf -- buffer used to hold chars
*        nbytes -- num of bytes we want to use
* Output: None
* Return Value: 0
* Side Effect: Nothing
*/
int32_t terminal_open(const uint8_t* filename){
    return 0;
}

/*
*   terminal_close()
* Description: closes the terminal "Does nothing"
* Input: fd -- keeps track of which terminal is open
*        buf -- buffer used to hold chars
*        nbytes -- num of bytes we want to use
* Output: None
* Return Value: 0
* Side Effect: Nothing
*/
int32_t terminal_close(int32_t fd){
    /*look for fd bounding conditions */
    // return -1;
    return 0;
}

/*
*   terminal_read()
* Description: Reads inputs from the keyboard up to the amount specified by nbytes
* Input: fd -- keeps track of which terminal is open
*        buf -- buffer used to hold chars
*        nbytes -- num of bytes we want to use
* Output: None
* Return Value: 0 if bytes is positive, -1 otherwise
* Side Effect: fills in keyboard buffer
*/
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes){
    uint32_t totalBytes, i;

    if(nbytes > BUFFER_SIZE) // if over the buffer size, set to max
        nbytes = BUFFER_SIZE;
    else if(nbytes < 0 || buf == NULL){ //STUPID ERROR FUCK ECE391
        return -1; // no bytes read/failed
    }
    sti();
    while(multi_process[running_terminal->process_id].enterFlag == 0) {} // waiting until enter is pressed, idk if correct
    cli();
    /*loop through nbytes or until a newline char is seen*/
    for(i = 0; i < nbytes; i++){
        /*have to typecast to make sure a char is in array and not a random number*/
        (((char*) buf)[i]) = multi_process[visualTerminal].keyboard_buf[i]; //idk if correct
        totalBytes = i;
        if(multi_process[visualTerminal].keyboard_buf[i] == '\n'){
            totalBytes = (i+1);
            break;    
        }
    }
    /*reset enter to 0*/
    multi_process[visualTerminal].enterFlag = 0;
    sti();
    return totalBytes;
}
/*
*   terminal_write()
* Description: Writes from buf to screen up to the amount specified by nbytes
* Input: fd -- keeps track of which terminal is open
*        buf -- buffer used to hold chars
*        nbytes -- num of bytes we want to use
* Output: None
* Return Value: 0 if bytes is positive, -1 otherwise
* Side Effect: Writes to the screen
*/
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes){
    cli();
    uint32_t totalBytes, i;
    if(nbytes < 0 || buf == NULL){ // if asking for less than 0, fail
        sti();
        return -1; // failed
    }
    /*loop through requested size*/
    for(i = 0; i < nbytes; i++){
        /*put write_buf to screen and updating bytes written*/
        if(visualTerminal == running_terminal->process_id)
            put_on_visable_term(((uint8_t*) buf)[i]);
        else
            putc(((uint8_t*) buf)[i]);
        totalBytes = (i+1);
    }
    /*clear buffer at the end*/
    memset(charBuffer, '\0', strlen(charBuffer));
    memset((void*)multi_process[visualTerminal].keyboard_buf, '\0', strlen(charBuffer));
    sti();
    
    return totalBytes;
}

/*
*   getc()
* Description: Reads from the keyboard and fills up the keyboard buffer with string to display
* Input: c -- Character we want to store in the buffer
         index -- index in buffer we want to place the character in
* Output: None
* Return Value: None
* Side Effect: stores C in the keyboard buffer
*/
void getc(char c, int index){
    /*don't accept more inputs than the max size*/
    if(index >= BUFFER_SIZE){ 
        if(c == '\n'){
            multi_process[visualTerminal].enterFlag = 1;
        }
        else
            return;
    }
    /*index will be shifted accordingly then put space in */
    else if(c == '\b'){
        multi_process[visualTerminal].keyboard_buf[index] = ' ';
    }
    /*place character into buffer*/
    else{
        multi_process[visualTerminal].keyboard_buf[index] = c;
        if(c == '\n')
            multi_process[visualTerminal].enterFlag = 1;
    }
}

/*
switch_terminal
    DESCRIPTION:    Switches the terminal.
    INPUTS:         terminal_id
    OUTPUTS:
    RETURN:
    SIDE-EFFECT:    Copies video memory to the old terminal's own video memory page
                    and copies from the new terminal's own video memory page into
                    video memory, updates cursor
*/
void switch_terminal(int terminal_id){
    cli();
    if(terminal_id == visualTerminal){
        sti();
        return;
    }
    /* grabbing terminal we are going to*/
    process_info_t* new_terminal = &(multi_process[terminal_id]);
    /*getting video offsets for the 2 terminals*/
    int new_source = (PAGE_TABLE_VIDEO_MEM_OFFSET + (terminal_id + 1))*_4KB;
    int old_source = (PAGE_TABLE_VIDEO_MEM_OFFSET + (visualTerminal + 1)) * _4KB;
    /*copying data from new terminal page into video memry*/
    video_mem_remap(visualTerminal);
    memcpy((void*)old_source, (const void*)(PAGE_TABLE_VIDEO_MEM_OFFSET*_4KB), _4KB);
    /*copying data from new terminal page into video memry*/
    memcpy((void*)(PAGE_TABLE_VIDEO_MEM_OFFSET*_4KB), (const void*)new_source, _4KB);
    video_mem_remap(running_terminal->process_id);
    /*remap fish user page */
    fish_remap(terminal_id);
    /*changing background*/
    change_background_color(terminal_id);
    /*updating cursor*/
    int screenX = (new_terminal->screen_x);
    int screenY = (new_terminal->screen_y);
    update_cursor(screenX, screenY);
    /*updating visual terminal*/
    visualTerminal = terminal_id;
}

/*
terminal_init
    DESCRIPTION:    Initalizes the 3 terminal structs.
    INPUTS:         
    OUTPUTS:
    RETURN:
    SIDE-EFFECT:    initalizes all elements 
*/
void terminal_init(){
    int i;
    for(i = 0; i < 3; i++){
        multi_process[i].screen_x = 0;
        multi_process[i].screen_y = 0;
        multi_process[i].charCount = 0;
        multi_process[i].enterFlag = 0;
        multi_process[i].cur_process = NULL;
        multi_process[i].fish = 0;
        multi_process[i].rtc_max = 0;
        multi_process[i].process_id = i;
    }
}
/*
current_terminal_num
    DESCRIPTION:    returns the ID of the terminal we see
    INPUTS:         
    OUTPUTS:
    RETURN:
    SIDE-EFFECT:    None 
*/
int current_terminal_num(){
    return visualTerminal;
}
/*
ret_running_term
    DESCRIPTION:    returns terminal struct of scheduled terminal
    INPUTS:         
    OUTPUTS:
    RETURN:
    SIDE-EFFECT:    None 
*/
process_info_t* ret_running_term(){
    return running_terminal;
}

