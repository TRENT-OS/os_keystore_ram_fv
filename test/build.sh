rm test
rm *.o
gcc -c -I../googletest/googletest/include KeystoreRamFVTest.cpp
gcc -c -I../googletest/googletest/include -I../stdlib_fv ../KeystoreRamFV.c
gcc -c -I../stdlib_fv ../stdlib_fv/stdlib_fv.c
g++ -o test KeystoreRamFV.o KeystoreRamFVTest.o stdlib_fv.o -Wl,-L/home/a/tmp/googletest/googletest/build/lib -Wl,-lgtest -Wl,-lpthread
./test
