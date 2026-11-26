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

; --- Video ----------------------------------------------------------------
SEG_MONO        = 0B000h                ; MDA / Hercules / EGA-VGA mode 7
SEG_COLOR       = 0B800h                ; CGA / EGA / PGC / MCGA / VGA mode 3
MODE_MONO       = 07h                   ; 80x25 monochrome text
MODE_COLOR      = 03h                   ; 80x25 16 colour text
CGA_STATUS      = 3DAh                  ; CGA status port (bit 0: retrace)
HGC_STATUS      = 3BAh                  ; MDA/Hercules status port
SCR_COLS        = 80
SCR_ROWS        = 25

; Monochrome attributes (MDA, Hercules, EGA/VGA with a mono monitor)
MONO_NORMAL     = 07h                   ; normal text
MONO_BRIGHT     = 0Fh                   ; high intensity
MONO_FIELD      = 01h                   ; underlined text (input fields)
MONO_REVERSE    = 70h                   ; reverse video

; Colour adapters show the same green on black look (no other theme)
GREEN_NORMAL    = 02h                   ; green on black
GREEN_BRIGHT    = 0Ah                   ; light green on black
GREEN_FIELD     = 02h                   ; green, padded with '_' (no underline)
GREEN_REVERSE   = 20h                   ; black on green

; Display adapters (value of [adapter])
ADP_MDA         = 0
ADP_HERCULES    = 1
ADP_CGA         = 2
ADP_EGA         = 3
ADP_PGC         = 4
ADP_MCGA        = 5
ADP_VGA         = 6

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
KEY_F8          = 4200h
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
SEC_REPORTS     = 6
SEC_TOOLS       = 7
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

; --- Item list filters (collect_matches) ----------------------------------
FILTER_TEXT     = 0                     ; code or name contains the text
FILTER_LOW      = 1                     ; quantity <= low stock level
FILTER_ZERO     = 2                     ; quantity = 0
