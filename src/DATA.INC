; ==========================================================================
; DATA.INC - Initialised data: strings, tables, forms and settings
; ==========================================================================

; --- File names -----------------------------------------------------------
data_file       db      'DATA.DAT', 0
cfg_file        db      'STOCK.CFG', 0

; --- Configuration (defaults, overwritten by STOCK.CFG) -------------------
cfg_company     db      L_COMPANY + 1 dup 0     ; empty by default
cfg_lowstock    dw      10
cfg_confirm     db      1
cfg_autosave    db      1

key_company     db      'COMPANY', 0
key_lowstock    db      'LOWSTOCK', 0
key_confirm     db      'CONFIRM', 0
key_autosave    db      'AUTOSAVE', 0
str_yes         db      'YES', 0
str_no          db      'NO', 0

cfg_header      db      '; STOCK.CFG - Stock Management System configuration', 13, 10
                db      ';', 13, 10
                db      '; COMPANY  = company name shown in the title bar (max 30 chars)', 13, 10
                db      '; LOWSTOCK = items with a quantity at or below this level are', 13, 10
                db      ';            flagged LOW (0-65535)', 13, 10
                db      '; CONFIRM  = ask before deleting an item (YES/NO)', 13, 10
                db      '; AUTOSAVE = write DATA.DAT after every change (YES/NO)', 13, 10
                db      ';            when NO, use F9 to save or answer the prompt on exit', 13, 10
                db      0

db_header       db      '# STOCK.COM database - one item per line:', 13, 10
                db      '# CODE;NAME;QUANTITY;UNIT;PRICE', 13, 10
                db      0

; --- DOS messages ($ terminated) ------------------------------------------
msg_need_mda    db      'STOCK.COM requires an MDA (monochrome) display adapter', 13, 10
                db      'in 80x25 monochrome text mode (mode 7).', 13, 10, '$'
msg_no_memory   db      'Not enough memory. STOCK.COM needs about 128 KB.', 13, 10, '$'
msg_bye         db      'Stock Management System - goodbye.', 13, 10, '$'

; --- Screen texts ---------------------------------------------------------
str_title       db      'STOCK MANAGEMENT SYSTEM', 0
str_modified    db      'Modified  ', 0
str_items       db      'Items: ', 0
str_empty       db      0
str_period      db      '.', 0
str_low         db      'LOW', 0

section_names   dw      name_search, name_add, name_update
                dw      name_delete, name_config, name_help
name_search     db      ' Search Items ', 0
name_add        db      ' Add Item ', 0
name_update     db      ' Update Item ', 0
name_delete     db      ' Delete Item ', 0
name_config     db      ' Configuration ', 0
name_help       db      ' Help / About ', 0

section_table   dw      sec_search, sec_add, sec_update
                dw      sec_delete, sec_config, sec_help

KEY_COUNT       = 8
KEY_LABEL_WIDTH = 7
key_table       dw      key_f1, lbl_f1, key_f2, lbl_f2, key_f3, lbl_f3
                dw      key_f4, lbl_f4, key_f5, lbl_f5, key_f6, lbl_f6
                dw      key_f9, lbl_f9, key_f10, lbl_f10
key_f1          db      'F1', 0
key_f2          db      'F2', 0
key_f3          db      'F3', 0
key_f4          db      'F4', 0
key_f5          db      'F5', 0
key_f6          db      'F6', 0
key_f9          db      'F9', 0
key_f10         db      'F10', 0
lbl_f1          db      ' Search', 0
lbl_f2          db      ' Add', 0
lbl_f3          db      ' Update', 0
lbl_f4          db      ' Delete', 0
lbl_f5          db      ' Config', 0
lbl_f6          db      ' Help', 0
lbl_f9          db      ' Save', 0
lbl_f10         db      ' Exit', 0

; --- Search section -------------------------------------------------------
str_search_hint db      'PgUp/PgDn: page   Esc: clear', 0
str_header      db      'Code', 0, 'Name', 0, 'Qty', 0, 'Unit', 0
                db      'Price', 0, 'Note', 0
str_no_match    db      'No matching items.', 0
str_matches     db      'Matches: ', 0
str_of          db      ' of ', 0
str_page        db      '    Page ', 0
str_low_level   db      '    Low stock level: ', 0

; --- Add / Update / Delete sections ---------------------------------------
str_add_intro   db      'Fill in the details of the new item.', 0

hint_add        db      'Enter/Tab: next field    Shift+Tab: previous field    '
                db      'Backspace: erase', 0
                db      'Enter on the last field (Unit price) saves the item.   '
                db      'Esc: clear the form', 0
                db      'Code: letters, digits and - _ . /    Quantity: 0-65535', 0
                db      'Price: up to 7 digits and 2 decimals, e.g. 1250.50', 0
                db      0

hint_update     db      'Type the code of the item and press Enter to load it.', 0
                db      'Enter/Tab: next field    Shift+Tab: previous field    '
                db      'Esc: cancel', 0
                db      'Enter on the last field (Unit price) saves the changes.', 0
                db      'Quantity: type a new value, or +n / -n to add or remove '
                db      'stock.', 0
                db      0

hint_delete     db      'Type the code of the item and press Enter to find it.', 0
                db      'The item is shown above before it is deleted.', 0
                db      'Esc: clear the code', 0
                db      0

msg_item        db      'Item ', 0
msg_added       db      ' added.', 0
msg_updated     db      ' updated.', 0
msg_deleted     db      ' deleted.', 0
msg_cleared     db      'Form cleared.', 0
msg_editing     db      'Edit the item and press Enter on the last field to save.', 0
msg_upd_cancel  db      'Update cancelled.', 0
msg_del_cancel  db      'Delete cancelled.', 0
ask_delete      db      'Delete this item? (Y/N)', 0

lbl_find        db      'Item code:', 0
lbl_code        db      'Item code:', 0
lbl_name        db      'Item name:', 0
lbl_qty         db      'Quantity:', 0
lbl_unit        db      'Unit:', 0
lbl_price       db      'Unit price:', 0
lbl_search      db      'Search:', 0

; --- Configuration section ------------------------------------------------
str_cfg_intro   db      'Settings are stored in STOCK.CFG.', 0
hint_config     db      'Enter/Tab: next field    Shift+Tab: previous field    '
                db      'Esc: discard', 0
                db      'Enter on the last field saves the configuration.', 0
                db      'Items with a quantity at or below the low stock level are', 0
                db      'shown highlighted and marked LOW in the search list.', 0
                db      'Yes/No fields: press Y or N.', 0
                db      0
lbl_company     db      'Company name:', 0
lbl_lowstock    db      'Low stock level:', 0
lbl_confirm     db      'Confirm deletes (Y/N):', 0
lbl_autosave    db      'Autosave (Y/N):', 0
msg_cfg_saved   db      'Configuration saved to STOCK.CFG.', 0
msg_cfg_discard db      'Changes discarded.', 0

; --- Load / save messages -------------------------------------------------
msg_loaded      db      'Loaded ', 0
msg_loaded2     db      ' items from DATA.DAT.', 0
msg_skipped     db      ' Skipped ', 0
msg_skipped2    db      ' invalid or duplicate lines!', 0
msg_new_db      db      'DATA.DAT not found - starting with an empty database.', 0
msg_saved       db      'Database saved to DATA.DAT (', 0
msg_saved2      db      ' items).', 0
ask_save_exit   db      'Save changes to DATA.DAT before exit? (Y/N, Esc: cancel)', 0

; --- Error messages -------------------------------------------------------
err_code_required db    'Item code is required.', 0
err_name_required db    'Item name is required.', 0
err_quantity    db      'Invalid quantity. Use 0-65535, or +n / -n to adjust.', 0
err_price       db      'Invalid price. Use up to 7 digits and 2 decimals.', 0
err_duplicate   db      'An item with this code already exists.', 0
err_full        db      'The database is full.', 0
err_not_found   db      'Item not found: ', 0
err_save        db      'Error: DATA.DAT could not be written!', 0
err_autosave    db      'Change kept in memory, but DATA.DAT could not be written!', 0
err_cfg_save    db      'Error: STOCK.CFG could not be written!', 0
err_lowstock    db      'Invalid low stock level. Use 0-65535.', 0

; --- Help / About ---------------------------------------------------------
; Lines starting with byte 1 are drawn highlighted.
help_text       db      1, 'STOCK MANAGEMENT SYSTEM  Version 1.0', 0
                db      'For IBM PC/XT compatibles (8088) with an MDA display.', 0
                db      'Written in 8088 assembly language with the flat assembler.', 0
                db      'Copyright (c) 2026 Ercan Ersoy. Released under the MIT License.', 0
                db      ' ', 0
                db      1, 'KEYS', 0
                db      'F1 Search  Find items by code or name as you type. PgUp/PgDn.', 0
                db      'F2 Add     Add a new item.', 0
                db      'F3 Update  Edit an item. Quantity +n / -n adds or removes stock.', 0
                db      'F4 Delete  Delete an item.', 0
                db      'F5 Config  Company name, low stock level, confirm and autosave.', 0
                db      'F6 Help    This screen.     F9 Save: save now.     F10 Exit', 0
                db      'In forms: Enter/Tab next field, Shift+Tab previous, Esc cancel.', 0
                db      ' ', 0
                db      1, 'FILES', 0
                db      'DATA.DAT   Items as text, one per line: CODE;NAME;QUANTITY;UNIT;PRICE', 0
                db      'STOCK.CFG  Settings as text, one KEY=VALUE per line.', 0
                db      ' ', 0
                db      'Up to 1000 items. Items at or below the low stock level are marked LOW.', 0
                db      0

; --- Forms ----------------------------------------------------------------
form_search     db      1
                FIELD   3, 12, lbl_search, f_query, L_NAME, FT_TEXT

form_lookup     db      1
                FIELD   3, COL_INPUT, lbl_find, f_find, L_CODE, FT_CODE

form_item       db      5
                FIELD   6, COL_INPUT, lbl_code, f_code, L_CODE, FT_CODE
                FIELD   8, COL_INPUT, lbl_name, f_name, L_NAME, FT_TEXT
                FIELD   10, COL_INPUT, lbl_qty, f_qty, L_QTY, FT_QTY
                FIELD   12, COL_INPUT, lbl_unit, f_unit, L_UNIT, FT_TEXT
                FIELD   14, COL_INPUT, lbl_price, f_price, L_PRICE, FT_PRICE

form_config     db      4
                FIELD   6, 28, lbl_company, c_company, L_COMPANY, FT_TEXT
                FIELD   8, 28, lbl_lowstock, c_lowstock, 5, FT_NUM
                FIELD   10, 28, lbl_confirm, c_confirm, 1, FT_YESNO
                FIELD   12, 28, lbl_autosave, c_autosave, 1, FT_YESNO

; ==========================================================================
; Uninitialised data (zeroed at start-up, not stored in STOCK.COM)
; ==========================================================================
bss_start:
rec_seg         rw      1               ; segment of the record block
rec_count       rw      1               ; number of records
skipped         rw      1               ; lines skipped while loading
modified        rb      1               ; unsaved changes
cur_section     rb      1               ; active section
cur_index       rw      1               ; record being updated/deleted
page_top        rw      1               ; first search result shown
match_count     rw      1               ; number of search results
ef_index        rb      1               ; active field of edit_form
msg_attr        rb      1               ; attribute of the message line
msg_buf         rb      SCR_COLS        ; message line text

f_handle        rw      1               ; file I/O state
io_pos          rw      1
io_len          rw      1
f_eof           rb      1
f_error         rb      1
io_buf          rb      IO_SIZE

sb_ptr          rw      1               ; string builder
sb_buf          rb      SCR_COLS + 2
num_buf         rb      8
tmp_buf         rb      LINE_MAX + 1
tmp_buf2        rb      LINE_MAX + 1
line_buf        rb      LINE_MAX + 1
needle          rb      L_NAME + 1
row_buf         rb      SCR_COLS

key_rec         rb      REC_SIZE        ; record buffers
io_rec          rb      REC_SIZE
view_rec        rb      REC_SIZE
edit_rec        rb      REC_SIZE

f_query         rb      L_NAME + 1      ; form edit buffers
f_find          rb      L_CODE + 1
f_code          rb      L_CODE + 1
f_name          rb      L_NAME + 1
f_qty           rb      L_QTY + 1
f_unit          rb      L_UNIT + 1
f_price         rb      L_PRICE + 1
c_company       rb      L_COMPANY + 1
c_lowstock      rb      6
c_confirm       rb      2
c_autosave      rb      2

match_idx       rw      MAX_RECS        ; search result record indexes
bss_end:
