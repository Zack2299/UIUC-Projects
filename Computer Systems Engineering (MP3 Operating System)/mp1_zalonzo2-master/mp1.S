
.data					# section declaration

        # Useful offset constants for accessing members of a 
        # struct mp1_blink_struct structure
        LOCATION   = 0    
        ON_CHAR    = 2
        OFF_CHAR   = 3 
        ON_LENGTH  = 4
        OFF_LENGTH = 6
        COUNTDOWN  = 8
        STATUS     = 10
        NEXT       = 12

        STRUCT_SIZE = 16

# Pointer to head of list (initialized to NULL)
mp1_list_head:
        .long   0

.text					# section declaration

# Export the function symbol names

.global mp1_rtc_tasklet
.global mp1_ioctl

# void mp1_poke(void);
#
# Interface: Register-based arguments (not C-style)
#    Inputs: %cl  - The byte you wish to write
#            %eax - Offset from the start of video memory that you wish
#                   to write to
#   Outputs: Text-mode video screen is written to at location %eax with
#            the byte in %cl
# Registers: Clobbers EDX
mp1_poke:
        
	movl    vmem_base_addr(,1),%edx
	movb    %cl,(%edx,%eax,1)
	ret

mp1_rtc_tasklet: # void mp1 rtc tasklet (unsigned long);

/* REGISTER REFERENCE
        ebx - pointer to head
        esi - temp
*/
        # CALLEE SAVED REGISTERS
        pushl   %ebp
        movl    %esp, %ebp

        pushl   %ebx
        pushl   %edi
        pushl   %esi

        # INIT POINTER TO HEAD
        movl    mp1_list_head,%ebx

TASKLET_LOOP:
        # BASE CASE (NULL POINTER)
        cmpl    $0, %ebx # [reg: pointer]
        je      FINISH # void function - don't return anything

        # DECREMENT COUNTDOWN AND CHECK IF ZERO
        movw    COUNTDOWN(%ebx), %si
        decw    %si
        movw    %si,COUNTDOWN(%ebx)
        cmpw    $0, %si
        jg      TASKLET_SKIP

        # COUNTDOWN IS ZERO
        movw    STATUS(%ebx), %si
        cmpw    $1, %si
        jne     STATUS_0

STATUS_1: # currently on -> want to change it to off char and move off_length to countdown
        movb    OFF_CHAR(%ebx), %cl
        movw    OFF_LENGTH(%ebx), %si
        movw    %si, COUNTDOWN(%ebx)
        movw    $0, STATUS(%ebx)

        jmp     SKIP_STATUS_0

STATUS_0: # currently off -> want to change it to on char and move on_length to countdown
        movb    ON_CHAR(%ebx),%cl
        movw    ON_LENGTH(%ebx), %si
        movw    %si, COUNTDOWN(%ebx)
        movw    $1, STATUS(%ebx)

SKIP_STATUS_0:
        # CALLER SAVED REGISTERS
        pushl   %ecx
        pushl   %edx
        # CALL MP1_POKE
        xorl    %eax, %eax
        movw    LOCATION(%ebx),%ax
        shl     %eax # multiply location by 2 because 2 bytes per character
        # correct char is already in %cl from status section
        call    mp1_poke
        # RESTORE CALLER SAVED REGISTERS
        popl    %edx
        popl    %ecx

TASKLET_SKIP:
        # LOOP BACK UNTIL FINISHED WITH LINKED LIST
        movl    NEXT(%ebx), %esi
        movl    %esi, %ebx
        jmp     TASKLET_LOOP

mp1_ioctl: # int mp1_ioctl (unsigned long arg, unsigned long cmd);

/* REGISTER REFERENCE
        ecx - cmd
        ebx - arg (pointer to struct)
        esi - temp register
        edi - void pointer from mp1_malloc
*/
        # DO NOT TOUCH STACK IN DISPATCHER (THIS SECTION)

        # FETCH PARAMETERS (do not do movl 8 and movl 12 like ps1 because we did not push ebp)
        # don't need this: movl    4(%esp), %ebx  # [reg: sp, arg]
        movl    8(%esp), %ecx   # [reg: sp, cmd]

        # HANDLE BAD CMD
        cmpl    $3, %ecx # [reg: cmd]
        jg      BAD_CMD  # jump if ecx > 3
        cmpl    $0, %ecx # [reg: cmd]
        jl      BAD_CMD  # jump if ecx < 0

        # JUMP BASED ON CMD
        jmp     *jumptable(,%ecx,4) # [reg: cmd]

# return -1 and do not pop callee saved registers (unlike ret_neg_1)
BAD_CMD:
        movl    $-1, %eax
        ret

jumptable:
    .long mp1_ioctl_add, mp1_ioctl_remove, mp1_ioctl_find, mp1_ioctl_sync


# CORE FUNCTIONS
mp1_ioctl_add:
        # CALLEE SAVED REGISTERS
        pushl   %ebp
        movl    %esp, %ebp

        pushl   %ebx
        pushl   %edi
        pushl   %esi

        # GET ARG
        movl    8(%ebp), %ebx   # [reg: bp, arg]

        # HANDLE NULL POINTER
        cmpl    $0, %ebx
        je      RET_NEG_1

        # CALLER SAVED REGISTERS
        pushl   %ecx
        pushl   %edx
        # PUSH PARAMETER (unsigned long size) AND CALL MP1_MALLOC
        pushl   $STRUCT_SIZE
        call    mp1_malloc # returns void pointer
        addl    $4,%esp # 'pop' parameter
        # RESTORE CALLER SAVED REGISTERS
        popl    %edx
        popl    %ecx

        # HANDLE MALLOC ERROR
        cmpl    $0, %eax # eax is void pointer to allocated memory returned from malloc
        je      RET_NEG_1 # jump if void pointer is NULL (equal to 0)
        movl    %eax, %edi # (put void pointer in eax into edi to save in case of error in copy_from_user)

        # CALLER SAVED REGISTERS
        pushl   %ecx
        pushl   %edx
        # PUSH PARAMETERS (void *to, const void *from, unsigned long n) AND CALL mp1_copy_from_user
        pushl   $STRUCT_SIZE
        pushl   %ebx # [reg: arg]
        pushl   %edi # [reg: void pointer from malloc]
        call    mp1_copy_from_user # returns unsigned long
        addl    $12,%esp # 'pop' 3 parameters
        # RESTORE CALLER SAVED REGISTERS
        popl    %edx
        popl    %ecx

        # HANDLE COPY FROM USER ERROR
        cmpl    $0, %eax
        jne     REVERT_MALLOC # jump if it's anything other than 0 because that means it could not copy all bits

        # HANDLE BAD LOCATION
        xor     %esi, %esi
        movw    LOCATION(%edi),%si # [reg: arg (pointer to struct), temp]     movs or movw
        cmpw    $1999, %si # [reg: location]
        jg      REVERT_MALLOC  # jump if esi > 80*25-1 (1999)
        cmpw    $0, %si # [reg: location]
        jl      REVERT_MALLOC  # jump if esi < 0

        # SET COUNTDOWN TO BE EQUAL TO ON_LENGTH
        movl    ON_LENGTH(%edi),%esi # [reg: arg (pointer to struct), temp]
        movl    %esi,COUNTDOWN(%edi)
        
        # SET STATUS TO 1
        movl    $1,STATUS(%edi)

        # INSERT STRUCTURE AT HEAD
        movl    mp1_list_head,%esi # [reg: temp]
        movl    %esi,NEXT(%edi) # point to the last struct that was the head, including NULL
        movl    %edi,mp1_list_head # [reg: arg (pointer to struct)]

        # CALLER SAVED REGISTERS
        pushl   %ecx
        pushl   %edx
        # CALL MP1_POKE (no parameters to push and 'pop', but we need to put inputs into correct registers)
        xorl    %eax, %eax
        movw    LOCATION(%edi),%ax
        shl     %eax # multiply location by 2 because 2 bytes per character
        movb    ON_CHAR(%edi),%cl
        call    mp1_poke
        # RESTORE CALLER SAVED REGISTERS
        popl    %edx
        popl    %ecx

        # SUCCESS: RETURN 0
        movl    $0, %eax

	jmp     FINISH

search_for_location: # search_for_location(void *head, int location)
/* REGISTER REFERENCE:
        ebx - mp1_blink_struct pointer to head
        edi - location to look for
        esi - temp
        edx - pointer to previous mp1_blink_struct

   RETURNS:
        eax - NULL if not found, pointer to mp1_blink_struct that has the same location if found
        edx - NULL if eax is head, otherwise it is the previous mp1_blink_struct

*/
        # CALLEE SAVED REGISTERS
        pushl   %ebp
        movl    %esp, %ebp

        pushl   %ebx
        pushl   %edi
        pushl   %esi
        
        # FETCH PARAMETERS
        movl    8(%ebp), %ebx # [reg: bp, mp1_blink_struct pointer to head]
        movl    12(%ebp), %edi # [reg: bp, location to look for]   (don't forget to clear all of location just like eax in add)

        # BASE CASE (NULL POINTER)
        cmpl    $0, %ebx # [reg: pointer]
        je      RET_0

        # IF EBX IS HEAD POINTER, SET PREV (edx) TO NULL
        cmpl    mp1_list_head, %ebx
        jne     EBX_NOT_HEAD
        xorl    %edx, %edx # do this if ebx is head of linked list
EBX_NOT_HEAD:

        # FOUND LOCATION?
        movw    LOCATION(%ebx),%si # [reg: pointer, temp]
        cmpw    %di, %si # [reg: location to look for, current struct's location]
        je      FOUND_LOCATION

        # DIDN'T FIND LOCATION -> SET PREV TO CURRENT
        movl    %ebx, %edx

        # CALLER SAVED REGISTERS
        pushl   %ecx
        # want edx to be clobbered
        # DID NOT FIND LOCATION (RECURSE)
        pushl   %edi
        movl    NEXT(%ebx), %esi # next struct pointer
        pushl   %esi
        call    search_for_location
        addl    $8,%esp # 'pop' 2 parameters
        # RESTORE CALLER SAVED REGISTERS
        popl    %ecx

        jmp     FINISH

FOUND_LOCATION:
        movl    %ebx, %eax
        jmp     FINISH

        
mp1_ioctl_remove:
/* REGISTER REFERENCE
        bx - location of what we want to remove
        edi - pointer to struct that was found
        edx - pointer to prev struct of what was found
        esi - temp
*/
        # CALLEE SAVED REGISTERS
        pushl   %ebp
        movl    %esp, %ebp

        pushl   %ebx
        pushl   %edi
        pushl   %esi

        # GET ARG
        movl    8(%ebp), %ebx   # [reg: bp, arg]

        # HANDLE BAD LOCATION
        cmpl    $1999, %ebx # [reg: location]
        jg      RET_NEG_1  # jump if bx > 80*25-1 (1999)
        cmpl    $0, %ebx # [reg: location]
        jl      RET_NEG_1  # jump if bx < 0

        # CALLER SAVED REGISTERS
        pushl   %ecx
        # want edx to be clobbered
        # CALL SEARCH (search_for_location(void *head, int location))
        pushl   %ebx # push location
        movl    mp1_list_head, %esi
        pushl   %esi
        call    search_for_location
        addl    $8,%esp # 'pop' 2 parameters
        # RESTORE CALLER SAVED REGISTERS
        # don't pop edx because we want search to clobber edx to save the pointer to the prev struct (in this case)
        popl    %ecx

        # RETURN -1 IF SEARCH FAILED
        cmpl    $0, %eax
        je      RET_NEG_1

        # MOVE RETURN FROM SEARCH TO EDI
        movl    %eax, %edi
 
        # TEST TO SEE IF EDI, THE STRUCT FOUND, IS HEAD OF LIST OR NOT
        cmpl    $0, %edx # when edx is 0, edi has no prev, so it's the head
        je     EDI_IS_HEAD
EDI_IS_NOT_HEAD: # set prev (edx) next to curr (edi) next
        movl    NEXT(%edi), %esi
        movl    %esi,NEXT(%edx)
        jmp     SKIP_EDI_IS_HEAD
EDI_IS_HEAD: # set curr (edi) next as the new list head
        movl    NEXT(%edi), %esi
        movl    %esi, mp1_list_head
SKIP_EDI_IS_HEAD: # deallocate curr (edi)
        jmp     REVERT_MALLOC_RET_0
        
mp1_ioctl_find:
/* REGISTER REFERENCE
        ebx - arg (pointer to struct)
        esi - temp register
        edi - pointer to struct in list (not user)
*/
        # CALLEE SAVED REGISTERS
        pushl   %ebp
        movl    %esp, %ebp

        pushl   %ebx
        pushl   %edi
        pushl   %esi

        # GET ARG
        movl    8(%ebp), %ebx   # [reg: bp, arg]

        # HANDLE NULL POINTER
        cmpl    $0, %ebx
        je      RET_NEG_1

        # CALLER SAVED REGISTERS
        pushl   %ecx
        pushl   %edx
        # CALL SEARCH (search_for_location(void *head, int location))
        xorl    %esi, %esi
        movl    LOCATION(%ebx), %esi
        pushl   %esi
        movl    mp1_list_head, %esi
        pushl   %esi
        call    search_for_location
        addl    $8,%esp # 'pop' 2 parameters
        # RESTORE CALLER SAVED REGISTERS
        popl    %edx
        popl    %ecx

        # RETURN -1 IF SEARCH FAILED
        cmpl    $0, %eax
        je      RET_NEG_1

        # MOVE RETURN FROM SEARCH TO EDI
        movl    %eax, %edi

        # CALLER SAVED REGISTERS
        pushl   %ecx
        pushl   %edx
        # PUSH PARAMETERS (void *to, const void *from, unsigned long n) AND CALL mp1_copy_to_user
        pushl   $STRUCT_SIZE
        pushl   %edi
        pushl   %ebx
        call    mp1_copy_to_user 
        addl    $12,%esp # 'pop' 3 parameters
        # RESTORE CALLER SAVED REGISTERS
        popl    %edx
        popl    %ecx

        # HANDLE COPY FROM USER ERROR
        cmpl    $0, %eax
        jne     RET_NEG_1

        # SUCCESS: RETURN 0
        movl    $0, %eax

	jmp     FINISH

mp1_ioctl_sync:
/* REGISTER REFERENCE
        ebx - arg
        bx - upper 16 bits of arg
        cx - lower 16 bits of arg
        edi - first pointer
        edx - second pointer
        esi - temp
*/
        # CALLEE SAVED REGISTERS
        pushl   %ebp
        movl    %esp, %ebp

        pushl   %ebx
        pushl   %edi
        pushl   %esi

        # GET ARG
        movl    8(%ebp), %ebx   # [reg: bp, arg]

        # EXTRACT FROM EBX
        xorl    %ecx, %ecx
        movw    %bx, %cx
        shrl     $16,%ebx

        # CALLER SAVED REGISTERS
        pushl   %ecx
        pushl   %edx
        # CALL SEARCH (search_for_location(void *head, int location))
        xorl    %esi, %esi
        movw    %bx, %si
        pushl   %esi
        movl    mp1_list_head, %esi
        pushl   %esi
        call    search_for_location
        addl    $8,%esp # 'pop' 2 parameters
        # RESTORE CALLER SAVED REGISTERS
        popl    %edx
        popl    %ecx

        # RETURN -1 IF SEARCH FAILED
        cmpl    $0, %eax
        je      RET_NEG_1

        # MOVE RETURN FROM SEARCH TO EDI
        movl    %eax, %edi

        # CALLER SAVED REGISTERS
        pushl   %ecx
        pushl   %edx
        # CALL SEARCH (search_for_location(void *head, int location))
        xorl    %esi, %esi
        movw    %cx, %si
        pushl   %esi
        movl    mp1_list_head, %esi
        pushl   %esi
        call    search_for_location
        addl    $8,%esp # 'pop' 2 parameters
        # RESTORE CALLER SAVED REGISTERS
        popl    %edx
        popl    %ecx

        # RETURN -1 IF SEARCH FAILED
        cmpl    $0, %eax
        je      RET_NEG_1

        # MOVE RETURN FROM SEARCH TO EDX
        movl    %eax, %edx

        # COPY FROM CHAR 1 (edi) TO CHAR 2 (edx)
        xorl    %esi, %esi
        movw    ON_LENGTH(%edi), %si
        movw    %si, ON_LENGTH(%edx)
        movw    OFF_LENGTH(%edi), %si
        movw    %si, OFF_LENGTH(%edx)
        movw    COUNTDOWN(%edi), %si
        movw    %si, COUNTDOWN(%edx)
        movw    STATUS(%edi), %si
        movw    %si, STATUS(%edx)

        # DETERMINE WHICH CHARACTER TO PRINT BASED ON STATUS
        cmpw    $0,%si
        je      PRINT_OFF # print off_char if status is 0
PRINT_ON:
        movb    ON_CHAR(%edx),%cl
        jmp SKIP_PRINT_OFF
PRINT_OFF:
        movb    OFF_CHAR(%edx),%cl
SKIP_PRINT_OFF:
        # CALLER SAVED REGISTERS
        pushl   %ecx
        pushl   %edx
        # CALL MP1_POKE
        xorl    %eax, %eax
        movw    LOCATION(%edx),%ax
        shl     %eax # multiply location by 2 because 2 bytes per character
        # correct char is already in %cl from print section
        call    mp1_poke
        # RESTORE CALLER SAVED REGISTERS
        popl    %edx
        popl    %ecx

        # SUCCESS: RETURN 0
        movl    $0, %eax

	jmp     FINISH

RET_0:
        movl    $0, %eax
        jmp     FINISH


REVERT_MALLOC_RET_0:
        # PUSH PARAMETER (void *ptr) THEN CALL MP1_FREE
        pushl   %edi # saved from malloc error section
        call    mp1_free
        addl    $4,%esp # 'pop' parameter
        movl    $0, %eax
        jmp     FINISH

REVERT_MALLOC:
        # PUSH PARAMETER (void *ptr) THEN CALL MP1_FREE
        pushl   %edi # saved from malloc error section
        call    mp1_free
        addl    $4,%esp # 'pop' parameter
# |
# v

RET_NEG_1:
        movl    $-1, %eax
# |
# v

FINISH:
        # RESTORE CALLEE SAVED REGISTERS
        popl    %esi
        popl    %edi
        popl    %ebx

        leave
        ret # ret automatically pops correct amount of parameters used
.end
