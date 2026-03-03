#include <stdio.h>

int main(void)
{
    int (*fp)(const char *) = puts;

    printf("puts        = %p\n", (void *)puts);
    printf("*puts       = %p\n", (void *)*puts);
    printf("&puts       = %p\n", (void *)&puts);
    printf("********puts= %p\n", (void *)********puts);
    printf("\n");
    printf("fp          = %p\n", (void *)fp);
    printf("*fp         = %p\n", (void *)*fp);
    printf("&fp         = %p\n", (void *)&fp);
    printf("\n");
    printf("call: (********puts)(\"Hello\")\n");
    int ret = (********puts)("Hello");
    printf("return value: %d\n", ret);
    return 0;
}
