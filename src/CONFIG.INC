; ==========================================================================
; CONFIG.INC - STOCK.CFG load/save
;
; STOCK.CFG is a plain text file with KEY=VALUE lines. Empty lines and lines
; starting with ';' or '#' are comments. Recognised keys:
;       COMPANY   company name shown in the title bar
;       LOWSTOCK  items with quantity at or below this level are flagged
;       CONFIRM   YES/NO - ask before deleting an item
;       AUTOSAVE  YES/NO - write DATA.DAT after every change
; ==========================================================================

L_COMPANY       = 30                    ; company name length
L_KEY           = 15                    ; longest configuration key

; --------------------------------------------------------------------------
; parse_yes_no - Interpret a YES/NO value
; In:  SI = value
; Out: AL = 1 for yes, 0 for no, CF = 1 if neither
; --------------------------------------------------------------------------
parse_yes_no:
        mov     al, [si]
        call    to_upper
        cmp     al, 'Y'
        je      .yes
        cmp     al, '1'
        je      .yes
        cmp     al, 'N'
        je      .no
        cmp     al, '0'
        je      .no
        stc
        ret
.yes:
        mov     al, 1
        clc
        ret
.no:
        xor     al, al
        clc
        ret

; --------------------------------------------------------------------------
; load_config - Read STOCK.CFG (creates it with defaults if missing)
; --------------------------------------------------------------------------
load_config:
        push    ax bx cx dx si di
        mov     dx, cfg_file
        call    fopen_read
        jnc     .line
        call    save_config             ; write the default configuration
        jmp     .done
.line:
        mov     di, line_buf
        mov     cx, LINE_MAX
        call    fgets
        jc      .eof
        mov     si, line_buf
        call    str_trim
        mov     al, [si]
        or      al, al
        jz      .line
        cmp     al, ';'
        je      .line
        cmp     al, '#'
        je      .line
        mov     di, tmp_buf             ; copy the key up to '='
        mov     cx, L_KEY
.key:
        lodsb
        or      al, al
        jz      .line                   ; no '=': ignore the line
        cmp     al, '='
        je      .key_end
        jcxz    .key
        call    to_upper
        mov     [di], al
        inc     di
        dec     cx
        jmp     .key
.key_end:
        mov     byte [di], 0
        call    str_trim                ; SI = value
        mov     bx, si
        mov     si, tmp_buf
        call    str_trim
        mov     di, key_company
        call    str_cmp
        je      .company
        mov     di, key_lowstock
        call    str_cmp
        je      .lowstock
        mov     di, key_confirm
        call    str_cmp
        je      .confirm
        mov     di, key_autosave
        call    str_cmp
        je      .autosave
        jmp     .line                   ; unknown key
.company:
        mov     si, bx
        mov     di, cfg_company
        mov     cx, L_COMPANY
        call    str_copy_n
        jmp     .line
.lowstock:
        mov     si, bx
        call    str_to_u16
        jc      .line
        mov     [cfg_lowstock], ax
        jmp     .line
.confirm:
        mov     si, bx
        call    parse_yes_no
        jc      .line
        mov     [cfg_confirm], al
        jmp     .line
.autosave:
        mov     si, bx
        call    parse_yes_no
        jc      .line
        mov     [cfg_autosave], al
        jmp     .line
.eof:
        call    fclose
.done:
        pop     di si dx cx bx ax
        ret

; --------------------------------------------------------------------------
; save_config - Write STOCK.CFG
; Out: CF = 1 if the file could not be written
; --------------------------------------------------------------------------
save_config:
        push    ax dx si di
        mov     dx, cfg_file
        call    fcreate
        jc      .done
        mov     si, cfg_header
        call    fputs
        mov     si, key_company
        call    fputs
        call    put_equals
        mov     si, cfg_company
        call    fputs
        call    fputnl
        mov     si, key_lowstock
        call    fputs
        call    put_equals
        mov     ax, [cfg_lowstock]
        mov     di, num_buf
        call    u16_to_str
        mov     si, num_buf
        call    fputs
        call    fputnl
        mov     si, key_confirm
        call    fputs
        call    put_equals
        mov     al, [cfg_confirm]
        call    put_yes_no
        mov     si, key_autosave
        call    fputs
        call    put_equals
        mov     al, [cfg_autosave]
        call    put_yes_no
        call    fclose_write
.done:
        pop     di si dx ax
        ret

put_equals:
        push    ax
        mov     al, '='
        call    fputc
        pop     ax
        ret

; In: AL = flag. Writes "YES" or "NO" and a line ending.
put_yes_no:
        push    si
        mov     si, str_yes
        or      al, al
        jnz     .write
        mov     si, str_no
.write:
        call    fputs
        call    fputnl
        pop     si
        ret
