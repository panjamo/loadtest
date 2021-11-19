net stop spooler
ren C:\Windows\System32\spool\prtprocs\x64\TPSpoolFlsHook.dll _TPSpoolFlsHook.dll
reg delete "HKLM\SYSTEM\CurrentControlSet\Control\Print\Environments\Windows x64\Print Processors\.TPSpoolFlsHook" /f
reg delete "HKLM\SYSTEM\CurrentControlSet\Control\Print\Environments\Windows x64\Print Processors\ATPSpoolFlsHook" /f
net start spooler
del C:\Windows\System32\spool\prtprocs\x64\_TPSpoolFlsHook.dll
