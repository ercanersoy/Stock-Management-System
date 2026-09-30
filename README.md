# Stock Management System

A text-mode stock (inventory) management program for **IBM PC/XT class
machines (Intel 8088)** with an **MDA (monochrome) display adapter**,
written entirely in 8088 assembly language for the
[flat assembler (FASM)](https://flatassembler.net).

## Features

- Runs on an 8088 with DOS 2.0 or later and about 128 KB of RAM; uses only
  8086/8088 instructions.
- Writes directly to the MDA text buffer (`B000:0000`, 80x25, mode 7) using
  the monochrome attributes: normal, bright, underline and reverse video.
- Up to 1000 items, kept sorted by item code.
- Sections are selected with function keys only - there is no arrow key menu:

| Key | Section | Description |
|-----|---------|-------------|
| F1  | Search  | Filters items by code or name while you type (case insensitive). PgUp/PgDn pages through the results, Esc clears the search. |
| F2  | Add     | Adds a new item. |
| F3  | Update  | Loads an item by code and edits it. A quantity of `+n` or `-n` adds or removes stock; changing the code re-sorts the item. |
| F4  | Delete  | Finds an item by code, shows it and deletes it (optionally after a Y/N confirmation). |
| F5  | Config  | Edits the settings stored in `STOCK.CFG`. |
| F6  | Help    | Key summary and About information. |
| F9  | Save    | Writes `DATA.DAT` immediately. |
| F10 | Exit    | Quits (offers to save unsaved changes). |

In forms: **Enter/Tab** moves to the next field, **Shift+Tab** to the previous
one, **Backspace** erases, **Esc** cancels. Pressing **Enter on the last
field** saves the form.

Items whose quantity is at or below the configured low stock level are shown
in bright text and marked `LOW` in the search list.

## Files

### `DATA.DAT` - database (plain text)

One item per line, fields separated by `;`:

```
# STOCK.COM database - one item per line:
# CODE;NAME;QUANTITY;UNIT;PRICE
```

| Field    | Rules |
|----------|-------|
| CODE     | Required, unique, up to 10 characters: `A-Z 0-9 - _ . /` (stored upper case) |
| NAME     | Required, up to 30 characters |
| QUANTITY | 0-65535 |
| UNIT     | Up to 6 characters (e.g. `PCS`, `BOX`, `M`) |
| PRICE    | Up to 7 digits and 2 decimals; stored as `0.00` format |

Lines starting with `#` and empty lines are ignored. Invalid or duplicate
lines are skipped when loading and reported on the message line. If the file
does not exist, the program starts with an empty database.

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
values if it is missing.

## Building

With FASM on DOS:

```
BUILD.BAT
```

With FASM on Linux (or any system with `make`):

```
make
```

Both produce `bin/STOCK.COM`. A prebuilt `bin/STOCK.COM` is included.

## Running

Copy `STOCK.COM` to a directory and run `STOCK` from there. The data files
are read from and written to the current directory; both are created when
needed. The program requires the display to be in
monochrome text mode 7 and exits with a message otherwise.

In DOSBox, set `machine=hercules` in the configuration to emulate a
monochrome adapter.

## Source layout

| File | Contents |
|------|----------|
| `src/STOCK.ASM`    | Entry point, main loop |
| `src/CONST.INC`    | Constants, record layout, macros |
| `src/VIDEO.INC`    | Direct MDA video output, cursor, speaker beep |
| `src/STRING.INC`   | String and number routines, string builder |
| `src/FILEIO.INC`   | Buffered DOS file I/O |
| `src/DB.INC`       | In-memory database, `DATA.DAT` load/save |
| `src/CONFIG.INC`   | `STOCK.CFG` load/save |
| `src/UI.INC`       | Screen frame, message line, keyboard, form engine |
| `src/SECTIONS.INC` | The Search, Add, Update, Delete, Config and Help sections |
| `src/DATA.INC`     | Strings, forms and variables |

## License

MIT - see [LICENSE](LICENSE).
