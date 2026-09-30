; ==========================================================================
; STOCK.ASM - Stock Management System
;
; Target  : IBM PC/XT and compatibles, Intel 8088, MDA display, DOS 2.0+
; Build   : fasm STOCK.ASM STOCK.COM
; Files   : DATA.DAT  - item database (text)
;           STOCK.CFG - configuration (text)
;
; Copyright (c) 2026 Ercan Ersoy. Released under the MIT License.
; ==========================================================================

        format  binary as 'COM'
        use16
        org     100h

include 'CONST.INC'

; --------------------------------------------------------------------------
; Program entry point
; --------------------------------------------------------------------------
start:
        cld
        mov     di, bss_start           ; zero the uninitialised data
        mov     cx, bss_end - bss_start
        xor     al, al
        rep     stosb

        mov     ah, 0Fh                 ; the MDA only supports mode 7
        int     10h
        cmp     al, 7
        je      .video_ok
        mov     dx, msg_need_mda
        jmp     fatal
.video_ok:
        mov     ah, 4Ah                 ; keep 64 KB for the program ...
        mov     bx, 1000h
        int     21h
        jc      .no_memory
        mov     ah, 48h                 ; ... and allocate the record block
        mov     bx, (MAX_RECS * REC_SIZE + 15) / 16
        int     21h
        jnc     .memory_ok
.no_memory:
        mov     dx, msg_no_memory
        jmp     fatal
.memory_ok:
        mov     [rec_seg], ax

        mov     ax, 0007h               ; reset the screen
        int     10h
        call    hide_cursor
        call    load_config
        call    load_db
        mov     byte [cur_section], SEC_SEARCH

main_loop:
        call    draw_screen
        mov     bl, [cur_section]
        xor     bh, bh
        shl     bx, 1
        call    word [section_table + bx]
        mov     byte [msg_buf], 0       ; messages do not survive a switch
        cmp     al, SEC_EXIT
        je      .exit
        mov     [cur_section], al
        jmp     main_loop
.exit:
        call    confirm_exit
        jc      main_loop
        mov     ax, 0007h               ; clear the screen, restore cursor
        int     10h
        mov     dx, msg_bye
        mov     ah, 09h
        int     21h
        mov     ax, 4C00h
        int     21h

; --------------------------------------------------------------------------
; fatal - Print a DOS message and terminate
; In:  DX = '$' terminated message
; --------------------------------------------------------------------------
fatal:
        mov     ah, 09h
        int     21h
        mov     ax, 4C01h
        int     21h

include 'VIDEO.INC'
include 'STRING.INC'
include 'FILEIO.INC'
include 'DB.INC'
include 'CONFIG.INC'
include 'UI.INC'
include 'SECTIONS.INC'
include 'DATA.INC'
