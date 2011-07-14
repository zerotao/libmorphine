#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <morphine.h>

int
main(int argc, char ** argv) {
    chno_iovec_t    iov[3];
    char            d1[15];
    int             d2;
    char          * d3;
    chno_buffer_t * buf = chno_buffer_new();

    memset(d1, 'A', sizeof(d1));

    d2 = 50;
    d3 = strdup("foobar");

    iov[0].iov_len  = sizeof(d1);
    iov[0].iov_data = d1;
    iov[1].iov_len  = sizeof(d2);
    iov[1].iov_data = &d2;
    iov[2].iov_len  = strlen(d3);
    iov[2].iov_data = d3;

    chno_buffer_iovec_add(buf, iov, 3);

    printf("%zu\n", chno_buffer_length(buf));

    return 0;
}

