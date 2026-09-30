#!/bin/sh
# Build STOCK.COM with the flat assembler (fasm) on Linux/Unix
set -e

mkdir -p bin

fasm src/STOCK.ASM bin/STOCK.COM
