savedcmd_custom_midi.o := ld -m elf_x86_64 -z noexecstack --no-warn-rwx-segments   -r -o custom_midi.o @custom_midi.mod 
