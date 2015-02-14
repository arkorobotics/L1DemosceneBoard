#!/bin/bash

# I stole these commands from MPLAB X so I didn't have to keep running it
set -x

/opt/microchip/xc16/v1.23/bin/xc16-gcc   main.c  -o main.o -c -mcpu=24FJ256DA206  -MMD -MF main.o.d -mno-eds-warn  -g -omf=elf -mlarge-arrays -O0 -msmart-io=1 -Wall -msfr-warn=off &&
/opt/microchip/xc16/v1.23/bin/xc16-gcc   -o production.elf  main.o      -mcpu=24FJ256DA206 -omf=elf -mlarge-arrays -Wl,--local-stack,--defsym=__MPLAB_BUILD=1,,--script=p24FJ256DA206.gld,--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map=production.map,--report-mem &&
/opt/microchip/xc16/v1.23/bin/xc16-bin2hex production.elf -a  -omf=elf

