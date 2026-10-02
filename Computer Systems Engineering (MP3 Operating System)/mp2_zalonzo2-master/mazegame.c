/*
 * tab:4
 *
 * mazegame.c - main source file for ECE398SSL maze game (F04 MP2)
 *
 * "Copyright (c) 2004 by Steven S. Lumetta."
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 * 
 * IN NO EVENT SHALL THE AUTHOR OR THE UNIVERSITY OF ILLINOIS BE LIABLE TO 
 * ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL 
 * DAMAGES ARISING OUT  OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, 
 * EVEN IF THE AUTHOR AND/OR THE UNIVERSITY OF ILLINOIS HAS BEEN ADVISED 
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE AUTHOR AND THE UNIVERSITY OF ILLINOIS SPECIFICALLY DISCLAIM ANY 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE 
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND NEITHER THE AUTHOR NOR
 * THE UNIVERSITY OF ILLINOIS HAS ANY OBLIGATION TO PROVIDE MAINTENANCE, 
 * SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS."
 *
 * Author:        Steve Lumetta
 * Version:       1
 * Creation Date: Fri Sep 10 09:57:54 2004
 * Filename:      mazegame.c
 * History:
 *    SL    1    Fri Sep 10 09:57:54 2004
 *        First written.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "blocks.h"
#include "maze.h"
#include "modex.h"
#include "text.h"

// New Includes and Defines
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/io.h>
#include <termios.h>
#include <pthread.h>

// ADDED
#include <string.h>
#include "module/tuxctl-ioctl.h"

#define BACKQUOTE 96
#define UP        65
#define DOWN      66
#define RIGHT     67
#define LEFT      68

#define MAX_INT   12        // max amount of char an int can fill up a string
#define MAX_CHARS 40        // max amount of chars that fit in status bar
#define FLOAT_TEXT_SIZE 1664 // 13*8*16 (13 is max length of fruit string, 8 and 16 are width and height of characters)
#define CHAR_WIDTH  8
#define NUM_COLORS 5
#define FLOATING_TEXT_TIME 5

/*
 * If NDEBUG is not defined, we execute sanity checks to make sure that
 * changes to enumerations, bit maps, etc., have been made consistently.
 */
#if defined(NDEBUG)
#define sanity_check() 0
#else
static int sanity_check();
#endif


/* a few constants */
#define PAN_BORDER      5  /* pan when border in maze squares reaches 5    */
#define MAX_LEVEL       10 /* maximum level number                         */

/* outcome of each level, and of the game as a whole */
typedef enum {GAME_WON, GAME_LOST, GAME_QUIT} game_condition_t;

/* structure used to hold game information */
typedef struct {
    /* parameters varying by level   */
    int number;                  /* starts at 1...                   */
    int maze_x_dim, maze_y_dim;  /* min to max, in steps of 2        */
    int initial_fruit_count;     /* 1 to 6, in steps of 1/2          */
    int time_to_first_fruit;     /* 300 to 120, in steps of -30      */
    int time_between_fruits;     /* 300 to 60, in steps of -30       */
    int tick_usec;         /* 20000 to 5000, in steps of -1750 */
    
    /* dynamic values within a level -- you may want to add more... */
    unsigned int map_x, map_y;   /* current upper left display pixel */
} game_info_t;

static game_info_t game_info;

/* local functions--see function headers for details */
static int prepare_maze_level(int level);
static void move_up(int* ypos);
static void move_right(int* xpos);
static void move_down(int* ypos);
static void move_left(int* xpos);
static int unveil_around_player(int play_x, int play_y);
static void *rtc_thread(void *arg);
static void *keyboard_thread(void *arg);

static void *tux_thread(void *arg);

/* 
 * prepare_maze_level
 *   DESCRIPTION: Prepare for a maze of a given level.  Fills the game_info
 *          structure, creates a maze, and initializes the display.
 *   INPUTS: level -- level to be used for selecting parameter values
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: writes entire game_info structure; changes maze;
 *                 initializes display
 */
static int prepare_maze_level(int level) {
    int i; /* loop index for drawing display */
    
    /*
     * Record level in game_info; other calculations use offset from
     * level 1.
     */
    game_info.number = level--;

    /* Set per-level parameter values. */
    if ((game_info.maze_x_dim = MAZE_MIN_X_DIM + 2 * level) > MAZE_MAX_X_DIM)
        game_info.maze_x_dim = MAZE_MAX_X_DIM;
    if ((game_info.maze_y_dim = MAZE_MIN_Y_DIM + 2 * level) > MAZE_MAX_Y_DIM)
        game_info.maze_y_dim = MAZE_MAX_Y_DIM;
    if ((game_info.initial_fruit_count = 1 + level / 2) > 6)
        game_info.initial_fruit_count = 6;
    //game_info.initial_fruit_count = 10;
    if ((game_info.time_to_first_fruit = 300 - 30 * level) < 120)
        game_info.time_to_first_fruit = 120;
    if ((game_info.time_between_fruits = 300 - 60 * level) < 60)
        game_info.time_between_fruits = 60;
    if ((game_info.tick_usec = 20000 - 1750 * level) < 5000)
        game_info.tick_usec = 5000;

    /* Initialize dynamic values. */
    game_info.map_x = game_info.map_y = SHOW_MIN;

    /* Create a maze. */
    if (make_maze(game_info.maze_x_dim, game_info.maze_y_dim, game_info.initial_fruit_count) != 0)
        return -1;
    
    /* Set logical view and draw initial screen. */
    set_view_window(game_info.map_x, game_info.map_y);
    for (i = 0; i < SCROLL_Y_DIM; i++)
        (void)draw_horiz_line (i);

    /* Return success. */
    return 0;
}

/* 
 * move_up
 *   DESCRIPTION: Move the player up one pixel (assumed to be a legal move)
 *   INPUTS: ypos -- pointer to player's y position (pixel) in the maze
 *   OUTPUTS: *ypos -- reduced by one from initial value
 *   RETURN VALUE: none
 *   SIDE EFFECTS: pans display by one pixel when appropriate
 */
static void move_up(int* ypos) {
    /*
     * Move player by one pixel and check whether display should be panned.
     * Panning is necessary when the player moves past the upper pan border
     * while the top pixels of the maze are not on-screen.
     */
    if (--(*ypos) < game_info.map_y + BLOCK_Y_DIM * PAN_BORDER && game_info.map_y > SHOW_MIN) {
        /*
         * Shift the logical view upwards by one pixel and draw the
         * new line.
         */
        set_view_window(game_info.map_x, --game_info.map_y);
        (void)draw_horiz_line(0);
    }
}

/* 
 * move_right
 *   DESCRIPTION: Move the player right one pixel (assumed to be a legal move)
 *   INPUTS: xpos -- pointer to player's x position (pixel) in the maze
 *   OUTPUTS: *xpos -- increased by one from initial value
 *   RETURN VALUE: none
 *   SIDE EFFECTS: pans display by one pixel when appropriate
 */
static void move_right(int* xpos) {
    /*
     * Move player by one pixel and check whether display should be panned.
     * Panning is necessary when the player moves past the right pan border
     * while the rightmost pixels of the maze are not on-screen.
     */
    if (++(*xpos) > game_info.map_x + SCROLL_X_DIM - BLOCK_X_DIM * (PAN_BORDER + 1) &&
        game_info.map_x + SCROLL_X_DIM < (2 * game_info.maze_x_dim + 1) * BLOCK_X_DIM - SHOW_MIN) {
        /*
         * Shift the logical view to the right by one pixel and draw the
         * new line.
         */
        set_view_window(++game_info.map_x, game_info.map_y);
        (void)draw_vert_line(SCROLL_X_DIM - 1);
    }
}

/* 
 * move_down
 *   DESCRIPTION: Move the player right one pixel (assumed to be a legal move)
 *   INPUTS: ypos -- pointer to player's y position (pixel) in the maze
 *   OUTPUTS: *ypos -- increased by one from initial value
 *   RETURN VALUE: none
 *   SIDE EFFECTS: pans display by one pixel when appropriate
 */
static void move_down(int* ypos) {
    /*
     * Move player by one pixel and check whether display should be panned.
     * Panning is necessary when the player moves past the right pan border
     * while the bottom pixels of the maze are not on-screen.
     */
    if (++(*ypos) > game_info.map_y + SCROLL_Y_DIM - BLOCK_Y_DIM * (PAN_BORDER + 1) && 
        game_info.map_y + SCROLL_Y_DIM < (2 * game_info.maze_y_dim + 1) * BLOCK_Y_DIM - SHOW_MIN) {
        /*
         * Shift the logical view downwards by one pixel and draw the
         * new line.
         */
        set_view_window(game_info.map_x, ++game_info.map_y);
        (void)draw_horiz_line(SCROLL_Y_DIM - 1);
    }
}

/* 
 * move_left
 *   DESCRIPTION: Move the player right one pixel (assumed to be a legal move)
 *   INPUTS: xpos -- pointer to player's x position (pixel) in the maze
 *   OUTPUTS: *xpos -- decreased by one from initial value
 *   RETURN VALUE: none
 *   SIDE EFFECTS: pans display by one pixel when appropriate
 */
static void move_left(int* xpos) {
    /*
     * Move player by one pixel and check whether display should be panned.
     * Panning is necessary when the player moves past the left pan border
     * while the leftmost pixels of the maze are not on-screen.
     */
    if (--(*xpos) < game_info.map_x + BLOCK_X_DIM * PAN_BORDER && game_info.map_x > SHOW_MIN) {
        /*
         * Shift the logical view to the left by one pixel and draw the
         * new line.
         */
        set_view_window(--game_info.map_x, game_info.map_y);
        (void)draw_vert_line (0);
    }
}

/* 
 * unveil_around_player
 *   DESCRIPTION: Show the maze squares in an area around the player.
 *                Consume any fruit under the player.  Check whether
 *                player has won the maze level.
 *   INPUTS: (play_x,play_y) -- player coordinates in pixels
 *   OUTPUTS: none
 *   RETURN VALUE: 1 if player wins the level by entering the square
 *                 0 if not
 *   SIDE EFFECTS: draws maze squares for newly visible maze blocks,
 *                 consumed fruit, and maze exit; consumes fruit and
 *                 updates displayed fruit counts
 */
static int unveil_around_player(int play_x, int play_y) {
    int x = play_x / BLOCK_X_DIM; /* player's maze lattice position */
    int y = play_y / BLOCK_Y_DIM;
    int i, j;            /* loop indices for unveiling maze squares */

    /* Check for fruit at the player's position. */
    (void)check_for_fruit (x, y); // WAS (void)check_for_fruit (x, y);

    /* Unveil spaces around the player. */
    for (i = -1; i < 2; i++)
        for (j = -1; j < 2; j++)
            unveil_space(x + i, y + j);
        unveil_space(x, y - 2);
        unveil_space(x + 2, y);
        unveil_space(x, y + 2);
        unveil_space(x - 2, y);

    /* Check whether the player has won the maze level. */
    return check_for_win (x, y);
}

#ifndef NDEBUG
/* 
 * sanity_check 
 *   DESCRIPTION: Perform checks on changes to constants and enumerated values.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0 if checks pass, -1 if any fail
 *   SIDE EFFECTS: none
 */
static int sanity_check() {
    /* 
     * Automatically detect when fruits have been added in blocks.h
     * without allocating enough bits to identify all types of fruit
     * uniquely (along with 0, which means no fruit).
     */
    if (((2 * LAST_MAZE_FRUIT_BIT) / MAZE_FRUIT_1) < NUM_FRUIT_TYPES + 1) {
        puts("You need to allocate more bits in maze_bit_t to encode fruit.");
        return -1;
    }
    return 0;
}
#endif /* !defined(NDEBUG) */

// Shared Global Variables
int quit_flag = 0;
int winner= 0;
int next_dir = UP;
int play_x, play_y, last_dir, dir;
int move_cnt = 0;
int fd;
unsigned long data;
static struct termios tio_orig;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

// ADDED
int fd_tux;
int buttons_pressed = 0;
int button = 0;
static pthread_cond_t cv = PTHREAD_COND_INITIALIZER;



/*
 * keyboard_thread
 *   DESCRIPTION: Thread that handles keyboard inputs
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
static void *keyboard_thread(void *arg) {
    char key;
    int state = 0;
    // Break only on win or quit input - '`'
    while (winner == 0) {        
        // Get Keyboard Input
        key = getc(stdin);
        
        // Check for '`' to quit
        if (key == BACKQUOTE) {
            quit_flag = 1;
            break;
        }
        
        // Compare and Set next_dir
        // Arrow keys deliver 27, 91, ##
        if (key == 27) {
            state = 1;
        }
        else if (key == 91 && state == 1) {
            state = 2;
        }
        else {    
            if (key >= UP && key <= LEFT && state == 2) {
                pthread_mutex_lock(&mtx);
                switch(key) {
                    case UP:
                        next_dir = DIR_UP;
                        break;
                    case DOWN:
                        next_dir = DIR_DOWN;
                        break;
                    case RIGHT:
                        next_dir = DIR_RIGHT;
                        break;
                    case LEFT:
                        next_dir = DIR_LEFT;
                        break;
                }
                pthread_mutex_unlock(&mtx);
            }
            state = 0;
        }
    }

    return 0;
}


/*
 * tux_thread
 *   DESCRIPTION: Thread that handles tux inputs
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
static void *tux_thread(void *arg) {
    // Break only on win
    while (winner == 0) {        

        // quit thread if user pressed ` on keyboard
        if (quit_flag) {
            break;
        }

        // wait for a tux button to be pressed (not polling)
        pthread_mutex_lock(&mtx);
        while (buttons_pressed == 0) {
            pthread_cond_wait(&cv, &mtx);
        }

        // change direction based on tux button
        switch(button) {
            case 239: // tux up
                next_dir = DIR_UP;
                break;
            case 223: // tux down
                next_dir = DIR_DOWN;
                break;
            case 127: // tux right
                next_dir = DIR_RIGHT;
                break;
            case 191: // tux left
                next_dir = DIR_LEFT;
                break;
        }
        pthread_mutex_unlock(&mtx);
    }

    return 0;
}







/* some stats about how often we take longer than a single timer tick */
static int goodcount = 0;
static int badcount = 0;
static int total = 0;

/*
 * rtc_thread
 *   DESCRIPTION: Thread that handles updating the screen
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
static void *rtc_thread(void *arg) {
    int ticks = 0;
    int level;
    int ret;
    int open[NUM_DIRS];
    int goto_next_level = 0;

    int min_tens, min_ones, sec_tens, sec_ones;

    // colors for walls, players, and status bar
    unsigned char red[NUM_COLORS] = {0x3F, 0x2F, 0x00, 0x11, 0x2F};
    unsigned char green[NUM_COLORS] = {0x3F, 0x3F, 0x3F, 0x3F, 0x00};
    unsigned char blue[NUM_COLORS] = {0x00, 0x00, 0x2F, 0x0F, 0x11};
    unsigned char wallRGB[3]; // 3 for red, green, blue
    unsigned char playerRGB[3];
    int player_color = 0;

    // Loop over levels until a level is lost or quit.
    for (level = 1; (level <= MAX_LEVEL) && (quit_flag == 0); level++) {
        // Prepare for the level.  If we fail, just let the player win.
        if (prepare_maze_level(level) != 0)
            break;
        goto_next_level = 0;


        // ADDED
        // reset total ticks
        total = 0;
        // flag for printing floating text
        int print_floating = 0;
        // floating text
        char * found_fruit_str = "             ";
        // how long we've been showing the floating text for
        int float_seconds = 0;
        // center offset for floating text
        int float_offset = 0;
        // set wall color and status bar at the start of each level
        wallRGB[0] = red[level % NUM_COLORS];
        wallRGB[1] = green[level % NUM_COLORS];
        wallRGB[2] = blue[level % NUM_COLORS];
        set_palette(0x22, wallRGB);
        
        int fruit_x = play_x;
        int fruit_y = play_y;


        // Start the player at (1,1)
        play_x = BLOCK_X_DIM;
        play_y = BLOCK_Y_DIM;

        // move_cnt tracks moves remaining between maze squares.
        // When not moving, it should be 0.
        move_cnt = 0;

        // Initialize last direction moved to up
        last_dir = DIR_UP;

        // Initialize the current direction of motion to stopped
        dir = DIR_STOP;
        next_dir = DIR_STOP;

        // Show maze around the player's original position
        (void)unveil_around_player(play_x, play_y);

        // use draw player block to store the background, draw the player only for the pixels that do not have the value 0x00,
        // draw to the screen and then draw the background (which will be updated on the next show_screen, essentially
        // making it so that when the player moves the old pixels where the player was gets overwritten)
        unsigned char * player_mask = get_player_mask(last_dir);
        draw_player_block(play_x, play_y, get_player_block(last_dir), player_mask, bg_blk);
        show_screen();
        draw_full_block(play_x, play_y, bg_blk);

        // get first Periodic Interrupt
        ret = read(fd, &data, sizeof(unsigned long));

        while ((quit_flag == 0) && (goto_next_level == 0)) {

            // PLAYER COLOR (change every half second)
            if (total % 16 == 0) { // 16 for half a second b/c half of 32hz
                player_color = (player_color + 1) % NUM_COLORS; 
                playerRGB[0] = red[player_color];
                playerRGB[1] = green[player_color];
                playerRGB[2] = blue[player_color];
                set_palette(0x20, playerRGB);
            }



                

            // ---- DRAW STATUS BAR ----

            // init variables
            int fruit_count = get_n_fruits();
            char status_str[MAX_CHARS] = ""; // 40 is total chars that can fit inside status bar
            char level_str[MAX_INT]; // 12 is max chars of an int in a string
            sprintf(level_str, "%d", level);
            char fruit_count_str[MAX_INT];
            sprintf(fruit_count_str, "%d", fruit_count);
            char sec_ones_str[MAX_INT];
            char sec_tens_str[MAX_INT];
            char min_ones_str[MAX_INT];
            char min_tens_str[MAX_INT];

            // convert total ticks to minutes and seconds and convert those into strings
            int total_sec = total / 32; // divide by 32 because 32 Hz to get seconds
            sec_ones = total_sec % 60 % 10; // 60 for seconds in a minute, 10 to split between ones place and tens place
            sec_tens = total_sec % 60 / 10;
            min_ones = total_sec / 60 % 10;
            min_tens = total_sec / 60 / 10;
            sprintf(sec_ones_str, "%d", sec_ones);
            sprintf(sec_tens_str, "%d", sec_tens);
            sprintf(min_ones_str, "%d", min_ones);
            sprintf(min_tens_str, "%d", min_tens);
            
            // put everything into the status string
            strcat(status_str, "Level ");
            strcat(status_str, level_str);
            strcat(status_str, "    ");
            strcat(status_str, fruit_count_str);
            if (fruit_count != 1)
                strcat(status_str, " Fruits   ");
            else
                strcat(status_str, " Fruit    ");
            strcat(status_str, min_tens_str);
            strcat(status_str, min_ones_str);
            strcat(status_str, ":");
            strcat(status_str, sec_tens_str);
            strcat(status_str, sec_ones_str);

            // calculate length of everything we want to print
            int length = strlen(status_str);
            // calculate number of spaces required to center status bar
            int num_spaces = (MAX_CHARS - length) / 2;
            int i;
            char space[MAX_CHARS] = " ";
            // put correct number of spaces before all of the text
            for (i = 0; i < num_spaces; i++) {
                strcat(space, status_str);
                strcpy(status_str, space);
                strcpy(space, " ");
            }

            // call function that calls text to graphics
            draw_status_bar(status_str);





            // --- DRAW TIME TO TUX ---
            // 12, 8, and 4 are correct bit shifts to get into corresponding 4 bit positions
            int led_time = (min_tens <<= 12) + (min_ones <<= 8) + (sec_tens <<= 4) + sec_ones;
            led_time |= 0x040F0000; // set 2nd decimal point and turn on all leds
            ioctl(fd_tux, TUX_SET_LED, led_time); // display time to leds




            // --- DRAW FLOATING TEXT ---
            int fnum = check_for_fruit (play_x/BLOCK_X_DIM, play_y/BLOCK_Y_DIM);
            if (fnum) { // if a fruit was found, set the string and set flag to be able to print it for a few seconds
                found_fruit_str = "             ";
                switch(fnum) {
                    case 1: // apple
                        found_fruit_str = "an apple!";
                        break;
                    case 2: // grapes
                        found_fruit_str = "grapes!";
                        break;
                    case 3: // peach
                        found_fruit_str = "a peach!";
                        break;
                    case 4: // strawberry
                        found_fruit_str = "a strawberry!";
                        break;
                    case 5: // banana
                        found_fruit_str = "a banana!";
                        break;
                    case 6: // watermelon
                        found_fruit_str = "watermelon!";
                        break;
                    case 7: // dew
                        found_fruit_str = "YEAH! DEW!";
                        break;
                    default:
                        break;
                }
                print_floating = 1; // flag for printing
                float_seconds = total_sec; // save time that we collected fruit so we can clear it 5 seconds after this
                float_offset = (int) (strlen(found_fruit_str) / 2 - 1) * CHAR_WIDTH;

                // do extra drawing of player and text to fix bugs with fruit on the wall (copied from below with better comments)
                unsigned char * player_mask = get_player_mask(last_dir);
                draw_player_block(play_x, play_y, get_player_block(last_dir), player_mask, bg_blk);
                unsigned char text_mask[FLOAT_TEXT_SIZE];
                unsigned char text_blk[FLOAT_TEXT_SIZE];
                make_text_mask("             ", text_mask);
                make_text_mask(found_fruit_str, text_mask);
                draw_floating_text(fruit_x, fruit_y, text_blk, text_mask, text_bg, float_offset, strlen(found_fruit_str));
                show_screen();
                redraw_text_bg(fruit_x, fruit_y, text_bg, float_offset, strlen(found_fruit_str));
                draw_full_block(play_x, play_y, bg_blk);
            }

            // check if we should be done printing the floating string (5 seconds has passed since last fruit pickup)
            if (print_floating && total_sec > float_seconds + FLOATING_TEXT_TIME) {
                found_fruit_str = "             ";
                print_floating = 0;
                float_offset = 0;

                // do extra drawing of player and text to fix bugs with stopping on wall after collecting fruit (copied from below with better comments)
                unsigned char * player_mask = get_player_mask(last_dir);
                draw_player_block(play_x, play_y, get_player_block(last_dir), player_mask, bg_blk);
                unsigned char text_mask[FLOAT_TEXT_SIZE];
                unsigned char text_blk[FLOAT_TEXT_SIZE];
                make_text_mask("             ", text_mask);
                make_text_mask(found_fruit_str, text_mask);
                draw_floating_text(fruit_x, fruit_y, text_blk, text_mask, text_bg, float_offset, strlen(found_fruit_str));
                show_screen();
                redraw_text_bg(fruit_x, fruit_y, text_bg, float_offset, strlen(found_fruit_str));
                draw_full_block(play_x, play_y, bg_blk);
            }
















            // Wait for Periodic Interrupt
            ret = read(fd, &data, sizeof(unsigned long));
        
            // Update tick to keep track of time.  If we missed some
            // interrupts we want to update the player multiple times so
            // that player velocity is smooth
            ticks = data >> 8;    

            total += ticks;

            // If the system is completely overwhelmed we better slow down:
            if (ticks > 8) ticks = 8;

            if (ticks > 1) {
                badcount++;
            }
            else {
                goodcount++;
            }

            while (ticks--) {

                ioctl(fd_tux, TUX_BUTTONS, &button);
                if (button != 0x000000FF)
                    buttons_pressed = 1;
                else
                    buttons_pressed = 0;

                pthread_mutex_lock(&mtx);
                if (buttons_pressed) {
                    pthread_cond_signal(&cv);
                }
                pthread_mutex_unlock(&mtx);

                // Lock the mutex
                pthread_mutex_lock(&mtx);

                // Check to see if a key has been pressed
                if (next_dir != dir) {
                    // Check if new direction is backwards...if so, do immediately
                    if ((dir == DIR_UP && next_dir == DIR_DOWN) ||
                        (dir == DIR_DOWN && next_dir == DIR_UP) ||
                        (dir == DIR_LEFT && next_dir == DIR_RIGHT) ||
                        (dir == DIR_RIGHT && next_dir == DIR_LEFT)) {
                        if (move_cnt > 0) {
                            if (dir == DIR_UP || dir == DIR_DOWN)
                                move_cnt = BLOCK_Y_DIM - move_cnt;
                            else
                                move_cnt = BLOCK_X_DIM - move_cnt;
                        }
                        dir = next_dir;
                    }
                }
                // New Maze Square!
                if (move_cnt == 0) {
                    // The player has reached a new maze square; unveil nearby maze
                    // squares and check whether the player has won the level.
                    if (unveil_around_player(play_x, play_y)) {
                        pthread_mutex_unlock(&mtx);
                        goto_next_level = 1;
                        break;
                    }
                
                    // Record directions open to motion.
                    find_open_directions (play_x / BLOCK_X_DIM, play_y / BLOCK_Y_DIM, open);
        
                    // Change dir to next_dir if next_dir is open 
                    if (open[next_dir]) {
                        dir = next_dir;
                    }
    
                    // The direction may not be open to motion...
                    //   1) ran into a wall
                    //   2) initial direction and its opposite both face walls
                    if (dir != DIR_STOP) {
                        if (!open[dir]) {
                            dir = DIR_STOP;
                        }
                        else if (dir == DIR_UP || dir == DIR_DOWN) {    
                            move_cnt = BLOCK_Y_DIM;
                        }
                        else {
                            move_cnt = BLOCK_X_DIM;
                        }
                    }
                }
                // Unlock the mutex
                pthread_mutex_unlock(&mtx);
        
                if (dir != DIR_STOP) {
                    // move in chosen direction
                    last_dir = dir;
                    move_cnt--;    
                    switch (dir) {
                        case DIR_UP:    
                            move_up(&play_y);    
                            break;
                        case DIR_RIGHT: 
                            move_right(&play_x); 
                            break;
                        case DIR_DOWN:  
                            move_down(&play_y);  
                            break;
                        case DIR_LEFT:  
                            move_left(&play_x);  
                            break;
                    }
                    // use draw player block to store the background, draw the player only for the pixels that do not have the value 0x00,
                    // draw to the screen and then draw the background (which will be updated on the next show_screen, essentially
                    // making it so that when the player moves the old pixels where the player was gets overwritten)
                    unsigned char * player_mask = get_player_mask(last_dir);
                    draw_player_block(play_x, play_y, get_player_block(last_dir), player_mask, bg_blk); 

                    // same logic as draw_player_block and draw_full_block except for the
                    // floating text (different dimensions and we must make our own text mask)
                    unsigned char text_mask[FLOAT_TEXT_SIZE];
                    unsigned char text_blk[FLOAT_TEXT_SIZE];
                    make_text_mask("             ", text_mask);
                    make_text_mask(found_fruit_str, text_mask);

                    // edge cases of text going to left and top borders (making it not go past those borders)
                    if (play_x < CHAR_WIDTH*(strlen(found_fruit_str)/2) + 2) // +2 because that looked the best when the floating text hit the top wall
                        fruit_x = CHAR_WIDTH*(strlen(found_fruit_str)/2) + 2;
                    else
                        fruit_x = play_x;
                        
                    if (play_y < 2 * BLOCK_Y_DIM + 8) // +8 because it looked the best. This makes it so the floating text on the left perfectly stops at the wall
                        fruit_y = 2 * BLOCK_Y_DIM + 8;
                    else
                        fruit_y = play_y;
                        
                    draw_floating_text(fruit_x, fruit_y, text_blk, text_mask, text_bg, float_offset, strlen(found_fruit_str)); // draw floating text

                    show_screen();

                    redraw_text_bg(fruit_x, fruit_y, text_bg, float_offset, strlen(found_fruit_str)); // replace background of floating text

                    draw_full_block(play_x, play_y, bg_blk); // redraw bg behind player
                }
            } // got rid of need_draw
        }    
    }
    if (quit_flag == 0)
        winner = 1;
    
    return 0;
}

/*
 * main
 *   DESCRIPTION: Initializes and runs the two threads
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: none
 */
int main() {
    int ret;
    struct termios tio_new;
    unsigned long update_rate = 32; /* in Hz */

    pthread_t tid1;
    pthread_t tid2;
    pthread_t tid_tux;

    // Initialize RTC
    fd = open("/dev/rtc", O_RDONLY, 0);

    // Initialize Tux
    fd_tux = open("/dev/ttyS0", O_RDWR | O_NOCTTY);
    int ldisc_num = N_MOUSE;
    ioctl(fd_tux, TIOCSETD, &ldisc_num);
    ioctl(fd_tux, TUX_INIT, 0);
    ioctl(fd_tux, TUX_SET_LED, 0x040F0000); // turn 2nd decimal on and only have right-most 3 leds on to start
    
    // Enable RTC periodic interrupts at update_rate Hz
    // Default max is 64...must change in /proc/sys/dev/rtc/max-user-freq
    ret = ioctl(fd, RTC_IRQP_SET, update_rate);    
    ret = ioctl(fd, RTC_PIE_ON, 0);

    // Initialize Keyboard
    // Turn on non-blocking mode
    if (fcntl(fileno(stdin), F_SETFL, O_NONBLOCK) != 0) {
        perror("fcntl to make stdin non-blocking");
        return -1;
    }
    
    // Save current terminal attributes for stdin.
    if (tcgetattr(fileno(stdin), &tio_orig) != 0) {
        perror("tcgetattr to read stdin terminal settings");
        return -1;
    }
    
    // Turn off canonical (line-buffered) mode and echoing of keystrokes
    // Set minimal character and timing parameters so as
    tio_new = tio_orig;
    tio_new.c_lflag &= ~(ICANON | ECHO);
    tio_new.c_cc[VMIN] = 1;
    tio_new.c_cc[VTIME] = 0;
    if (tcsetattr(fileno(stdin), TCSANOW, &tio_new) != 0) {
        perror("tcsetattr to set stdin terminal settings");
        return -1;
    }

    // Perform Sanity Checks and then initialize input and display
    if ((sanity_check() != 0) || (set_mode_X(fill_horiz_buffer, fill_vert_buffer) != 0)){
        return 3;
    }

    // Create the threads
    pthread_create(&tid1, NULL, rtc_thread, NULL);
    pthread_create(&tid2, NULL, keyboard_thread, NULL);
    pthread_create(&tid_tux, NULL, tux_thread, NULL);
    
    // Wait for all the threads to end
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    pthread_cancel(tid_tux);

    // Shutdown Display
    clear_mode_X();
    
    // Close Keyboard
    (void)tcsetattr(fileno(stdin), TCSANOW, &tio_orig);
        
    // Close RTC
    close(fd);
    
    // Close Tux
    close(fd_tux);

    // Print outcome of the game
    if (winner == 1) {    
        printf("You win the game! CONGRATULATIONS!\n");
    }
    else if (quit_flag == 1) {
        printf("Quitter!\n");
    }
    else {
        printf ("Sorry, you lose...\n");
    }

    // Return success
    return 0;
}
