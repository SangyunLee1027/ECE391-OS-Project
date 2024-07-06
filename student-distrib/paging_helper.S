#define ASM     1

.text
.globl loadPageDirectory

.text
.globl enablePaging

loadPageDirectory:
    pushl   %ebp
    movl    %esp, %ebp
    movl    8(%esp), %eax
    movl    %eax, %cr3
    movl    %ebp, %esp
    popl    %ebp
    ret

enablePaging:
    movl    %cr4, %eax
    orl     $0x00000010, %eax /* enable page size extension */
    movl    %eax, %cr4
    movl    %cr0, %eax
    orl      $0x80000001, %eax /* enable Paging by 8 and protection mode by 1 */
    movl    %eax, %cr0
    ret
