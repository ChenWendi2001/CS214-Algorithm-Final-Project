@REM run this to generate data 
@REM    and test all schedulers
@echo off
:loop
python data_generator.py
main_random.exe
main_greedy.exe
main_networksum.exe
main_kgreedy.exe
main_networkneck.exe


pause
goto loop