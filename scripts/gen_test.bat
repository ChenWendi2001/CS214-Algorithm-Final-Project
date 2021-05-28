@REM run this to generate data 
@REM    and test all schedulers
@echo off
set cnt=1
:loop
python data_generator.py
main_random.exe
main_greedy.exe
main_networksum.exe
main_kgreedy.exe
main_networkneck.exe

set /p in="copy y/n?"
if %in%==y goto copy

goto loop


:copy
set name="data"%cnt%
mkdir %name%
copy /y *.json %name%

set /a cnt=cnt+1

pause
goto loop