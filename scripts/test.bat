@REM test all schedulers
@echo off

:loop
echo "TEST"
main_random.exe
main_greedy.exe
main_networksum.exe
main_kgreedy.exe
main_networkneck.exe

pause
goto loop