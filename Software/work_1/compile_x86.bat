@echo off
setlocal
pushd "%~dp0"

where cl >nul 2>nul
if errorlevel 1 (
    echo [ERROR] cl.exe not found.
    echo Please open "x86 Native Tools Command Prompt for VS" first,
    echo then run this batch file from that prompt.
    goto :end
)

echo [*] Compiling SSEx_vul.c as 32-bit executable...
cl /nologo /GS- SSEx_vul.c

:end
pause
popd
endlocal
