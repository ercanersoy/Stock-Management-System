; ==========================================================================
; DB.INC - In-memory stock database and DATA.DAT load/save
;
; Records are kept in their own DOS memory block (rec_seg), sorted by item
; code, REC_SIZE bytes each. The rest of the program works on copies of
; records held in the program segment.
;
; DATA.DAT is a plain text file, one item per line:
;       CODE;NAME;QUANTITY;UNIT;PRICE
; Empty lines and lines starting with '#' are ignored.
; ==========================================================================

; --------------------------------------------------------------------------
; load_rec - Copy a record from the database into the program segment
; In:  AX = record index, DI = destination (REC_SIZE bytes)
; --------------------------------------------------------------------------
load_rec:
        push    ax cx si di ds es
        push    ds
        pop     es
        mov     cl, REC_SHIFT
        shl     ax, cl
        mov     si, ax
        mov     ds, [rec_seg]
        mov     cx, REC_SIZE / 2
        rep     movsw
        pop     es ds di si cx ax
        ret

; --------------------------------------------------------------------------
; store_rec - Copy a record from the program segment into the database
; In:  AX = record index, SI = source (REC_SIZE bytes)
; --------------------------------------------------------------------------
store_rec:
        push    ax cx si di es
        mov     cl, REC_SHIFT
        shl     ax, cl
        mov     di, ax
        mov     es, [rec_seg]
        mov     cx, REC_SIZE / 2
        rep     movsw
        pop     es di si cx ax
        ret

; --------------------------------------------------------------------------
; find_code - Binary search for an item code
; In:  SI = code (upper case)
; Out: CF = 0: found, AX = record index
;      CF = 1: not found, AX = index where the code would be inserted
; --------------------------------------------------------------------------
find_code:
        push    bx dx di
        xor     bx, bx                  ; BX = low bound
        mov     dx, [rec_count]         ; DX = high bound (exclusive)
        mov     di, key_rec
.search:
        cmp     bx, dx
        jae     .not_found
        mov     ax, bx
        add     ax, dx
        shr     ax, 1                   ; AX = middle
        call    load_rec
        call    str_cmp                 ; compare code with key_rec code
        je      .found
        jb      .lower
        inc     ax
        mov     bx, ax                  ; search the upper half
        jmp     .search
.lower:
        mov     dx, ax                  ; search the lower half
        jmp     .search
.found:
        clc
        jmp     .done
.not_found:
        mov     ax, bx
        stc
.done:
        pop     di dx bx
        ret

; --------------------------------------------------------------------------
; insert_rec - Insert a record keeping the database sorted
; In:  SI = record (code must be upper case and not empty)
; Out: CF = 0: inserted, AX = new record index
;      CF = 1: failed, AL = 1 duplicate code, AL = 2 database full
; --------------------------------------------------------------------------
insert_rec:
        push    bx cx dx si di ds es
        cmp     word [rec_count], MAX_RECS
        jae     .full
        call    find_code
        jnc     .duplicate
        mov     bx, ax                  ; BX = insert position
        mov     ax, [rec_count]
        sub     ax, bx                  ; AX = records to move up
        jz      .store
        mov     cl, REC_SHIFT - 1
        shl     ax, cl
        mov     dx, ax                  ; DX = words to move
        mov     ax, [rec_count]
        mov     cl, REC_SHIFT
        shl     ax, cl
        push    si
        mov     si, ax
        sub     si, 2                   ; last word of the last record
        mov     di, si
        add     di, REC_SIZE
        mov     cx, dx
        mov     ax, [rec_seg]
        push    ds
        mov     ds, ax
        mov     es, ax
        std
        rep     movsw
        cld
        pop     ds
        pop     si
.store:
        mov     ax, bx
        call    store_rec
        inc     word [rec_count]
        mov     byte [modified], 1
        clc
        jmp     .done
.duplicate:
        mov     al, 1
        stc
        jmp     .done
.full:
        mov     al, 2
        stc
.done:
        pop     es ds di si dx cx bx
        ret

; --------------------------------------------------------------------------
; delete_rec - Remove a record
; In:  AX = record index
; --------------------------------------------------------------------------
delete_rec:
        push    ax bx cx si di ds es
        mov     bx, [rec_count]
        dec     bx
        sub     bx, ax                  ; BX = records after the deleted one
        mov     cl, REC_SHIFT
        shl     ax, cl
        mov     di, ax
        mov     si, ax
        add     si, REC_SIZE
        mov     ax, bx
        mov     cl, REC_SHIFT - 1
        shl     ax, cl
        mov     cx, ax                  ; CX = words to move down
        mov     ax, [rec_seg]
        push    ds
        mov     ds, ax
        mov     es, ax
        rep     movsw
        pop     ds
        dec     word [rec_count]
        mov     byte [modified], 1
        pop     es ds di si cx bx ax
        ret

; --------------------------------------------------------------------------
; clear_rec - Fill a record buffer with zeros
; In:  DI = record buffer
; --------------------------------------------------------------------------
clear_rec:
        push    ax cx di
        xor     ax, ax
        mov     cx, REC_SIZE / 2
        rep     stosw
        pop     di cx ax
        ret

; --------------------------------------------------------------------------
; get_field - Extract one ';' separated field from a text line
; In:  SI = current position in the line, DI = destination, CX = max length
; Out: SI = start of the next field, destination trimmed and terminated
; --------------------------------------------------------------------------
get_field:
        push    ax cx di
        push    di
.next:
        lodsb
        or      al, al
        jz      .end_line
        cmp     al, ';'
        je      .end
        jcxz    .next                   ; too long: drop the rest
        mov     [di], al
        inc     di
        dec     cx
        jmp     .next
.end_line:
        dec     si                      ; stay on the terminator
.end:
        mov     byte [di], 0
        pop     di
        xchg    si, di
        call    str_trim
        xchg    si, di
        pop     di cx ax
        ret

; --------------------------------------------------------------------------
; parse_line - Convert a DATA.DAT line into a record
; In:  line_buf = text line
; Out: io_rec = record
;      CF = 1 and AL = 0 : line is empty or a comment
;      CF = 1 and AL = 1 : line is invalid
; --------------------------------------------------------------------------
parse_line:
        push    cx si di
        mov     di, io_rec
        call    clear_rec
        mov     si, line_buf
        call    str_trim
        mov     al, [si]
        or      al, al
        jz      .skip
        cmp     al, '#'
        je      .skip
        mov     di, io_rec + R_CODE
        mov     cx, L_CODE
        call    get_field
        mov     di, io_rec + R_NAME
        mov     cx, L_NAME
        call    get_field
        mov     di, tmp_buf
        mov     cx, L_QTY
        call    get_field
        mov     di, io_rec + R_UNIT
        mov     cx, L_UNIT
        call    get_field
        mov     di, tmp_buf2
        mov     cx, L_PRICE
        call    get_field
        mov     si, io_rec + R_CODE     ; the code is required
        cmp     byte [si], 0
        je      .invalid
        call    str_upper
        call    check_code
        jc      .invalid
        mov     si, tmp_buf             ; quantity
        call    str_to_u16
        jc      .invalid
        mov     word [io_rec + R_QTY], ax
        mov     si, tmp_buf2            ; price
        mov     di, io_rec + R_PRICE
        call    normalize_price
        jc      .invalid
        clc
        jmp     .done
.skip:
        xor     al, al
        stc
        jmp     .done
.invalid:
        mov     al, 1
        stc
.done:
        pop     di si cx
        ret

; --------------------------------------------------------------------------
; is_code_char - Check whether a character may be used in an item code
; In:  AL = character (upper case)
; Out: CF = 1 if not allowed
; --------------------------------------------------------------------------
is_code_char:
        cmp     al, 'A'
        jb      .not_letter
        cmp     al, 'Z'
        jbe     .ok
.not_letter:
        cmp     al, '0'
        jb      .symbol
        cmp     al, '9'
        jbe     .ok
.symbol:
        cmp     al, '-'
        je      .ok
        cmp     al, '_'
        je      .ok
        cmp     al, '.'
        je      .ok
        cmp     al, '/'
        je      .ok
        stc
        ret
.ok:
        clc
        ret

; --------------------------------------------------------------------------
; check_code - Check that every character of a code is allowed
; In:  SI = code (upper case)
; Out: CF = 1 if the code contains an invalid character
; --------------------------------------------------------------------------
check_code:
        push    ax si
.next:
        lodsb
        or      al, al
        jz      .done                   ; CF = 0 after OR
        call    is_code_char
        jnc     .next
.done:
        pop     si ax
        ret

; --------------------------------------------------------------------------
; load_db - Load DATA.DAT into memory
; Out: message line set to the result of the operation
; --------------------------------------------------------------------------
load_db:
        push    ax cx dx si di
        mov     word [rec_count], 0
        mov     word [skipped], 0
        mov     dx, data_file
        call    fopen_read
        jc      .no_file
.line:
        mov     di, line_buf
        mov     cx, LINE_MAX
        call    fgets
        jc      .eof
        call    parse_line
        jnc     .insert
        or      al, al
        jz      .line                   ; comment or empty line
        inc     word [skipped]
        jmp     .line
.insert:
        mov     si, io_rec
        call    insert_rec
        jnc     .line
        inc     word [skipped]          ; duplicate code or database full
        jmp     .line
.eof:
        call    fclose
        mov     byte [modified], 0
        call    sb_reset
        mov     si, msg_loaded
        call    sb_str
        mov     ax, [rec_count]
        call    sb_num
        mov     si, msg_loaded2
        call    sb_str
        mov     ax, [skipped]
        or      ax, ax
        jz      .show
        mov     si, msg_skipped
        call    sb_str
        call    sb_num
        mov     si, msg_skipped2
        call    sb_str
        mov     si, sb_buf
        call    set_error
        jmp     .done
.show:
        mov     si, sb_buf
        call    set_msg
        jmp     .done
.no_file:
        mov     byte [modified], 0
        mov     si, msg_new_db
        call    set_msg
.done:
        pop     di si dx cx ax
        ret

; --------------------------------------------------------------------------
; save_db - Write the database to DATA.DAT
; Out: CF = 1 if the file could not be written
; --------------------------------------------------------------------------
save_db:
        push    ax dx si di
        mov     dx, data_file
        call    fcreate
        jc      .done
        mov     si, db_header
        call    fputs
        xor     ax, ax
.next:
        cmp     ax, [rec_count]
        jae     .close
        mov     di, io_rec
        call    load_rec
        mov     si, io_rec + R_CODE
        call    fputs
        call    put_separator
        mov     si, io_rec + R_NAME
        call    fputs
        call    put_separator
        push    ax
        mov     ax, word [io_rec + R_QTY]
        mov     di, num_buf
        call    u16_to_str
        pop     ax
        mov     si, num_buf
        call    fputs
        call    put_separator
        mov     si, io_rec + R_UNIT
        call    fputs
        call    put_separator
        mov     si, io_rec + R_PRICE
        call    fputs
        call    fputnl
        inc     ax
        jmp     .next
.close:
        call    fclose_write
        jc      .done
        mov     byte [modified], 0
.done:
        pop     di si dx ax
        ret

put_separator:
        push    ax
        mov     al, ';'
        call    fputc
        pop     ax
        ret
