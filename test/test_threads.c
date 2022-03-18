#undef NDEBUG
#include <assert.h>
#include <threads.h>

static int _thread(void *arg)
{
    assert(arg == 0);
    return 0;
}

void t0()
{
    thrd_t thread;
    assert(!thrd_create(&thread, _thread, 0));
    int res;
    assert(!thrd_join(thread, &res));
}

int main(int argc, char **argv)
{
    t0();
}
