; Multiplication table of 10
.org 0x9000

; Print table header
lda 77            ; ASCII 'M' 
printc            ; Print 'M'
lda 117           ; ASCII 'u'
printc            ; Print 'u'
lda 108           ; ASCII 'l'
printc            ; Print 'l'
lda 116           ; ASCII 't'
printc            ; Print 't'
lda 32            ; ASCII Space
printc            ; Print ' '
lda 84            ; ASCII 'T'
printc            ; Print 'T'
lda 97            ; ASCII 'a'
printc            ; Print 'a'
lda 98            ; ASCII 'b'
printc            ; Print 'b'
lda 108           ; ASCII 'l'
printc            ; Print 'l'
lda 101           ; ASCII 'e'
printc            ; Print 'e'
lda 58            ; ASCII ':'
printc            ; Print ':'
lda 10            ; ASCII newline
printc            ; Print newline

ldb 1       ;multiple
ldc 11      ;upto
mov8_mem_imm 0x0000 42  ; * 
mov8_mem_imm 0x0001 10  ; table of
mov8_mem_imm 0x0002 32  ; space
mov8_mem_imm 0x0003 61  ; =

print:
    lda 10;
    printa;
    lda 32;
    printc;
    lda 42;
    printc;
    lda 32;
    printc;
    mov_reg_reg a b;
    printa;
    lda 32;
    printc;
    lda 61;
    printc;
    lda 32;
    printc;
    lda 10;
    mul;
    printa;
    lda 10;
    printc;

inc b;
jlt print;
halt;