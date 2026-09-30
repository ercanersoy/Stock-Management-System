; ==========================================================================
; CONST.INC - Constants, record layout and helper macros
; ==========================================================================

; --- Program limits -------------------------------------------------------
MAX_RECS        = 1000                  ; maximum number of stock items
REC_SIZE        = 64                    ; bytes per record (power of two)
REC_SHIFT       = 6                     ; log2(REC_SIZE)
LINE_MAX        = 127                   ; longest text line read from files
IO_SIZE         = 512                   ; file I/O buffer size

; --- Record layout (all strings are zero terminated) ----------------------
L_CODE          = 10                    ; item code length
L_NAME          = 30                    ; item name length
L_UNIT          = 6                     ; unit of measure length
L_PRICE         = 10                    ; unit price length ("9999999.99")
L_QTY           = 6                     ; quantity input length ("+65535")

R_CODE          = 0                     ; offset of code  (L_CODE+1 bytes)
R_NAME          = 11                    ; offset of name  (L_NAME+1 bytes)
R_UNIT          = 42                    ; offset of unit  (L_UNIT+1 bytes)
R_PRICE         = 49                    ; offset of price (L_PRICE+1 bytes)
R_QTY           = 60                    ; offset of quantity (word)

; --- MDA video ------------------------------------------------------------
VIDEO_SEG       = 0B000h                ; MDA text buffer segment
SCR_COLS        = 80
SCR_ROWS        = 25

ATTR_NORMAL     = 07h                   ; normal text
ATTR_BRIGHT     = 0Fh                   ; high intensity
ATTR_UNDERLINE  = 01h                   ; underlined text
ATTR_REVERSE    = 70h                   ; reverse video
ATTR_BLINK      = 8Fh                   ; blinking high intensity

; --- Screen layout --------------------------------------------------------
ROW_TITLE       = 0                     ; title bar
ROW_BOX_TOP     = 1                     ; top border of the work window
ROW_BOX_BOTTOM  = 22                    ; bottom border of the work window
ROW_MSG         = 23                    ; message line
ROW_KEYS        = 24                    ; function key bar
COL_LABEL       = 3                     ; column of form labels
COL_INPUT       = 22                    ; column of form input fields

RES_TOP         = 7                     ; first row of search results
RES_ROWS        = 13                    ; number of result rows per page

; --- Keyboard (value returned by INT 16h, AH=00h) -------------------------
KEY_ENTER       = 0Dh
KEY_ESC         = 1Bh
KEY_TAB         = 09h
KEY_BACKSPACE   = 08h
KEY_SHIFT_TAB   = 0F00h
KEY_F1          = 3B00h
KEY_F6          = 4000h
KEY_F9          = 4300h
KEY_F10         = 4400h
KEY_PGUP        = 4900h
KEY_PGDN        = 5100h

; --- Sections -------------------------------------------------------------
SEC_SEARCH      = 0
SEC_ADD         = 1
SEC_UPDATE      = 2
SEC_DELETE      = 3
SEC_CONFIG      = 4
SEC_HELP        = 5
SEC_EXIT        = 0FFh

; --- Form engine ----------------------------------------------------------
F_ROW           = 0                     ; byte: screen row of the field
F_COL           = 1                     ; byte: screen column of the input
F_LABEL         = 2                     ; word: label string
F_BUF           = 4                     ; word: edit buffer
F_MAX           = 6                     ; byte: maximum text length
F_TYPE          = 7                     ; byte: field type (FT_xxx)
FIELD_SIZE      = 8
FIELD_SHIFT     = 3

FT_TEXT         = 0                     ; any printable character
FT_CODE         = 1                     ; A-Z 0-9 - _ . / (forced upper case)
FT_NUM          = 2                     ; digits only
FT_QTY          = 3                     ; digits, optional leading + or -
FT_PRICE        = 4                     ; digits and a single decimal point
FT_YESNO        = 5                     ; single Y or N

FORM_SUBMIT     = 0                     ; Enter pressed on the last field
FORM_ESC        = 1                     ; Esc pressed
FORM_SWITCH     = 2                     ; section key pressed (AH = section)

; Defines one form field descriptor.
macro FIELD row, col, lbl, buf, max, type
{
        db      row, col
        dw      lbl, buf
        db      max, type
}
