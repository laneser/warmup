#include <stdio.h>

struct opaque;
struct opaque *create(void);
void destroy(struct opaque *);

#ifdef TEST_POINTER
/* Declare a pointer to incomplete type — should always work */
struct opaque *ptr;
#endif

#ifdef TEST_OBJECT
/* Try declaring an object of incomplete type */
struct opaque x;
#endif

#ifdef TEST_ARRAY
/* Try declaring an array of incomplete type */
struct opaque a[3];
#endif

#ifdef TEST_SIZEOF_STRUCT
/* Try sizeof on incomplete type */
size_t s = sizeof(struct opaque);
#endif

int main(void)
{
    printf("sizeof(struct opaque *) = %zu\n", sizeof(struct opaque *));
    return 0;
}
