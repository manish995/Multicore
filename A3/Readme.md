# Documentation
## Part1:
To generate the trace, run the below code:
```
make
../../../pin -t obj-intel64/addrtrace.so -- ./prog1 8
```
8 traces will be formed for all 8 threads which is used by our next program.



## Part2

To run the complete Coherence protocol for these traces please run:

```
g++ main.cpp
./a.out
```

This will print all the stats.
To run for all progs run the pin command and then ./a.out.