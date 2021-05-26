@REM run this to generate data 
@REM    and test all schedulers
@echo off
:loop
python data_generator.py
main_random.exe
main_greedy.exe
main_network.exe

pause
goto loop