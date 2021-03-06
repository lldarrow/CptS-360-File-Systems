.global main, mymain, myprintf

.data
    FMT: .asciz "main() in assembly call mymain() in C\n"
    a: .long 1234
    b: .long 5678
    fmt: .asciz "a=%d b=%d\n"

main:
    pushl %ebp
    movl %esp, %ebp

    #code to call myprintf(FMT)
    pushl $FMT
    call myprintf
    addl $4, %esp
    
    #$code to call mymain
    pushl 16(%ebp)
    pushl 12(%ebp)
    push 8(%ebp)
    call mymain
    addl $12, %esp
    
    #myprint(fmt,a,b)
    pushl $FMT
    call myprintf
    addl $4, %esp

    #return to caller
    movl %ebp, %esp
    popl %ebp
    ret
