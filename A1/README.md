# A1
## Group 10
## Manish (190477), Sarthak Rout (190772)

<hr>

1. Unzip the traces file in the folder where the source code is located
    
    ```
    Struture:
    A1
    |--README.md
    |--analyse.cpp
    |--cache.cpp
    |--cache.h
    |--results
            |--inclusive.txt
            |--nine.txt
            |--exclusive.txt
            |--2.txt
    |--traces
            |--bzip2.log_l1misstrace_0
            |--..
    ```
```bash
wget https://www.cse.iitk.ac.in/users/mainakc/2023Spring/traces.zip
```
2. Compile the executable
```bash
g++ -o analyse analyse.cpp cache.cpp cache.h -O3
```
3. To run the executable for problem 1, pass either 'i' for Inclusive Policy, 'n' for Not-Inclusive-Not-Exclusive Policy, 'e' for Exclusive Policy to the executable. It will use this policy and process all trace files.
For example, for using inclusive policy,
```bash
./analyse i
```
4. It outputs the Hits and Misses for both layers of cache for each of the six applications.
```
Application to analyse traces
Choosing Inclusive Policy

Analysing trace for bzip2
Count: 10657625
Printing Statistics
Layer 2 Hits and Misses:
5259459(49%)	5398166(51%)
Layer 3 Hits and Misses:
3951778(73%)	1446388(27%)

Analysing trace for gcc
Count: 14610809
Printing Statistics
...
```
5. You may check the `results/` folder for the complete results.

## Problem 2

1. You can run the executable with arguments '2' to show results for problem 2
```bash
./analyse 2
```

2. Output
```
Application to analyse traces
Problem 2 : Inclusive Policy. Will show results for problem 1 and 2

Analysing trace for bzip2
Printing Statistics
Layer 2 Hits and Misses:
5259461(49%)	5398166(51%)
Layer 3 Hits and Misses:
3951778(73%)	1446388(27%)
Printing Statistics for Problem 2
Layer 3 misses:
Full Associative Cache Misses: 1361401
Cold Misses: 119753
Capacity Misses: 1241648
Conflict Misses: 84987

Full Associative Cache Misses (With Belady): 536836
Misses with Belady:
Cold Misses: 119753
Capacity Misses: 417083
Conflict Misses: 909552

Analysing trace for gcc
Printing Statistics
Layer 2 Hits and Misses:
11574350(79%)	3036461(21%)
Layer 3 Hits and Misses:
1663059(54%)	1373402(46%)
...
```

3. You may check the `results/` folder for the complete results.
