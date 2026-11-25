# Stock Management System

A text-mode stock (inventory) management program for **IBM PC/XT class
machines (Intel 8088)**, written entirely in 8088 assembly language for the
[flat assembler (FASM)](https://flatassembler.net). It runs on **MDA,
Hercules, CGA, EGA, PGC, MCGA and VGA** display adapters, always with the
same green on black look.

## Features

- Runs on an 8088 with DOS 2.0 or later and about 128 KB of RAM; uses only
  8086/8088 instructions.
- Writes directly to the text buffer (see [Display adapters](#display-adapters)).
- Up to 1000 items, kept sorted by item code.
- Sections are selected with the function keys F1-F10 only - there is no
  arrow key menu. F11 and F12 are not used; F7 and F8 open sub-menus whose
  options are chosen with the number keys:

| Key | Section | Description |
|-----|---------|-------------|
| F1  | Search  | Filters items by code or name while you type (case insensitive). PgUp/PgDn pages through the results, Esc clears the search. |
| F2  | Add     | Adds a new item. |
| F3  | Update  | Loads an item by code and edits it. A quantity of `+n` or `-n` adds or removes stock; changing the code re-sorts the item. |
| F4  | Delete  | Finds an item by code, shows it and deletes it (optionally after a Y/N confirmation). |
| F5  | Config  | Edits the settings stored in `STOCK.CFG`. |
| F6  | Help    | Key summary and About information. |
| F7  | Reports | Sub-menu: **1** low stock items, **2** out of stock items, **3** stock value (total quantity and quantity x price). Lists page with PgUp/PgDn, Esc returns to the menu. |
| F8  | Tools   | Sub-menu: **1** reload `DATA.DAT` from disk, **2** back up the database to `DATA.BAK`, **3** display adapter information. |
| F9  | Save    | Writes `DATA.DAT` immediately. |
| F10 | Exit    | Quits (offers to save unsaved changes). |

The key bar at the bottom shows the keys in the usual DOS style
(`1Search 2Add ... 10Exit`, the number is the function key).

In forms: **Enter/Tab** moves to the next field, **Shift+Tab** to the previous
one, **Backspace** erases, **Esc** cancels. Pressing **Enter on the last
field** saves the form. In the price field a comma may be typed instead of
the decimal point.

In the sub-menus of F7 and F8 the number keys choose an option and **Esc**
inside an option returns to the menu. F1-F8 switch to another section, F9
saves and F10 exits from anywhere, also from a sub-menu.

Y/N questions on the message line (delete, reload, save before exit) must be
answered with **Y**, **N** or **Esc**; F9 does not save while a question is
open. A section key (F1-F8) pressed at the "save before exit?" question
cancels the exit and opens that section.

Items whose quantity is at or below the configured low stock level are shown
in bright text and marked `LOW` in the search list and the reports.

## Files

### `DATA.DAT` - database (plain text)

One item per line, fields separated by `;`:

```
# STOCK.COM database - one item per line:
# CODE;NAME;QUANTITY;UNIT;PRICE
```

| Field    | Rules |
|----------|-------|
| CODE     | Required, unique, up to 10 characters: `A-Z 0-9 - _ . /` (stored upper case). A longer code makes the line invalid. |
| NAME     | Required, up to 30 characters (longer names are shortened) |
| QUANTITY | 0-65535 (leading zeros are allowed) |
| UNIT     | Up to 6 characters, e.g. `PCS`, `BOX`, `M` (longer units are shortened) |
| PRICE    | Up to 7 digits and 2 decimals, `.` or `,` as the decimal separator; stored as `0.00` |

Spaces around the fields are removed. Lines starting with `#` and empty lines
are ignored. Names and units may use the national characters of the DOS code
page (e.g. Turkish letters in CP857). If the file does not exist, the program
starts with an empty database.

Invalid and duplicate lines are skipped when loading and counted on the
message line. They are not lost: the first save after such a load keeps the
old `DATA.DAT` as `DATA.BAK`, so the skipped lines can be corrected there and
copied back.

### `DATA.BAK` - backup

Written by F8, option 2 (a copy of the database in memory), or by the first
save after lines were skipped while loading (the old `DATA.DAT`, see above).
An older `DATA.BAK` is replaced.

### `STOCK.CFG` - configuration (plain text)

```
COMPANY=
LOWSTOCK=10
CONFIRM=YES
AUTOSAVE=YES
```

| Key      | Meaning |
|----------|---------|
| COMPANY  | Company name shown in the title bar (max 30 characters, empty by default) |
| LOWSTOCK | Items with a quantity at or below this level are flagged `LOW` |
| CONFIRM  | `YES`/`NO` - ask before deleting an item |
| AUTOSAVE | `YES`/`NO` - write `DATA.DAT` after every change; with `NO` use F9 or save when exiting |

Lines starting with `;` or `#` are comments. The file is created with default
values if it is missing; a file that exists but cannot be read is left
unchanged and the defaults are used.

### Safe saving

`DATA.DAT`, `DATA.BAK` and `STOCK.CFG` are first written to `STOCK.$$$`, which
replaces the real file only after it has been written completely. A full disk
or a write error therefore never destroys the previous version. Ctrl+C and
Ctrl+Break are ignored while the program runs, and on DOS 3.1 and later disk
errors (also those DOS reports when it closes a file) are reported on the
message line instead of the "Abort, Retry, Fail?" prompt. If `DATA.DAT` exists but cannot be read, the
program stops without changing anything instead of starting with a
partial database.

## Display adapters

The adapter is detected at start-up (VGA/MCGA and EGA BIOS calls, the
current video mode, the Hercules vertical sync bit and the PGC communication
buffer). With two adapters installed the one in use is taken. F8, option 3 shows the result.

| Adapter | Text mode | Look |
|---------|-----------|------|
| MDA, Hercules | 7 (`B000:0000`) | monochrome attributes: normal, bright, underline, reverse |
| EGA / VGA with a monochrome monitor | 7 (`B000:0000`) | as above |
| CGA, PGC (CGA emulation), EGA, MCGA, VGA with a colour monitor | 3 (`B800:0000`) | green on black: green, light green, black on green; input fields are padded with `_` since colour adapters cannot underline |

A colour EGA/VGA always gets mode 3, even if mode 7 was active when the
program was started, so the screen stays green on black.

On a CGA every write to the screen waits for the horizontal retrace, so the
original IBM CGA shows no "snow". The video mode that was active at start-up
is restored on exit (an EGA/VGA that was in 43 or 50 line mode returns
with 25 lines).

## Building

With FASM on DOS:

```
BUILD.BAT
```

With FASM on Linux (or any system with `make`):

```
make
```

Both produce `bin/STOCK.COM` (the `bin` directory is not stored in the
repository).

## Running

Copy `STOCK.COM` to a directory and run `STOCK` from there. The data files
are read from and written to the current directory; both are created when
needed. Saving needs room for one extra copy of the file being written
(`STOCK.$$$`).

In DOSBox the adapter is chosen with `machine=` (`hercules`, `cga`, `ega`,
`vgaonly`); DOSBox-X also offers `mda` and `mcga`.

## Source layout

| File | Contents |
|------|----------|
| `src/STOCK.ASM`    | Entry point, main loop |
| `src/CONST.INC`    | Constants, record layout, macros |
| `src/VIDEO.INC`    | Adapter detection, video mode, direct text output (CGA snow safe), cursor, speaker beep |
| `src/STRING.INC`   | String and number routines (incl. 64 bit totals), string builder |
| `src/FILEIO.INC`   | Buffered DOS file I/O |
| `src/DB.INC`       | In-memory database, `DATA.DAT` load/save |
| `src/CONFIG.INC`   | `STOCK.CFG` load/save |
| `src/UI.INC`       | Screen frame, message line, keyboard, sub-menus, form engine |
| `src/SECTIONS.INC` | The Search, Add, Update, Delete, Config, Help, Reports and Tools sections |
| `src/DATA.INC`     | Strings, forms and variables |

## License

MIT - see [LICENSE](LICENSE).
