; ==========================================================================
; SECTIONS.INC - The program sections selected with F1..F6
;
; Every section draws its own screen and runs until a section key is
; pressed. It returns the next section (SEC_xxx or SEC_EXIT) in AL.
; ==========================================================================

; --------------------------------------------------------------------------
; Shared helpers
; --------------------------------------------------------------------------

; rec_to_form - Copy a record into the item form buffers
; In:  SI = record
rec_to_form:
        push    ax cx si di
        push    si
        add     si, R_CODE
        mov     di, f_code
        call    str_copy
        pop     si
        push    si
        add     si, R_NAME
        mov     di, f_name
        call    str_copy
        pop     si
        push    si
        add     si, R_UNIT
        mov     di, f_unit
        call    str_copy
        pop     si
        push    si
        add     si, R_PRICE
        mov     di, f_price
        call    str_copy
        pop     si
        mov     ax, [si + R_QTY]
        mov     di, f_qty
        call    u16_to_str
        pop     di si cx ax
        ret

; form_to_rec - Validate the item form and build edit_rec from it
; In:  BX = base quantity (used for empty, +n and -n quantity entries)
; Out: CF = 0: edit_rec filled
;      CF = 1: SI = error message, AL = index of the field in error
form_to_rec:
        push    cx di
        mov     di, edit_rec
        call    clear_rec
        mov     si, f_code
        call    str_trim
        call    str_upper
        mov     si, f_name
        call    str_trim
        mov     si, f_unit
        call    str_trim
        mov     si, f_qty
        call    str_trim
        mov     si, f_price
        call    str_trim
        cmp     byte [f_code], 0
        je      .no_code
        cmp     byte [f_name], 0
        je      .no_name
        mov     si, f_qty
        call    parse_qty
        jc      .bad_qty
        mov     word [edit_rec + R_QTY], ax
        mov     si, f_price
        mov     di, edit_rec + R_PRICE
        call    normalize_price
        jc      .bad_price
        mov     si, f_code
        mov     di, edit_rec + R_CODE
        mov     cx, L_CODE
        call    str_copy_n
        mov     si, f_name
        mov     di, edit_rec + R_NAME
        mov     cx, L_NAME
        call    str_copy_n
        mov     si, f_unit
        mov     di, edit_rec + R_UNIT
        mov     cx, L_UNIT
        call    str_copy_n
        clc
        jmp     .done
.no_code:
        mov     si, err_code_required
        xor     al, al
        jmp     .error
.no_name:
        mov     si, err_name_required
        mov     al, 1
        jmp     .error
.bad_qty:
        mov     si, err_quantity
        mov     al, 2
        jmp     .error
.bad_price:
        mov     si, err_price
        mov     al, 4
.error:
        stc
.done:
        pop     di cx
        ret

; item_message - Build "<prefix><code><suffix>" in sb_buf
; In:  SI = prefix, DI = suffix, BX = code
; Out: SI = sb_buf
item_message:
        call    sb_reset
        call    sb_str
        mov     si, bx
        call    sb_str
        mov     si, di
        call    sb_str
        mov     si, sb_buf
        ret

; finish_change - Report a successful change, saving first if AUTOSAVE is on
; In:  SI = message describing the change
finish_change:
        cmp     byte [cfg_autosave], 0
        je      .report
        call    save_db
        jc      .failed
.report:
        call    set_msg
        ret
.failed:
        mov     si, err_autosave
        call    set_error
        ret

; lookup_item - Find the item typed into the lookup field
; Out: CF = 0: found, cur_index set, view_rec loaded, item form filled
;      CF = 1: not found (error message shown)
lookup_item:
        push    ax bx di si
        mov     si, f_find
        call    str_trim
        call    str_upper
        cmp     byte [si], 0
        je      .empty
        call    find_code
        jc      .not_found
        mov     [cur_index], ax
        mov     di, view_rec
        call    load_rec
        mov     si, view_rec
        call    rec_to_form
        clc
        jmp     .done
.empty:
        mov     si, err_code_required
        call    set_error
        stc
        jmp     .done
.not_found:
        mov     bx, f_find
        mov     si, err_not_found
        mov     di, str_period
        call    item_message
        call    set_error
        stc
.done:
        pop     si di bx ax
        ret

; draw_item_screen - Common layout of the Add/Update/Delete sections
; In:  SI = hint lines (for draw_text_block)
draw_item_screen:
        push    ax dx
        call    draw_screen
        mov     dh, 4
        call    draw_separator
        mov     dh, 16
        call    draw_separator
        mov     dx, 17 * 256 + COL_LABEL
        mov     ah, ATTR_NORMAL
        call    draw_text_block
        pop     dx ax
        ret

; --------------------------------------------------------------------------
; F1 - Search
; --------------------------------------------------------------------------
sec_search:
        call    draw_screen
        mov     si, form_search
        call    draw_form
        mov     si, str_search_hint
        mov     dx, 3 * 256 + 46
        mov     ah, ATTR_NORMAL
        call    put_str
        mov     si, str_header          ; column headings
        mov     dh, 5
        mov     ah, ATTR_BRIGHT
        call    draw_columns
        mov     dh, 6
        call    draw_separator
        mov     dh, 20
        call    draw_separator
        mov     word [page_top], 0
        call    run_search
.redraw:
        call    draw_results
.key:
        mov     bx, form_search + 1
        mov     ah, ATTR_REVERSE
        call    draw_field
        call    field_cursor
        call    get_key
        call    check_section_key
        jc      .switch
        cmp     ax, KEY_PGDN
        je      .page_down
        cmp     ax, KEY_PGUP
        je      .page_up
        cmp     al, KEY_ESC
        je      .clear
        cmp     al, KEY_BACKSPACE
        je      .backspace
        cmp     al, ' '
        jb      .key
        cmp     al, '~'
        ja      .key
        call    field_insert
        jmp     .changed
.backspace:
        call    field_backspace
        jmp     .changed
.clear:
        mov     byte [f_query], 0
.changed:
        mov     word [page_top], 0
        call    run_search
        jmp     .redraw
.page_down:
        mov     ax, [page_top]
        add     ax, RES_ROWS
        cmp     ax, [match_count]
        jae     .key
        mov     [page_top], ax
        jmp     .redraw
.page_up:
        mov     ax, [page_top]
        or      ax, ax
        jz      .key
        sub     ax, RES_ROWS
        jnc     .set_page
        xor     ax, ax
.set_page:
        mov     [page_top], ax
        jmp     .redraw
.switch:
        call    hide_cursor
        ret

; run_search - Collect the indexes of all items whose code or name
;              contains the search text
run_search:
        push    ax bx si di
        mov     si, f_query
        mov     di, needle
        call    str_copy
        mov     si, needle
        call    str_upper
        xor     ax, ax
        xor     bx, bx
.next:
        cmp     ax, [rec_count]
        jae     .done
        mov     di, view_rec
        call    load_rec
        mov     di, needle
        mov     si, view_rec + R_CODE
        call    str_find_ci
        jnc     .match
        mov     si, view_rec + R_NAME
        call    str_find_ci
        jc      .skip
.match:
        mov     [match_idx + bx], ax
        add     bx, 2
.skip:
        inc     ax
        jmp     .next
.done:
        shr     bx, 1
        mov     [match_count], bx
        pop     di si bx ax
        ret

; draw_columns - Draw one result row from five strings
; In:  SI = code, name, quantity, unit, price, note (six strings in a row)
;      DH = row, AH = attribute
draw_columns:
        push    cx dx si
        call    clear_row
        mov     dl, 2
        mov     cx, L_CODE
        call    put_field
        call    .next_string
        mov     dl, 13
        mov     cx, L_NAME
        call    put_field
        call    .next_string
        mov     dl, 44
        mov     cx, 7
        call    put_field_right
        call    .next_string
        mov     dl, 53
        mov     cx, L_UNIT
        call    put_field
        call    .next_string
        mov     dl, 60
        mov     cx, 12
        call    put_field_right
        call    .next_string
        mov     dl, 74
        mov     cx, 5
        call    put_field
        pop     si dx cx
        ret
.next_string:
        push    ax
.skip:
        lodsb
        or      al, al
        jnz     .skip
        pop     ax
        ret

; draw_results - Draw the current page of search results and the status
draw_results:
        push    ax bx cx dx si di
        mov     dh, RES_TOP
        mov     bx, [page_top]
.row:
        cmp     bx, [match_count]
        jae     .blank
        push    bx
        shl     bx, 1
        mov     ax, [match_idx + bx]
        pop     bx
        mov     di, view_rec
        call    load_rec
        call    build_row               ; row strings in row_buf
        mov     ah, ATTR_NORMAL
        mov     cx, word [view_rec + R_QTY]
        cmp     cx, [cfg_lowstock]
        ja      .draw
        mov     ah, ATTR_BRIGHT
.draw:
        mov     si, row_buf
        call    draw_columns
        jmp     .next
.blank:
        call    clear_row
.next:
        inc     bx
        inc     dh
        cmp     dh, RES_TOP + RES_ROWS - 1
        jbe     .row
        cmp     word [match_count], 0
        jne     .status
        mov     si, str_no_match
        mov     dx, RES_TOP * 256 + 13
        mov     ah, ATTR_NORMAL
        call    put_str
.status:
        call    sb_reset                ; "Matches: n of m   Page p/q"
        mov     si, str_matches
        call    sb_str
        mov     ax, [match_count]
        call    sb_num
        mov     si, str_of
        call    sb_str
        mov     ax, [rec_count]
        call    sb_num
        mov     si, str_page
        call    sb_str
        mov     cx, RES_ROWS
        mov     ax, [page_top]
        xor     dx, dx
        div     cx
        inc     ax
        call    sb_num
        mov     al, '/'
        call    sb_char
        mov     ax, [match_count]
        add     ax, RES_ROWS - 1
        xor     dx, dx
        div     cx
        or      ax, ax
        jnz     .pages
        inc     ax
.pages:
        call    sb_num
        mov     si, str_low_level
        call    sb_str
        mov     ax, [cfg_lowstock]
        call    sb_num
        mov     dh, 21
        call    clear_row
        mov     dl, COL_LABEL
        mov     si, sb_buf
        mov     ah, ATTR_NORMAL
        call    put_str
        pop     di si dx cx bx ax
        ret

; build_row - Build the six column strings of view_rec in row_buf
build_row:
        push    ax cx si di
        mov     di, row_buf
        mov     si, view_rec + R_CODE
        call    .copy
        mov     si, view_rec + R_NAME
        call    .copy
        mov     ax, word [view_rec + R_QTY]
        call    u16_to_str
        call    .skip
        mov     si, view_rec + R_UNIT
        call    .copy
        mov     si, view_rec + R_PRICE
        call    .copy
        mov     si, str_empty
        mov     ax, word [view_rec + R_QTY]
        cmp     ax, [cfg_lowstock]
        ja      .note
        mov     si, str_low
.note:
        call    .copy
        pop     di si cx ax
        ret
.copy:
        call    str_copy
.skip:
        cmp     byte [di], 0            ; move DI past the copied string
        je      .end
        inc     di
        jmp     .skip
.end:
        inc     di
        ret

; --------------------------------------------------------------------------
; F2 - Add a new item
; --------------------------------------------------------------------------
sec_add:
        mov     si, hint_add
        call    draw_item_screen
        mov     si, str_add_intro
        mov     dx, 3 * 256 + COL_LABEL
        mov     ah, ATTR_NORMAL
        call    put_str
        mov     si, form_item
        call    clear_form
        call    draw_form
        xor     al, al
.edit:
        mov     si, form_item
        call    edit_form
        cmp     al, FORM_SWITCH
        je      .switch
        cmp     al, FORM_ESC
        je      .cancel
        xor     bx, bx                  ; new items start from zero
        call    form_to_rec
        jc      .invalid
        mov     si, edit_rec
        call    insert_rec
        jc      .insert_failed
        mov     si, msg_item
        mov     di, msg_added
        mov     bx, edit_rec + R_CODE
        call    item_message
        call    finish_change
        mov     si, form_item
        call    clear_form
        call    draw_form
        xor     al, al
        jmp     .edit
.insert_failed:
        mov     si, err_duplicate
        cmp     al, 1
        je      .dup
        mov     si, err_full
.dup:
        xor     al, al
.invalid:
        call    set_error
        mov     si, form_item
        call    draw_form
        jmp     .edit
.cancel:
        mov     si, form_item
        call    clear_form
        call    draw_form
        mov     si, msg_cleared
        call    set_msg
        xor     al, al
        jmp     .edit
.switch:
        mov     al, ah
        ret

; --------------------------------------------------------------------------
; F3 - Update an item
; --------------------------------------------------------------------------
sec_update:
        mov     si, hint_update
        call    draw_item_screen
.reset:
        mov     si, form_lookup
        call    clear_form
        call    draw_form
        mov     si, form_item
        call    clear_form
        call    show_form
.lookup:
        mov     si, form_lookup
        xor     al, al
        call    edit_form
        cmp     al, FORM_SWITCH
        je      .switch
        cmp     al, FORM_ESC
        je      .reset
        call    lookup_item
        pushf
        mov     si, form_lookup
        call    draw_form
        popf
        jc      .lookup
        mov     si, form_item
        call    draw_form
        mov     si, msg_editing
        call    set_msg
        xor     al, al
.edit:
        mov     si, form_item
        call    edit_form
        cmp     al, FORM_SWITCH
        je      .switch
        cmp     al, FORM_ESC
        je      .cancel
        mov     bx, word [view_rec + R_QTY]
        call    form_to_rec
        jc      .invalid
        mov     si, edit_rec + R_CODE
        mov     di, view_rec + R_CODE
        call    str_cmp
        je      .same_code
        call    find_code               ; the new code must be unused
        jnc     .duplicate
        mov     ax, [cur_index]
        call    delete_rec
        mov     si, edit_rec
        call    insert_rec
        jmp     .updated
.same_code:
        mov     ax, [cur_index]
        mov     si, edit_rec
        call    store_rec
        mov     byte [modified], 1
.updated:
        mov     si, msg_item
        mov     di, msg_updated
        mov     bx, edit_rec + R_CODE
        call    item_message
        call    finish_change
        jmp     .reset
.duplicate:
        mov     si, err_duplicate
        xor     al, al
.invalid:
        call    set_error
        mov     si, form_item
        call    draw_form
        jmp     .edit
.cancel:
        mov     si, msg_upd_cancel
        call    set_msg
        jmp     .reset
.switch:
        mov     al, ah
        ret

; --------------------------------------------------------------------------
; F4 - Delete an item
; --------------------------------------------------------------------------
sec_delete:
        mov     si, hint_delete
        call    draw_item_screen
.reset:
        mov     si, form_lookup
        call    clear_form
        call    draw_form
        mov     si, form_item
        call    clear_form
        call    show_form
.lookup:
        mov     si, form_lookup
        xor     al, al
        call    edit_form
        cmp     al, FORM_SWITCH
        je      .switch
        cmp     al, FORM_ESC
        je      .reset
        call    lookup_item
        pushf
        mov     si, form_lookup
        call    draw_form
        popf
        jc      .lookup
        mov     si, form_item
        call    show_form
        cmp     byte [cfg_confirm], 0
        je      .delete
        mov     si, ask_delete
        call    ask_yes_no
        cmp     al, 3
        je      .switch
        cmp     al, 1
        je      .delete
        mov     si, msg_del_cancel
        call    set_msg
        jmp     .reset
.delete:
        mov     ax, [cur_index]
        call    delete_rec
        mov     si, msg_item
        mov     di, msg_deleted
        mov     bx, view_rec + R_CODE
        call    item_message
        call    finish_change
        jmp     .reset
.switch:
        mov     al, ah
        ret

; --------------------------------------------------------------------------
; F5 - Configuration
; --------------------------------------------------------------------------
sec_config:
        mov     si, hint_config
        call    draw_item_screen
        mov     si, str_cfg_intro
        mov     dx, 3 * 256 + COL_LABEL
        mov     ah, ATTR_NORMAL
        call    put_str
.reset:
        call    config_to_form
        mov     si, form_config
        call    draw_form
        xor     al, al
.edit:
        mov     si, form_config
        call    edit_form
        cmp     al, FORM_SWITCH
        je      .switch
        cmp     al, FORM_ESC
        je      .cancel
        mov     si, c_lowstock
        call    str_to_u16
        jc      .bad_level
        mov     [cfg_lowstock], ax
        mov     si, c_company
        call    str_trim
        mov     di, cfg_company
        call    str_copy
        mov     al, [c_confirm]
        call    yes_no_flag
        mov     [cfg_confirm], al
        mov     al, [c_autosave]
        call    yes_no_flag
        mov     [cfg_autosave], al
        call    save_config
        jc      .save_failed
        mov     si, msg_cfg_saved
        call    set_msg
        cmp     byte [cfg_autosave], 0  ; autosave just enabled: save now
        je      .reset
        cmp     byte [modified], 0
        je      .reset
        call    save_db
        jnc     .reset
        mov     si, err_save
        call    set_error
        jmp     .reset
.save_failed:
        mov     si, err_cfg_save
        call    set_error
        jmp     .reset
.bad_level:
        mov     si, err_lowstock
        call    set_error
        mov     al, 1
        jmp     .edit
.cancel:
        mov     si, msg_cfg_discard
        call    set_msg
        jmp     .reset
.switch:
        mov     al, ah
        ret

; yes_no_flag - Convert 'Y' to 1, anything else to 0 (AL in/out)
yes_no_flag:
        cmp     al, 'Y'
        mov     al, 0
        jne     .done
        inc     al
.done:
        ret

; config_to_form - Copy the configuration into the form buffers
config_to_form:
        push    ax si di
        mov     si, cfg_company
        mov     di, c_company
        call    str_copy
        mov     ax, [cfg_lowstock]
        mov     di, c_lowstock
        call    u16_to_str
        mov     al, [cfg_confirm]
        mov     di, c_confirm
        call    flag_to_yes_no
        mov     al, [cfg_autosave]
        mov     di, c_autosave
        call    flag_to_yes_no
        pop     di si ax
        ret

; flag_to_yes_no - Store "Y" or "N" for the flag in AL at DI
flag_to_yes_no:
        mov     byte [di], 'N'
        or      al, al
        jz      .done
        mov     byte [di], 'Y'
.done:
        mov     byte [di + 1], 0
        ret

; --------------------------------------------------------------------------
; F6 - Help / About
; --------------------------------------------------------------------------
sec_help:
        call    draw_screen
        mov     si, help_text
        mov     dx, 2 * 256 + COL_LABEL
        mov     ah, ATTR_NORMAL
        call    draw_text_block
.key:
        call    get_key
        call    check_section_key
        jnc     .key
        ret

; --------------------------------------------------------------------------
; cmd_save - F9: write the database to DATA.DAT
; --------------------------------------------------------------------------
cmd_save:
        push    ax si
        call    save_db
        jc      .failed
        call    sb_reset
        mov     si, msg_saved
        call    sb_str
        mov     ax, [rec_count]
        call    sb_num
        mov     si, msg_saved2
        call    sb_str
        mov     si, sb_buf
        call    set_msg
        jmp     .done
.failed:
        mov     si, err_save
        call    set_error
.done:
        pop     si ax
        ret

; --------------------------------------------------------------------------
; confirm_exit - F10: offer to save unsaved changes
; Out: CF = 1 if the user cancelled the exit
; --------------------------------------------------------------------------
confirm_exit:
        cmp     byte [modified], 0
        je      .leave
        mov     si, ask_save_exit
        call    ask_yes_no
        cmp     al, 0
        je      .leave                  ; discard the changes
        cmp     al, 1
        jne     .cancel
        call    save_db
        jnc     .leave
        mov     si, err_save
        call    set_error
.cancel:
        stc
        ret
.leave:
        clc
        ret
