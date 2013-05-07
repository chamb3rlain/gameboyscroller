..\..\bin\lcc -Wa-l -Wl-yp0x143=0x80 -Wl-m -Wl-j -DUSE_SFR_FOR_REG -c -o commc.o commc.c
..\..\bin\lcc -Wa-l -Wl-yp0x143=0x80 -Wl-m -Wl-j -DUSE_SFR_FOR_REG -c -o color.o color.c
..\..\bin\lcc -Wa-l -Wl-yp0x143=0x80 -Wl-m -Wl-j -DUSE_SFR_FOR_REG -o comm.gb commc.o color.o
