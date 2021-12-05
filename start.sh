echo compiling ...
cc -std=c99 -Wall ./main.c mpc.c -ledit -lm -o ./build/alisp

echo compilation done, starting
./build/alisp