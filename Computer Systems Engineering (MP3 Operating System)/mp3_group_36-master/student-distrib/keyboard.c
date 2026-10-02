# include "keyboard.h"


/*will use the scanCode the computer sends to index into this array*/
/*0x0 indicates special keys*/
char codeToChar[MAX_PRINTABLE][SHIFTABLE] =  // add another dim for the shifted verisons later
{{0x0, 0x0}, {0x0, 0x0}, {'1', '!'}, {'2', '@'}, // scancodes start at 1 so I put a blank at the beginning, then it's the esc key
 {'3', '#'}, {'4', '$'}, {'5', '%'}, {'6', '^'}, 
 {'7', '&'}, {'8', '*'}, {'9', '('}, {'0', ')'}, 
 {'-', '_'}, {'=', '+'}, {'\b', '\b'}, 
 {' ', ' '}, {'q', 'Q'}, {'w', 'W'}, {'e', 'E'}, // this first empty is used for tab
 {'r', 'R'}, {'t', 'T'}, {'y', 'Y'}, {'u', 'U'}, 
 {'i', 'I'}, {'o', 'O'}, {'p', 'P'}, {'[', '{'}, 
 {']', '}'}, {NEWLINE_CHAR, NEWLINE_CHAR}, {0x0, 0x0}, //left control key
 {'a', 'A'}, {'s', 'S'}, {'d', 'D'}, {'f', 'F'}, 
 {'g', 'G'}, {'h', 'H'}, {'j', 'J'}, {'k', 'K'}, 
 {'l', 'L'}, {';', ':'}, {'\'', '"'}, {'`', '~'}, // "\'" will print as '
 {0x0, 0x0}, {'\\', '|'}, {'z', 'Z'}, {'x', 'X'}, // left shift, "\\" will print as "\"
 {'c', 'C'}, {'v', 'V'}, {'b', 'B'}, {'n', 'N'}, 
 {'m', 'M'}, {',', '<'}, {'.', '>'}, {'/', '?'},
 {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {' ', ' '}, {0x0, 0x0}}; // right shift, keypad *, left alt, space, capslock

int totalNums = 0; // keeps track of how many entries are in the buffer
int lShiftFlag = 0; // flag used to indicate left shift pressed or not
int rShiftFlag = 0; // flag used to indicate right shift pressed or not
int capsLockFlag = 0; // flag used to indicate caps lock pressed or not
int ctrlFlag = 0; // flag used to indicate ctrl pressed or not
int altFlag = 0; // flag used to indicate if alt is pressed or not

/*
*   keyboard_init()
* Description: Enables Keyboard by connecting it to the PIC
* and connecting to a port to send interupts 
* Input: None
* Output: None
* Return Value: None
* Side Effect: Connects Keyboard to IRQ 1 on the PIC
*/
void keyboard_init(void){
    enable_irq(KEYBOARD_IRQ);
}

/*
*   keyboard_handler()
* Description: Takes the scancode sent by the keyboard and
* prints the corresponding character onto the screen
* Input: None
* Output: None
* Return Value: None
* Side Effect: Prints character onto the screen
*/
void keyboard_handler(void){
    // mask interupts
    int i;
    uint32_t flags;
    cli_and_save(flags);
    /*Get scancode from the keyboard*/
    uint8_t scanCode = inb(KEYBOARD_DATA_PORT);
    /*Checks if right ctrl or right alt was pressed*/
    if(scanCode == 0xE0){
        scanCode = inb(KEYBOARD_DATA_PORT);
    }

    /*checks to see if key pressed was special*/
    if(special_key_pressed(scanCode)){
        send_eoi(KEYBOARD_IRQ);
        restore_flags(flags);
        return;
    }
    int terminal_num = current_terminal_num();
    process_info_t* multi_process = ret_multi_process();
    process_info_t* runningTerminal = ret_running_term();
    if(scanCode < MAX_PRINTABLE && scanCode > 1){ // ignores key releases and don't print if over max
       /*if clear cmd is sent then clear the screen and move the cursor*/
        if(scanCode == L_PRESS && ctrlFlag){
            video_mem_remap(terminal_num);
            clear();
            video_mem_remap(runningTerminal->process_id);
            multi_process[terminal_num].charCount = 0;
        }
        /*Here would be the logic to switch terminals, probably involves pcb*/
        else if((scanCode == F1_PRESSED || scanCode == F2_PRESSED || scanCode == F3_PRESSED) && altFlag){
            // int process_num = get_pcb_idx();
            switch(scanCode){
                case F1_PRESSED:
                    switch_terminal(0);
                    break;
                case F2_PRESSED: 
                    switch_terminal(1);
                    break;
                case F3_PRESSED:
                    switch_terminal(2);
                    break;
                default:
                    return;
            }
        }

        /*If backspace is pressed, decrement number of chars in buffer and putc takes care of rest*/
        else if(scanCode == BACKSPACE_PRESS){
            if(multi_process[terminal_num].charCount == 0){
                send_eoi(KEYBOARD_IRQ);
                restore_flags(flags);
                return;
            }
            multi_process[terminal_num].charCount--;
            put_on_visable_term(codeToChar[scanCode][0]);
            getc(codeToChar[scanCode][0], multi_process[terminal_num].charCount);
            
        }
        /*when tab is pressed, print a space 4 times*/
        else if(scanCode == TAB_PRESS){
            for(i = 0; i < 4; i++){
                put_on_visable_term(codeToChar[scanCode][0]);
                getc(codeToChar[scanCode][0], multi_process[terminal_num].charCount);
                multi_process[terminal_num].charCount++;
            }
        }
        /*checks if number is printable, then prints it*/
        else if((((scanCode >= NUMS_LOWER_LIMIT) && (scanCode <= NUMS_UPPER_LIMIT)) ||
           ((scanCode >= FIRST_ROW_LOWER) && (scanCode <= FIRST_ROW_UPPER)) ||
           ((scanCode >= SECOND_ROW_LOWER) && (scanCode <= SECOND_ROW_UPPER)) ||
           ((scanCode >= THIRD_ROW_LOWER) && (scanCode <= THIRD_ROW_UPPER)) ||
           (scanCode == SPACE_PRESS) || (scanCode == ENTER_PRESS))){
            /*checks whether to print shifted version or not and prints accordingly*/
            if(capsLockFlag ^ (lShiftFlag | rShiftFlag)){
                put_on_visable_term(codeToChar[scanCode][1]);
                getc(codeToChar[scanCode][1], multi_process[terminal_num].charCount);
            }
            else{
                put_on_visable_term(codeToChar[scanCode][0]);
                getc(codeToChar[scanCode][0], multi_process[terminal_num].charCount);
            }
            /*Resets number of chars in buffer if enter was pressed*/
            if(scanCode == ENTER_PRESS)
                multi_process[terminal_num].charCount = 0;
            else
                multi_process[terminal_num].charCount++;
        }
    }
    
    // multi_process[terminal_num].charCount = totalNums
    /*tell PIC interupt is done*/
    send_eoi(KEYBOARD_IRQ);
    /*unmask interupts*/
    restore_flags(flags);
}

/*
*   special_key_pressed()
* Description: Takes the scancode sent by the keyboard and
* checks to see if it was a special key and sets the correct flag
* Input: uint8_t scanCode, Code sent in by keyboard
* Output: None
* Return Value: 1 if special key was pressed, 0 otherwise
* Side Effect: Sets special key flags
*/
int special_key_pressed(uint8_t scanCode){
    switch(scanCode){
        case RIGHT_SHIFT_PRESS:
            rShiftFlag = 1;
            break;
        case LEFT_SHIFT_PRESS:
            lShiftFlag = 1;
            break;
        case CTRL_PRESS:
            ctrlFlag = 1;
            break;
        case ALT_PRESS:
            altFlag = 1;
        case (RIGHT_SHIFT_PRESS | RELEASED):
            rShiftFlag = 0;
            break;
        case (LEFT_SHIFT_PRESS | RELEASED):
            lShiftFlag = 0;
            break;
        case (CTRL_PRESS | RELEASED):
            ctrlFlag = 0;
            break;
        case (ALT_PRESS | RELEASED):
            altFlag = 0;
            break;
        case CAPS_LOCK_PRESS:
           capsLockFlag = !capsLockFlag;
           break;
        default:
            return 0;
    }
    return 1;
}

