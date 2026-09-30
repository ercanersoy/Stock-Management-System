; ==========================================================================
; UI.INC - Screen frame, message line, keyboard and the form engine
; ==========================================================================

; --------------------------------------------------------------------------
; set_msg - Show an information message on the message line
; In:  SI = message
; --------------------------------------------------------------------------
set_msg:
        mov     byte [msg_attr], ATTR_BRIGHT
        jmp     store_msg

; --------------------------------------------------------------------------
; set_error - Show an error message on the message line and beep
; In:  SI = message
; --------------------------------------------------------------------------
set_error:
        mov     byte [msg_attr], ATTR_REVERSE
        call    beep

store_msg:
        push    cx di
        mov     di, msg_buf
        mov     cx, SCR_COLS - 2
        call    str_copy_n
        pop     di cx
        ; fall through

; --------------------------------------------------------------------------
; draw_msg - Redraw the message line
; --------------------------------------------------------------------------
draw_msg:
        push    ax cx dx si
        mov     dx, ROW_MSG * 256
        mov     cx, SCR_COLS
        mov     ax, ATTR_NORMAL * 256 + ' '
        call    fill_chars
        cmp     byte [msg_buf], 0
        je      .done
        mov     ah, [msg_attr]
        mov     al, ' '
        call    put_char                ; small margin in the message colour
        inc     dl
        mov     si, msg_buf
        call    put_str
        call    put_char
.done:
        pop     si dx cx ax
        ret

; --------------------------------------------------------------------------
; clear_msg - Remove the current message
; --------------------------------------------------------------------------
clear_msg:
        mov     byte [msg_buf], 0
        jmp     draw_msg

; --------------------------------------------------------------------------
; draw_title - Draw the title bar (program, company, item count)
; --------------------------------------------------------------------------
draw_title:
        push    ax cx dx si
        mov     dx, ROW_TITLE * 256
        mov     cx, SCR_COLS
        mov     ax, ATTR_REVERSE * 256 + ' '
        call    fill_chars
        mov     dl, 1
        mov     si, str_title
        call    put_str
        mov     si, cfg_company         ; company name, centred
        call    str_len
        shr     cx, 1
        mov     dl, SCR_COLS / 2
        sub     dl, cl
        call    put_str
        call    sb_reset                ; right side: state and item count
        cmp     byte [modified], 0
        je      .count
        mov     si, str_modified
        call    sb_str
.count:
        mov     si, str_items
        call    sb_str
        mov     ax, [rec_count]
        call    sb_num
        mov     si, sb_buf
        mov     dl, SCR_COLS - 22
        mov     cx, 21
        mov     ah, ATTR_REVERSE
        call    put_field_right
        pop     si dx cx ax
        ret

; --------------------------------------------------------------------------
; draw_keys - Draw the function key bar, highlighting the active section
; --------------------------------------------------------------------------
draw_keys:
        push    ax bx cx dx si
        mov     dx, ROW_KEYS * 256
        mov     cx, SCR_COLS
        mov     ax, ATTR_NORMAL * 256 + ' '
        call    fill_chars
        mov     si, key_table
        xor     bx, bx
.item:
        push    si
        mov     si, [si]                ; key name, e.g. "F1"
        mov     ah, ATTR_NORMAL
        call    put_str
        pop     si
        push    si
        mov     si, [si + 2]            ; key label, e.g. " Search"
        mov     ah, ATTR_REVERSE
        cmp     bl, [cur_section]
        jne     .label
        mov     ah, ATTR_BRIGHT
.label:
        mov     cx, KEY_LABEL_WIDTH
        call    put_field
        add     dl, KEY_LABEL_WIDTH + 1
        pop     si
        add     si, 4
        inc     bx
        cmp     bx, KEY_COUNT
        jb      .item
        pop     si dx cx bx ax
        ret

; --------------------------------------------------------------------------
; draw_screen - Draw the complete frame for the current section
; --------------------------------------------------------------------------
draw_screen:
        push    ax bx cx dx si
        call    hide_cursor
        call    clear_screen
        call    draw_title
        mov     cx, ROW_BOX_TOP * 256
        mov     dx, ROW_BOX_BOTTOM * 256 + SCR_COLS - 1
        mov     ah, ATTR_NORMAL
        call    draw_box
        mov     bl, [cur_section]           ; section name in the top border
        xor     bh, bh
        shl     bx, 1
        mov     si, [section_names + bx]
        mov     dx, ROW_BOX_TOP * 256 + 2
        mov     ax, ATTR_NORMAL * 256 + 0B5h
        call    put_char
        inc     dl
        mov     ah, ATTR_BRIGHT
        call    put_str
        mov     ax, ATTR_NORMAL * 256 + 0C6h
        call    put_char
        call    draw_keys
        call    draw_msg
        pop     si dx cx bx ax
        ret

; --------------------------------------------------------------------------
; draw_separator - Draw a horizontal divider across the work window
; In:  DH = row
; --------------------------------------------------------------------------
draw_separator:
        push    ax bx cx dx
        xor     dl, dl
        mov     cx, SCR_COLS
        mov     ax, ATTR_NORMAL * 256 + 0C7h
        mov     bx, 0B6C4h
        call    hline
        pop     dx cx bx ax
        ret

; --------------------------------------------------------------------------
; draw_text_block - Draw a list of strings on consecutive rows
; In:  SI = list of zero terminated strings, ended by an empty string
;      DH = first row, DL = column, AH = attribute
; A string starting with the byte 1 is drawn highlighted (byte skipped).
; --------------------------------------------------------------------------
draw_text_block:
        push    ax dx si
.line:
        cmp     byte [si], 0
        je      .done
        push    ax dx
        cmp     byte [si], 1
        jne     .draw
        inc     si
        mov     ah, ATTR_BRIGHT
.draw:
        call    put_str
        pop     dx ax
.skip:
        lodsb                           ; move past the terminator
        or      al, al
        jnz     .skip
        inc     dh
        jmp     .line
.done:
        pop     si dx ax
        ret

; --------------------------------------------------------------------------
; get_key - Wait for a key. F9 (save) is handled here for every section.
; Out: AX = key code (AL = ASCII, AH = scan code)
; --------------------------------------------------------------------------
get_key:
        call    draw_title
        xor     ah, ah
        int     16h
        cmp     ax, KEY_F9
        jne     .done
        call    cmd_save
        jmp     get_key
.done:
        ret

; --------------------------------------------------------------------------
; check_section_key - Test for a section switching key
; In:  AX = key code
; Out: CF = 1: AL = new section (SEC_xxx) or SEC_EXIT for F10
;      CF = 0: not a section key, AX unchanged
; --------------------------------------------------------------------------
check_section_key:
        or      al, al
        jnz     .no
        cmp     ah, KEY_F1 shr 8
        jb      .no
        cmp     ah, KEY_F6 shr 8
        ja      .f10
        mov     al, ah
        sub     al, KEY_F1 shr 8
        stc
        ret
.f10:
        cmp     ah, KEY_F10 shr 8
        jne     .no
        mov     al, SEC_EXIT
        stc
        ret
.no:
        clc
        ret

; --------------------------------------------------------------------------
; ask_yes_no - Ask a question on the message line
; In:  SI = question
; Out: AL = 1 yes, 0 no, 2 Esc, 3 section key (AH = section)
; --------------------------------------------------------------------------
ask_yes_no:
        push    cx dx
        call    set_msg
        call    str_len
        mov     dx, ROW_MSG * 256 + 2
        add     dl, cl
        call    set_cursor
.key:
        call    get_key
        call    check_section_key
        jc      .switch
        cmp     al, KEY_ESC
        je      .escape
        call    to_upper
        cmp     al, 'Y'
        je      .yes
        cmp     al, 'N'
        je      .no
        call    beep
        jmp     .key
.yes:
        mov     al, 1
        jmp     .done
.no:
        xor     al, al
        jmp     .done
.escape:
        mov     al, 2
        jmp     .done
.switch:
        mov     ah, al
        mov     al, 3
.done:
        call    hide_cursor
        call    clear_msg
        pop     dx cx
        ret

; ==========================================================================
; Form engine
;
; A form is a count byte followed by FIELD descriptors (see CONST.INC).
; Labels are drawn at COL_LABEL, the input at the field's own column.
; ==========================================================================

; --------------------------------------------------------------------------
; draw_field - Draw the input area of one field
; In:  BX = field descriptor, AH = attribute
; --------------------------------------------------------------------------
draw_field:
        push    cx dx si
        mov     dh, [bx + F_ROW]
        mov     dl, [bx + F_COL]
        mov     si, [bx + F_BUF]
        mov     cl, [bx + F_MAX]
        xor     ch, ch
        inc     cx                      ; room for the cursor
        call    put_field
        pop     si dx cx
        ret

; --------------------------------------------------------------------------
; draw_form - Draw all labels and fields of a form
; In:  SI = form
; --------------------------------------------------------------------------
draw_form:
        push    ax bx cx dx si
        mov     cl, [si]
        xor     ch, ch
        lea     bx, [si + 1]
.field:
        mov     dh, [bx + F_ROW]
        mov     dl, COL_LABEL
        mov     si, [bx + F_LABEL]
        mov     ah, ATTR_NORMAL
        call    put_str
        mov     ah, ATTR_UNDERLINE
        call    draw_field
        add     bx, FIELD_SIZE
        loop    .field
        pop     si dx cx bx ax
        ret

; --------------------------------------------------------------------------
; show_form - Draw a form with plain (not editable looking) values
; In:  SI = form
; --------------------------------------------------------------------------
show_form:
        push    ax bx cx dx si
        mov     cl, [si]
        xor     ch, ch
        lea     bx, [si + 1]
.field:
        mov     dh, [bx + F_ROW]
        mov     dl, COL_LABEL
        mov     si, [bx + F_LABEL]
        mov     ah, ATTR_NORMAL
        call    put_str
        mov     ah, ATTR_BRIGHT
        call    draw_field
        add     bx, FIELD_SIZE
        loop    .field
        pop     si dx cx bx ax
        ret

; --------------------------------------------------------------------------
; clear_form - Empty every edit buffer of a form
; In:  SI = form
; --------------------------------------------------------------------------
clear_form:
        push    bx cx di
        mov     cl, [si]
        xor     ch, ch
        lea     bx, [si + 1]
.field:
        mov     di, [bx + F_BUF]
        mov     byte [di], 0
        add     bx, FIELD_SIZE
        loop    .field
        pop     di cx bx
        ret

; --------------------------------------------------------------------------
; edit_form - Let the user fill in a form
;   Enter / Tab  : next field (Enter on the last field submits the form)
;   Shift+Tab    : previous field
;   Backspace    : delete the last character
;   Esc          : cancel
; In:  SI = form, AL = index of the first field to edit
; Out: AL = FORM_SUBMIT, FORM_ESC or FORM_SWITCH (AH = section)
; --------------------------------------------------------------------------
edit_form:
        push    bx cx dx
        mov     [ef_index], al
.select:
        mov     al, [ef_index]
        xor     ah, ah
        mov     cl, FIELD_SHIFT
        shl     ax, cl
        lea     bx, [si + 1]
        add     bx, ax                  ; BX = active field
        mov     ah, ATTR_REVERSE
        call    draw_field
.key:
        call    field_cursor
        call    get_key
        call    check_section_key
        jc      .switch
        cmp     al, KEY_ESC
        je      .escape
        cmp     al, KEY_ENTER
        je      .enter
        cmp     al, KEY_TAB
        je      .next
        cmp     ax, KEY_SHIFT_TAB
        je      .previous
        cmp     al, KEY_BACKSPACE
        je      .backspace
        cmp     al, ' '
        jb      .key
        cmp     al, '~'
        ja      .key
        call    field_insert
        jmp     .key
.backspace:
        call    field_backspace
        jmp     .key
.enter:
        mov     al, [ef_index]
        inc     al
        cmp     al, [si]
        jb      .move
        mov     al, FORM_SUBMIT
        jmp     .leave
.next:
        mov     al, [ef_index]
        inc     al
        cmp     al, [si]
        jb      .move
        xor     al, al                  ; wrap to the first field
        jmp     .move
.previous:
        mov     al, [ef_index]
        or      al, al
        jnz     .prev_ok
        mov     al, [si]                ; wrap to the last field
.prev_ok:
        dec     al
.move:
        push    ax
        mov     ah, ATTR_UNDERLINE
        call    draw_field
        pop     ax
        mov     [ef_index], al
        jmp     .select
.escape:
        mov     al, FORM_ESC
        jmp     .leave
.switch:
        mov     ah, al
        mov     al, FORM_SWITCH
.leave:
        push    ax
        mov     ah, ATTR_UNDERLINE
        call    draw_field
        pop     ax
        call    hide_cursor
        pop     dx cx bx
        ret

; --------------------------------------------------------------------------
; field_cursor - Place the cursor after the text of a field
; In:  BX = field
; --------------------------------------------------------------------------
field_cursor:
        push    cx dx si
        mov     si, [bx + F_BUF]
        call    str_len
        mov     dh, [bx + F_ROW]
        mov     dl, [bx + F_COL]
        add     dl, cl
        call    set_cursor
        pop     si dx cx
        ret

; --------------------------------------------------------------------------
; field_insert - Append a typed character to a field, if it is allowed
; In:  BX = field, AL = character
; --------------------------------------------------------------------------
field_insert:
        push    ax cx si di
        mov     si, [bx + F_BUF]
        call    str_len                 ; CX = current length
        cmp     al, ';'                 ; reserved as the DATA.DAT separator
        je      .reject
        mov     ah, [bx + F_TYPE]
        cmp     ah, FT_YESNO
        je      .yes_no
        cmp     ah, FT_CODE
        je      .code
        cmp     ah, FT_NUM
        je      .digit
        cmp     ah, FT_QTY
        je      .qty
        cmp     ah, FT_PRICE
        je      .price
        jmp     .append                 ; FT_TEXT
.code:
        call    to_upper
        call    is_code_char
        jc      .reject
        jmp     .append
.qty:
        jcxz    .sign                   ; a sign is allowed at the start
        jmp     .digit
.sign:
        cmp     al, '+'
        je      .append
        cmp     al, '-'
        je      .append
        jmp     .digit
.price:
        cmp     al, '.'
        jne     .digit
        push    di
        mov     di, si                  ; only one decimal point
.find_point:
        cmp     byte [di], 0
        je      .no_point
        cmp     byte [di], '.'
        je      .has_point
        inc     di
        jmp     .find_point
.has_point:
        pop     di
        jmp     .reject
.no_point:
        pop     di
        jmp     .append
.digit:
        cmp     al, '0'
        jb      .reject
        cmp     al, '9'
        ja      .reject
.append:
        cmp     cl, [bx + F_MAX]
        jae     .reject
        mov     di, si
        add     di, cx
        mov     [di], al
        mov     byte [di + 1], 0
        jmp     .redraw
.yes_no:
        call    to_upper                ; Y or N replaces the value
        cmp     al, 'Y'
        je      .set_yes_no
        cmp     al, 'N'
        jne     .reject
.set_yes_no:
        mov     [si], al
        mov     byte [si + 1], 0
.redraw:
        mov     ah, ATTR_REVERSE
        call    draw_field
        jmp     .done
.reject:
        call    beep
.done:
        pop     di si cx ax
        ret

; --------------------------------------------------------------------------
; field_backspace - Delete the last character of a field
; In:  BX = field
; --------------------------------------------------------------------------
field_backspace:
        push    ax cx si
        mov     si, [bx + F_BUF]
        call    str_len
        jcxz    .done
        add     si, cx
        mov     byte [si - 1], 0
        mov     ah, ATTR_REVERSE
        call    draw_field
.done:
        pop     si cx ax
        ret
