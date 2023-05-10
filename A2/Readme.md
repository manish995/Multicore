# Documentation
## Part1:
To generate the trace, run the below code:
```
make
../../../pin -t obj-intel64/addrtrace.so -- ./prog1 8
```
mem_trace.out file will have all the machine access

## Part2:
To generate the cdf, run the below code
```
g++ cdf.cpp -o cdf
./cdf mem_trace.out > out.txt
```
out.txt will contain all the distances and number of blocks access with that access distance.


## Part3:
To generate the cdf of lru cache miss trace, run the below code:
```
g++ lru_cdf.cpp -o lrucdf
./lrucdf mem_trace.out > out.txt
```
out.txt will contain all the discatnces and number of block acess with that access distance in miss trace.
At the end of out.txt statistics regarding the cache is present;

## Part4

To generate the sharing profile, run the below code:
```
g++ sharing_profile.cpp -o share
./share mem_trace.out > out.txt
```
out.txt will have the output of the sharers.
