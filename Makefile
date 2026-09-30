# Builds STOCK.COM with the flat assembler (https://flatassembler.net)
FASM ?= fasm

bin/STOCK.COM: src/*.ASM src/*.INC
	mkdir -p bin
	$(FASM) src/STOCK.ASM $@

clean:
	rm -f bin/STOCK.COM

.PHONY: clean
