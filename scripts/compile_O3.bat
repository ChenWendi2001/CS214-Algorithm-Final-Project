@echo off

del *.exe
g++ -O3 ../main_greedy.cpp -o main_greedy.exe
g++ -O3 ../main_random.cpp -o main_random.exe
g++ -O3 ../main_network.cpp -o main_network.exe


pause&&exit