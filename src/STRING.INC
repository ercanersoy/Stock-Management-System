; ==========================================================================
; STRING.INC - String, number and string builder routines
;
; Strings are zero terminated and live in the program segment (DS = ES).
; ==========================================================================

; --------------------------------------------------------------------------
; str_len - Length of a string
; In:  SI = string
; Out: CX = length
; --------------------------------------------------------------------------
str_len:
        push    si
        xor     cx, cx
.next:
        cmp     byte [si], 0
        je      .done
        inc     si
        inc     cx
        jmp     .next
.done:
        pop     si
        ret

; --------------------------------------------------------------------------
; str_copy - Copy a string
; In:  SI = source, DI = destination
; --------------------------------------------------------------------------
str_copy:
        push    ax si di
.next:
        lodsb
        mov     [di], al
        inc     di
        or      al, al
        jnz     .next
        pop     di si ax
        ret

; --------------------------------------------------------------------------
; str_copy_n - Copy at most CX characters of a string (always terminated)
; In:  SI = source, DI = destination, CX = maximum length
; --------------------------------------------------------------------------
str_copy_n:
        push    ax cx si di
        jcxz    .done
.next:
        lodsb
        or      al, al
        jz      .done
        mov     [di], al
        inc     di
        loop    .next
.done:
        mov     byte [di], 0
        pop     di si cx ax
        ret

; --------------------------------------------------------------------------
; str_cmp - Compare two strings (case sensitive)
; In:  SI = first string, DI = second string
; Out: flags as after CMP first, second (ZF = equal, CF = first < second)
; --------------------------------------------------------------------------
str_cmp:
        push    ax si di
.next:
        mov     al, [si]
        cmp     al, [di]
        jne     .done
        or      al, al                  ; end of both strings: ZF=1, CF=0
        jz      .done
        inc     si
        inc     di
        jmp     .next
.done:
        pop     di si ax
        ret

; --------------------------------------------------------------------------
; to_upper - Convert a character to upper case
; In/Out: AL = character
; --------------------------------------------------------------------------
to_upper:
        cmp     al, 'a'
        jb      .done
        cmp     al, 'z'
        ja      .done
        sub     al, 'a' - 'A'
.done:
        ret

; --------------------------------------------------------------------------
; str_upper - Convert a string to upper case in place
; In:  SI = string
; --------------------------------------------------------------------------
str_upper:
        push    ax si
.next:
        mov     al, [si]
        or      al, al
        jz      .done
        call    to_upper
        mov     [si], al
        inc     si
        jmp     .next
.done:
        pop     si ax
        ret

; --------------------------------------------------------------------------
; str_trim - Remove leading and trailing spaces in place
; In:  SI = string
; --------------------------------------------------------------------------
str_trim:
        push    ax bx si di
        mov     bx, si                  ; BX = start of the string
        mov     di, si
.lead:
        cmp     byte [si], ' '
        jne     .copy
        inc     si
        jmp     .lead
.copy:
        lodsb
        mov     [di], al
        inc     di
        or      al, al
        jnz     .copy
        dec     di                      ; DI -> terminator
.trail:
        cmp     di, bx
        je      .done
        cmp     byte [di - 1], ' '
        jne     .done
        dec     di
        mov     byte [di], 0
        jmp     .trail
.done:
        pop     di si bx ax
        ret

; --------------------------------------------------------------------------
; str_find_ci - Case insensitive substring search
; In:  SI = text, DI = pattern (must already be upper case)
; Out: CF = 0 if the pattern occurs in the text, CF = 1 otherwise
; --------------------------------------------------------------------------
str_find_ci:
        push    ax bx si di
        cmp     byte [di], 0
        je      .found                  ; an empty pattern always matches
.outer:
        cmp     byte [si], 0
        je      .not_found
        mov     bx, si
        push    di
.inner:
        mov     ah, [di]
        or      ah, ah
        jz      .match
        mov     al, [bx]
        or      al, al
        jz      .exhausted
        call    to_upper
        cmp     al, ah
        jne     .mismatch
        inc     bx
        inc     di
        jmp     .inner
.mismatch:
        pop     di
        inc     si
        jmp     .outer
.exhausted:
        pop     di
        jmp     .not_found
.match:
        pop     di
.found:
        clc
        jmp     .done
.not_found:
        stc
.done:
        pop     di si bx ax
        ret

; --------------------------------------------------------------------------
; str_to_u16 - Parse an unsigned decimal number (0..65535)
; In:  SI = string (digits only)
; Out: AX = value, CF = 1 if empty, not a number or out of range
; --------------------------------------------------------------------------
str_to_u16:
        push    bx cx dx si
        xor     ax, ax
        xor     cx, cx
        xor     bh, bh
.next:
        mov     bl, [si]
        or      bl, bl
        jz      .end
        sub     bl, '0'
        jb      .error
        cmp     bl, 9
        ja      .error
        mov     dx, 10
        mul     dx
        jc      .error                  ; DX <> 0: overflow
        add     ax, bx
        jc      .error
        inc     cx
        inc     si
        jmp     .next
.end:
        jcxz    .error
        clc
        jmp     .done
.error:
        stc
.done:
        pop     si dx cx bx
        ret

; --------------------------------------------------------------------------
; u16_to_str - Convert an unsigned number to decimal text
; In:  AX = value, DI = destination buffer (at least 6 bytes)
; --------------------------------------------------------------------------
u16_to_str:
        push    ax bx cx dx di
        mov     bx, 10
        xor     cx, cx
.divide:
        xor     dx, dx
        div     bx
        push    dx
        inc     cx
        or      ax, ax
        jnz     .divide
.store:
        pop     ax
        add     al, '0'
        mov     [di], al
        inc     di
        loop    .store
        mov     byte [di], 0
        pop     di dx cx bx ax
        ret

; --------------------------------------------------------------------------
; parse_qty - Parse a quantity entry
;   ""      -> base value (unchanged)
;   "n"     -> n
;   "+n"    -> base + n
;   "-n"    -> base - n
; In:  SI = string, BX = base value
; Out: AX = quantity, CF = 1 if invalid or out of range
; --------------------------------------------------------------------------
parse_qty:
        push    si
        cmp     byte [si], 0
        je      .base
        cmp     byte [si], '+'
        je      .add
        cmp     byte [si], '-'
        je      .sub
        call    str_to_u16
        jmp     .done
.base:
        mov     ax, bx
        clc
        jmp     .done
.add:
        inc     si
        call    str_to_u16
        jc      .done
        add     ax, bx                  ; CF = 1 on overflow
        jmp     .done
.sub:
        inc     si
        call    str_to_u16
        jc      .done
        push    bx
        sub     bx, ax                  ; CF = 1 if n > base
        mov     ax, bx
        pop     bx
.done:
        pop     si
        ret

; --------------------------------------------------------------------------
; normalize_price - Validate a price and format it with two decimals
;   "12" -> "12.00", "3.5" -> "3.50", ".75" -> "0.75", "" -> "0.00"
; In:  SI = input string, DI = output buffer (L_PRICE + 1 bytes)
; Out: CF = 1 if the input is not a valid price
; --------------------------------------------------------------------------
normalize_price:
        push    ax bx cx si di
        xor     cx, cx                  ; integer digits written
.zeros:
        cmp     byte [si], '0'          ; drop leading zeros
        jne     .int
        inc     si
        jmp     .zeros
.int:
        mov     al, [si]
        cmp     al, '0'
        jb      .int_end
        cmp     al, '9'
        ja      .int_end
        cmp     cx, L_PRICE - 3
        jae     .error
        mov     [di], al
        inc     di
        inc     cx
        inc     si
        jmp     .int
.int_end:
        or      cx, cx
        jnz     .point
        mov     byte [di], '0'
        inc     di
.point:
        mov     byte [di], '.'
        inc     di
        mov     bx, 2                   ; decimals still allowed
        or      al, al
        jz      .pad
        cmp     al, '.'
        jne     .error
        inc     si
.decimals:
        mov     al, [si]
        or      al, al
        jz      .pad
        cmp     al, '0'
        jb      .error
        cmp     al, '9'
        ja      .error
        or      bx, bx
        jz      .error
        mov     [di], al
        inc     di
        inc     si
        dec     bx
        jmp     .decimals
.pad:
        or      bx, bx
        jz      .finish
        mov     byte [di], '0'
        inc     di
        dec     bx
        jmp     .pad
.finish:
        mov     byte [di], 0
        clc
        jmp     .done
.error:
        stc
.done:
        pop     di si cx bx ax
        ret

; --------------------------------------------------------------------------
; String builder: assembles one line of text in sb_buf.
; --------------------------------------------------------------------------

; sb_reset - Start a new, empty string
sb_reset:
        mov     word [sb_ptr], sb_buf
        mov     byte [sb_buf], 0
        ret

; sb_char - Append the character in AL
sb_char:
        push    di
        mov     di, [sb_ptr]
        cmp     di, sb_buf + SCR_COLS
        jae     .done
        mov     [di], al
        inc     di
        mov     byte [di], 0
        mov     [sb_ptr], di
.done:
        pop     di
        ret

; sb_str - Append the string at SI
sb_str:
        push    ax si
.next:
        lodsb
        or      al, al
        jz      .done
        call    sb_char
        jmp     .next
.done:
        pop     si ax
        ret

; sb_num - Append the unsigned number in AX
sb_num:
        push    si di
        mov     di, num_buf
        call    u16_to_str
        mov     si, num_buf
        call    sb_str
        pop     di si
        ret
