savedcmd_custom_midi.mod := printf '%s\n'   main.o fops.o hw_sim.o | awk '!x[$$0]++ { print("./"$$0) }' > custom_midi.mod
