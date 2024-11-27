#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "operations.h"

int main()
{
        Operations_T op = Operations_new();
        Operations_free(&op);

        printf("Exit Success\n");
        return EXIT_SUCCESS;
}