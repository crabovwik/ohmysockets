```
// static lib
cd ./common/lib/mbedtls-2.16.6/build
cmake ..
make

// client + server
cd ./build
cmake ..
make

// exec
cd ./build
./server/server 1337

./client/client 127.0.0.1 1337
```
