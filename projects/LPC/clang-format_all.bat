%~d0
cd %~dp0
for /R %%G in (*.c *.cpp *.h *.inc) do (
  if "%%~nG" NEQ "LPC43xx" (
    clang-format.exe --verbose -i -style=file "%%G"
  )
)
pause
