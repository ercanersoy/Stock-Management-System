; ==========================================================================
; FILEIO.INC - Buffered text file input/output through DOS handles
;
; Only one file is open at a time; it shares io_buf for reading and writing.
; ==========================================================================

; --------------------------------------------------------------------------
; fopen_read - Open a file for reading
; In:  DX = file name
; Out: CF = 1 if the file could not be opened
; --------------------------------------------------------------------------
fopen_read:
        push    ax
        mov     ax, 3D00h
        int     21h
        jc      .done
        mov     [f_handle], ax
        xor     ax, ax
        mov     [io_pos], ax
        mov     [io_len], ax
        mov     [f_eof], al
        mov     [f_error], al
        clc
.done:
        pop     ax
        ret

; --------------------------------------------------------------------------
; fcreate - Create (or truncate) a file for writing
; In:  DX = file name
; Out: CF = 1 if the file could not be created
; --------------------------------------------------------------------------
fcreate:
        push    ax cx
        mov     ah, 3Ch
        xor     cx, cx
        int     21h
        jc      .done
        mov     [f_handle], ax
        xor     ax, ax
        mov     [io_pos], ax
        mov     [f_error], al
        clc
.done:
        pop     cx ax
        ret

; --------------------------------------------------------------------------
; fclose - Close the current file
; --------------------------------------------------------------------------
fclose:
        push    ax bx
        mov     ah, 3Eh
        mov     bx, [f_handle]
        int     21h
        pop     bx ax
        ret

; --------------------------------------------------------------------------
; fgetc - Read one character
; Out: AL = character, CF = 1 at end of file (Ctrl-Z also ends the file)
; --------------------------------------------------------------------------
fgetc:
        push    bx cx dx si
        cmp     byte [f_eof], 0
        jne     .eof
        mov     si, [io_pos]
        cmp     si, [io_len]
        jb      .have
        mov     ah, 3Fh
        mov     bx, [f_handle]
        mov     cx, IO_SIZE
        mov     dx, io_buf
        int     21h
        jc      .set_eof
        or      ax, ax
        jz      .set_eof
        mov     [io_len], ax
        xor     si, si
.have:
        mov     al, [io_buf + si]
        inc     si
        mov     [io_pos], si
        cmp     al, 1Ah
        je      .set_eof
        clc
        jmp     .done
.set_eof:
        mov     byte [f_eof], 1
.eof:
        stc
.done:
        pop     si dx cx bx
        ret

; --------------------------------------------------------------------------
; fgets - Read one line (CR/LF removed, overlong lines truncated)
; In:  DI = buffer, CX = maximum length without the terminator
; Out: CF = 1 at end of file with nothing read
; --------------------------------------------------------------------------
fgets:
        push    ax bx cx di
        xor     bx, bx                  ; characters seen on this line
.next:
        call    fgetc
        jc      .eof
        cmp     al, 13
        je      .next
        cmp     al, 10
        je      .eol
        inc     bx
        jcxz    .next                   ; buffer full: drop the rest
        mov     [di], al
        inc     di
        dec     cx
        jmp     .next
.eof:
        mov     byte [di], 0
        or      bx, bx
        jnz     .ok
        stc
        jmp     .done
.eol:
        mov     byte [di], 0
.ok:
        clc
.done:
        pop     di cx bx ax
        ret

; --------------------------------------------------------------------------
; fflush - Write the output buffer to the file
; --------------------------------------------------------------------------
fflush:
        push    ax bx cx dx
        mov     cx, [io_pos]
        jcxz    .done
        mov     ah, 40h
        mov     bx, [f_handle]
        mov     dx, io_buf
        int     21h
        jc      .error
        cmp     ax, cx
        je      .written
.error:
        mov     byte [f_error], 1       ; write error or disk full
.written:
        mov     word [io_pos], 0
.done:
        pop     dx cx bx ax
        ret

; --------------------------------------------------------------------------
; fputc - Write one character
; In:  AL = character
; --------------------------------------------------------------------------
fputc:
        push    si
        mov     si, [io_pos]
        mov     [io_buf + si], al
        inc     si
        mov     [io_pos], si
        cmp     si, IO_SIZE
        jb      .done
        call    fflush
.done:
        pop     si
        ret

; --------------------------------------------------------------------------
; fputs - Write a zero terminated string
; In:  SI = string
; --------------------------------------------------------------------------
fputs:
        push    ax si
.next:
        lodsb
        or      al, al
        jz      .done
        call    fputc
        jmp     .next
.done:
        pop     si ax
        ret

; --------------------------------------------------------------------------
; fputnl - Write a CR/LF line ending
; --------------------------------------------------------------------------
fputnl:
        push    ax
        mov     al, 13
        call    fputc
        mov     al, 10
        call    fputc
        pop     ax
        ret

; --------------------------------------------------------------------------
; fclose_write - Flush and close a file opened with fcreate
; Out: CF = 1 if any write failed
; --------------------------------------------------------------------------
fclose_write:
        call    fflush
        call    fclose
        cmp     byte [f_error], 1       ; CF = 1 if f_error = 0 ...
        cmc                             ; ... so invert it
        ret
