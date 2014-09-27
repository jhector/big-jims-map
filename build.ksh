#!/usr/bin/env katana
$e=load "shell"
$ehframe=dwarfscript compile "shell_mod.dws"
replace section $e ".eh_frame" $ehframe[0]
replace section $e ".eh_frame_hdr" $ehframe[1]
replace section $e ".gcc_except_table" $ehframe[2]
save $e "shell_mod"
!chmod +x shell_mod
