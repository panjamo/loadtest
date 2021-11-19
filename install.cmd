net stop spooler
copy .\TPSpoolFlsHook.dll C:\Windows\System32\spool\prtprocs\x64
reg add "HKLM\SYSTEM\CurrentControlSet\Control\Print\Environments\Windows x64\Print Processors\ATPSpoolFlsHook" /v "Driver" /d "TPSpoolFlsHook.dll" /t reg_sz /f
net start spooler