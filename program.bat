..\..\openocd\openocd.exe -f interface/picoprobe.cfg -f target/rp2040.cfg -s d:\RaspberryPiPico\openocd\tcl  -c "program stepper2.elf reset"
pause
