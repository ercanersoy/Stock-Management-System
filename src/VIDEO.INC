; ==========================================================================
; VIDEO.INC - Direct MDA text buffer output routines
;
; All routines write straight into the MDA buffer at B000:0000 and preserve
; every register unless an output register is documented.
; ==========================================================================

; --------------------------------------------------------------------------
; calc_offset - Convert a screen position into a buffer offset
; In:  DH = row, DL = column
; Out: DI = offset into the video buffer
; --------------------------------------------------------------------------
calc_offset:
        push    ax
        mov     al, SCR_COLS * 2
        mul     dh
        mov     di, ax
        mov     al, dl
        xor     ah, ah
        shl     ax, 1
        add     di, ax
        pop     ax
        ret

; --------------------------------------------------------------------------
; video_es - Load ES with the video segment (AX preserved)
; --------------------------------------------------------------------------
video_es:
        push    ax
        mov     ax, VIDEO_SEG
        mov     es, ax
        pop     ax
        ret

; --------------------------------------------------------------------------
; put_char - Write one character
; In:  DH = row, DL = column, AL = character, AH = attribute
; --------------------------------------------------------------------------
put_char:
        push    di es
        call    calc_offset
        call    video_es
        stosw
        pop     es di
        ret

; --------------------------------------------------------------------------
; fill_chars - Write a character several times on one row
; In:  DH = row, DL = column, CX = count, AL = character, AH = attribute
; --------------------------------------------------------------------------
fill_chars:
        push    cx di es
        call    calc_offset
        call    video_es
        rep     stosw
        pop     es di cx
        ret

; --------------------------------------------------------------------------
; put_str - Write a zero terminated string
; In:  DH = row, DL = column, SI = string, AH = attribute
; Out: DL = column after the last character written
; --------------------------------------------------------------------------
put_str:
        push    ax si di es
        call    calc_offset
        call    video_es
.next:
        lodsb
        or      al, al
        jz      .done
        stosw
        inc     dl
        jmp     .next
.done:
        pop     es di si ax
        ret

; --------------------------------------------------------------------------
; put_field - Write a string left aligned in a fixed width cell
;             (padded with spaces, truncated if too long)
; In:  DH = row, DL = column, SI = string, CX = width, AH = attribute
; --------------------------------------------------------------------------
put_field:
        push    ax cx si di es
        jcxz    .done
        call    calc_offset
        call    video_es
.next:
        lodsb
        or      al, al
        jz      .pad
        stosw
        loop    .next
        jmp     .done
.pad:
        mov     al, ' '
        rep     stosw
.done:
        pop     es di si cx ax
        ret

; --------------------------------------------------------------------------
; put_field_right - Write a string right aligned in a fixed width cell
; In:  DH = row, DL = column, SI = string, CX = width, AH = attribute
; --------------------------------------------------------------------------
put_field_right:
        push    ax bx cx dx
        mov     bx, cx
        call    str_len                 ; CX = string length
        cmp     cx, bx
        jae     .fits
        xchg    cx, bx                  ; CX = width, BX = length
        sub     cx, bx                  ; CX = padding
        mov     al, ' '
        call    fill_chars
        add     dl, cl
        mov     cx, bx
        call    put_field
        jmp     .done
.fits:
        mov     cx, bx
        call    put_field
.done:
        pop     dx cx bx ax
        ret

; --------------------------------------------------------------------------
; hline - Draw a horizontal line with distinct end characters
; In:  DH = row, DL = column, CX = total width (>= 2), AH = attribute
;      AL = left character, BL = middle character, BH = right character
; --------------------------------------------------------------------------
hline:
        push    ax cx dx
        call    put_char
        inc     dl
        sub     cx, 2
        mov     al, bl
        call    fill_chars
        add     dl, cl
        mov     al, bh
        call    put_char
        pop     dx cx ax
        ret

; --------------------------------------------------------------------------
; draw_box - Draw a double line box
; In:  CH = top row, CL = left column, DH = bottom row, DL = right column
;      AH = attribute
; --------------------------------------------------------------------------
draw_box:
        push    ax bx cx dx di
        mov     di, dx                  ; keep bottom row / right column
        mov     bl, dl
        sub     bl, cl
        inc     bl
        xor     bh, bh
        mov     dx, cx                  ; DH = top row, DL = left column
        mov     cx, bx                  ; CX = width
        mov     al, 0C9h                ; top line
        mov     bx, 0BBCDh
        call    hline
.row:
        inc     dh
        mov     bx, di
        cmp     dh, bh
        jae     .bottom
        mov     al, 0BAh                ; side lines
        mov     bx, 0BA20h
        call    hline
        jmp     .row
.bottom:
        mov     al, 0C8h                ; bottom line
        mov     bx, 0BCCDh
        call    hline
        pop     di dx cx bx ax
        ret

; --------------------------------------------------------------------------
; clear_screen - Fill the whole screen with blanks
; --------------------------------------------------------------------------
clear_screen:
        push    ax cx dx
        xor     dx, dx
        mov     cx, SCR_COLS * SCR_ROWS
        mov     ax, ATTR_NORMAL * 256 + ' '
        call    fill_chars
        pop     dx cx ax
        ret

; --------------------------------------------------------------------------
; clear_row - Blank the inside of the work window on one row
; In:  DH = row
; --------------------------------------------------------------------------
clear_row:
        push    ax cx dx
        mov     dl, 1
        mov     cx, SCR_COLS - 2
        mov     ax, ATTR_NORMAL * 256 + ' '
        call    fill_chars
        pop     dx cx ax
        ret

; --------------------------------------------------------------------------
; set_cursor - Move the hardware cursor
; In:  DH = row, DL = column
; --------------------------------------------------------------------------
set_cursor:
        push    ax bx
        mov     ah, 02h
        xor     bh, bh
        int     10h
        pop     bx ax
        ret

; --------------------------------------------------------------------------
; hide_cursor - Move the hardware cursor off the visible screen
; --------------------------------------------------------------------------
hide_cursor:
        push    dx
        mov     dx, SCR_ROWS * 256
        call    set_cursor
        pop     dx
        ret

; --------------------------------------------------------------------------
; beep - Short 1 kHz tone on the PC speaker (PIT channel 2, ~110 ms)
; --------------------------------------------------------------------------
beep:
        push    ax cx dx es
        mov     al, 0B6h                ; channel 2, lobyte/hibyte, square
        out     43h, al
        mov     ax, 1193                ; 1193182 Hz / 1000 Hz
        out     42h, al
        mov     al, ah
        out     42h, al
        in      al, 61h
        mov     ah, al                  ; keep the original port value
        or      al, 03h                 ; gate channel 2 to the speaker
        out     61h, al
        mov     cx, 40h                 ; wait two BIOS timer ticks
        mov     es, cx
        mov     cx, [es:6Ch]
.wait:
        mov     dx, [es:6Ch]
        sub     dx, cx
        cmp     dx, 2
        jb      .wait
        mov     al, ah                  ; speaker off
        out     61h, al
        pop     es dx cx ax
        ret
