#define ASM     1
.global syscall_handler, INVALIDCMD, DONE, syscall_table

syscall_handler:
    pushfl
    pushl   %esi
    pushl   %edi

    pushl   %edx
    pushl   %ecx 
    pushl   %ebx

    cmpl     $1, %eax /* is input in valid range, 1 - 6? */
    jl      INVALIDCMD
    cmpl     $10, %eax
    jg      INVALIDCMD

    call    *syscall_table(, %eax, 4)
    jmp     DONE

INVALIDCMD:
    movl    $-1, %eax 

DONE:
    popl    %ebx
    popl    %ecx
    popl    %edx
    
    popl    %edi
    popl    %esi
    popfl

    iret

syscall_table:
    .long   0x0
    .long   halt
    .long   execute
    .long   read
    .long   write
    .long   open
    .long   close
    .long   getargs
    .long   vidmap
    .long   set_handler
    .long   sigreturn
