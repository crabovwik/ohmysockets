#include <stdio.h>
#include <string.h>

#include "helper.h"

int main() {
    char *buffer = "123";
    unsigned char buffer_hash[MD5_SIZE_HEX + 1];

    if ((helper_md5((unsigned char *) buffer, strlen(buffer), buffer_hash)) != 0) {
        printf("[error] helper_md5\n");
        return 1;
    }

    printf("[ok] hash: %s\n", buffer_hash);

    return 0;
}
