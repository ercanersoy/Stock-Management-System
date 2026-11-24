; ==========================================================================
; STOCK.ASM - Stock Management System
;
; Target  : IBM PC/XT and compatibles, Intel 8088, DOS 2.0+
;           MDA, Hercules, CGA, EGA, PGC, MCGA or VGA display adapter
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

        mov     ah, 30h                 ; DOS version (DOS 1.x returns 0)
        int     21h
        cmp     al, 2
        jae     .dos_ok
        mov     dx, msg_need_dos
        mov     ah, 09h
        int     21h
        int     20h
.dos_ok:
        mov     [dos_major], al
        mov     ax, 3300h               ; remember the BREAK setting and
        int     21h                     ; turn it off: Ctrl+C/Ctrl+Break
        mov     [break_state], dl       ; must not abort a file write
        mov     ax, 3301h
        xor     dl, dl
        int     21h
        mov     ax, 2523h               ; ignore Ctrl+C / Ctrl+Break
        mov     dx, break_handler       ; (DOS restores both vectors
        int     21h                     ; when the program ends)
        cmp     byte [dos_major], 3     ; DOS 3+: fail critical errors
        jb      .memory                 ; instead of "Abort, Retry..."
        mov     ax, 2524h
        mov     dx, critical_handler
        int     21h
.memory:
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

        call    init_video              ; detect the adapter, set the mode
        call    hide_cursor
        call    load_config
        call    load_db
        jnc     .loaded
        mov     dx, msg_read_error
        jmp     fatal_video
.loaded:
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
        call    restore_video           ; original mode, clears the screen
        call    restore_break
        mov     dx, msg_bye
        mov     ah, 09h
        int     21h
        mov     ax, 4C00h
        int     21h

; --------------------------------------------------------------------------
; fatal_video - Restore the video mode, then print a message and terminate
; fatal       - Print a DOS message and terminate
; In:  DX = '$' terminated message
; --------------------------------------------------------------------------
fatal_video:
        call    restore_video
fatal:
        call    restore_break
        mov     ah, 09h
        int     21h
        mov     ax, 4C01h
        int     21h

; restore_break - Restore the BREAK setting found at start-up
restore_break:
        push    ax dx
        mov     ax, 3301h
        mov     dl, [break_state]
        int     21h
        pop     dx ax
        ret

; INT 23h handler: ignore Ctrl+C and Ctrl+Break
break_handler:
        iret

; INT 24h handler (DOS 3+): fail the call, the caller reports the error
critical_handler:
        mov     al, 3
        iret

include 'VIDEO.INC'
include 'STRING.INC'
include 'FILEIO.INC'
include 'DB.INC'
include 'CONFIG.INC'
include 'UI.INC'
include 'SECTIONS.INC'
include 'DATA.INC'
