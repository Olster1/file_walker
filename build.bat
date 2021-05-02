@echo off
set commonCompilerFlags=-nologo -FC -Zi -Gm- -GR- -EHa- -Zo -Oi -Zi -O2

cl %commonCompilerFlags% main.c /link shlwapi.lib

REM.\main.exe ../md_files/ ../