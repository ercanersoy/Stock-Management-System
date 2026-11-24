; ==========================================================================
; VIDEO.INC - Display adapter detection and direct text buffer output
;
; Supported adapters: MDA, Hercules, CGA, EGA, PGC, MCGA and VGA.
; Monochrome displays run in mode 7 (B000:0000) with the MDA attributes,
; colour displays in mode 3 (B800:0000) with the same green on black look.
; On a CGA every write waits for the horizontal retrace to avoid "snow".
;
; The output routines preserve every register unless an output register is
; documented. They expect DS to address the program segment.
; ==========================================================================

; --------------------------------------------------------------------------
; detect_adapter - Find the active display adapter
; Out: [adapter] = ADP_xxx, [mono_display] = 1 for a monochrome monitor
; --------------------------------------------------------------------------
detect_adapter:
        push    ax bx cx dx
        mov     ax, 1A00h               ; VGA/MCGA BIOS: display combination
        int     10h
        cmp     al, 1Ah
        jne     .no_vga
        mov     al, bl                  ; BL = active display code
        mov     ah, 1
        cmp     al, 1
        je      .mda
        mov     ah, 0
        mov     bl, ADP_CGA
        cmp     al, 2
        je      .found
        mov     bl, ADP_EGA
        cmp     al, 4
        je      .found
        mov     ah, 1
        cmp     al, 5
        je      .found
        mov     ah, 0
        mov     bl, ADP_PGC
        cmp     al, 6
        je      .found
        mov     bl, ADP_VGA
        mov     ah, 1
        cmp     al, 7
        je      .found
        mov     ah, 0
        cmp     al, 8
        je      .found
        mov     bl, ADP_MCGA
        mov     ah, 1
        cmp     al, 0Bh
        je      .found
        mov     ah, 0
        cmp     al, 0Ch
        je      .found
.no_vga:
        mov     ah, 12h                 ; EGA BIOS: get configuration
        mov     bl, 10h
        int     10h
        cmp     bl, 10h
        je      .no_ega
        mov     cl, bh                  ; CL = 0 colour, 1 mono monitor
        mov     ah, 0Fh                 ; is the EGA the active adapter?
        int     10h
        and     al, 7Fh
        cmp     al, MODE_MONO
        je      .ega_mode7
        or      cl, cl                  ; colour mode: EGA needs a colour
        jnz     .no_ega                 ; monitor, else it is a CGA
        mov     bl, ADP_EGA
        xor     ah, ah
        jmp     .found
.ega_mode7:
        or      cl, cl                  ; mode 7: EGA needs a mono monitor,
        jz      .no_ega                 ; else it is an MDA/Hercules
        mov     bl, ADP_EGA
        mov     ah, 1
        jmp     .found
.no_ega:
        int     11h                     ; BIOS equipment list
        and     al, 30h
        cmp     al, 30h                 ; initial video mode 80x25 mono
        je      .mda_card
        mov     ah, 0Fh                 ; CGA card, unless mode 7 is active
        int     10h
        and     al, 7Fh
        cmp     al, MODE_MONO
        je      .mda_card
        call    detect_pgc
        mov     bl, ADP_PGC
        jnc     .color
        mov     bl, ADP_CGA
.color:
        xor     ah, ah
        jmp     .found
.mda_card:
        mov     ah, 1
.mda:
        call    detect_hercules
        mov     bl, ADP_HERCULES
        jnc     .found
        mov     bl, ADP_MDA
.found:
        mov     [adapter], bl
        mov     [mono_display], ah
        pop     dx cx bx ax
        ret

; --------------------------------------------------------------------------
; detect_hercules - Tell a Hercules card from an MDA: bit 7 of the status
;                   port (vertical sync) only changes on a Hercules card
; Out: CF = 0 Hercules, CF = 1 MDA
; --------------------------------------------------------------------------
detect_hercules:
        push    ax bx cx dx
        mov     dx, HGC_STATUS
        in      al, dx
        and     al, 80h
        mov     bl, al
        mov     cx, 8000h
.poll:
        in      al, dx
        and     al, 80h
        cmp     al, bl
        jne     .hercules
        loop    .poll
        stc
        jmp     .done
.hercules:
        clc
.done:
        pop     dx cx bx ax
        ret

; --------------------------------------------------------------------------
; detect_pgc - Look for the PGC communication buffer (RAM at C600:0000)
; Out: CF = 0 PGC found, CF = 1 not found
; --------------------------------------------------------------------------
detect_pgc:
        push    ax bx es
        mov     ax, 0C600h
        mov     es, ax
        pushf
        cli                             ; nothing may run while the byte
        mov     al, [es:0]              ; is changed: original byte
        mov     ah, al
        not     al
        mov     [es:0], al
        mov     bl, [es:0]
        mov     [es:0], ah              ; restore it
        popf
        cmp     bl, al
        je      .found                  ; CF = 0
        stc
.found:
        pop     es bx ax
        ret

; --------------------------------------------------------------------------
; init_video - Detect the adapter and set up the text mode and attributes
; --------------------------------------------------------------------------
init_video:
        push    ax bx
        mov     ah, 0Fh                 ; remember the mode to restore at exit
        int     10h
        and     al, 7Fh
        mov     [orig_mode], al
        call    detect_adapter
        mov     al, [adapter]
        cmp     al, ADP_MDA
        je      .mono
        cmp     al, ADP_HERCULES
        je      .mono
        cmp     al, ADP_MCGA            ; MCGA has no mode 7
        je      .color
        cmp     byte [mono_display], 0  ; EGA/VGA with a mono monitor (a
        jne     .mono                   ; colour one gets green, even if
                                        ; mode 7 was active)
.color:
        mov     word [video_seg], SEG_COLOR
        mov     word [attr_normal], GREEN_BRIGHT * 256 + GREEN_NORMAL
        mov     word [attr_field], GREEN_REVERSE * 256 + GREEN_FIELD
        mov     byte [field_pad], '_'
        cmp     byte [adapter], ADP_CGA
        jne     .no_snow
        mov     byte [snow], 1
.no_snow:
        mov     al, MODE_COLOR
        jmp     .set_mode
.mono:
        mov     al, MODE_MONO           ; defaults in DATA.INC are for mono
.set_mode:
        mov     [video_mode], al
        xor     ah, ah                  ; set the mode (also clears screen)
        int     10h
        pop     bx ax
        ret

; --------------------------------------------------------------------------
; restore_video - Restore the video mode that was active at start-up
; --------------------------------------------------------------------------
restore_video:
        push    ax
        mov     al, [orig_mode]
        xor     ah, ah
        int     10h
        pop     ax
        ret

; --------------------------------------------------------------------------
; vput - Store one character/attribute word at ES:DI (DI advanced by 2).
;        On a CGA the write waits for the horizontal retrace (no snow).
; In:  AX = character and attribute
; --------------------------------------------------------------------------
vput:
        cmp     byte [snow], 0
        jne     .wait
        stosw
        ret
.wait:
        push    bx dx
        mov     bx, ax
        mov     dx, CGA_STATUS
        cli
.in_retrace:                            ; wait for the end of a retrace ...
        in      al, dx
        test    al, 1
        jnz     .in_retrace
.no_retrace:                            ; ... and the start of the next one
        in      al, dx
        test    al, 1
        jz      .no_retrace
        mov     ax, bx
        stosw
        sti
        pop     dx bx
        ret

; --------------------------------------------------------------------------
; vfill - Store a character/attribute word CX times at ES:DI
; In:  AX = character and attribute, CX = count
; Out: CX = 0, DI advanced
; --------------------------------------------------------------------------
vfill:
        cmp     byte [snow], 0
        jne     .slow
        rep     stosw
        ret
.slow:
        jcxz    .done
.next:
        call    vput
        loop    .next
.done:
        ret

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
        mov     ax, [video_seg]
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
        call    vput
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
        call    vfill
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
        call    vput
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
        call    vput
        loop    .next
        jmp     .done
.pad:
        mov     al, ' '
        call    vfill
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
        mov     ah, [attr_normal]
        mov     al, ' '
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
        mov     ah, [attr_normal]
        mov     al, ' '
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
