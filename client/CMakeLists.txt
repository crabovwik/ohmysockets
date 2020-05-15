cmake_minimum_required(VERSION 3.16)
project(client C)

set(CMAKE_C_STANDARD 11)

add_executable(client main.c ../common/helper/helper.c ../common/helper/helper.h)

include_directories(/home/username/code/ohmysockets/common/lib/mbedtls-2.16.6/build/include)
link_directories(/home/username/code/ohmysockets/common/lib/mbedtls-2.16.6/build/library)
target_link_libraries(client /home/username/code/ohmysockets/common/lib/mbedtls-2.16.6/build/library/libmbedcrypto.a)
target_link_libraries(client /home/username/code/ohmysockets/common/lib/mbedtls-2.16.6/build/library/libmbedtls.a)
target_link_libraries(client /home/username/code/ohmysockets/common/lib/mbedtls-2.16.6/build/library/libmbedx509.a)
