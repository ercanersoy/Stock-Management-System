#!/bin/sh
# Build STOCK.COM with the flat assembler
set -e

mkdir -p bin

fasm src/STOCK.ASM bin/STOCK.COM
