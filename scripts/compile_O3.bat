@echo off

g++ -O3 ../main_kgreedy.cpp -o main_kgreedy.exe
g++ -O3 ../main_greedy.cpp -o main_greedy.exe
g++ -O3 ../main_random.cpp -o main_random.exe
g++ -O3 ../main_networkneck.cpp -o main_networkneck.exe
g++ -O3 ../main_networksum.cpp -o main_networksum.exe


pause&&exit