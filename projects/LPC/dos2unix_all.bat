%~d0
cd %~dp0
for /R %%f in (*.c *.cpp *.h *.inc *.ld *.sh *.txt *.md *.in *.service) do "C:\Program Files\Git\usr\bin\dos2unix.exe" "%%f"
pause
