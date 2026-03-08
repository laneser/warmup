/*
 * test_quiz1_q1_bug.c - Run shared tests against ORIGINAL implementation.
 *
 * Same 10 tests as test_quiz1_q1.c, but linked against quiz1_q1.c
 * (original). Each test runs in a fork() child to survive segfaults
 * and infinite loops. Expected: several FAILs confirming issues.
 */

#include "unity.h"
#include "quiz1_q1_tests.h"

#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

/* Original implementations (defined in quiz1_q1.c) */
extern void FuncB(struct node **start, int value);
extern void FuncC(struct node **start, int value1, int value2);
extern void bubble_sort(struct node **start, int length);

void setUp(void) {}
void tearDown(void) {}

/*
 * Run a Unity test function in a child process.
 * Returns: 0 = test passed, >0 = assertion failure, -1 = crash, -2 = timeout
 */
static int run_in_child(void (*func)(void), int timeout_sec)
{
    fflush(stdout);
    fflush(stderr);
    pid_t pid = fork();
    if (pid == 0) {
        freopen("/dev/null", "w", stdout);
        freopen("/dev/null", "w", stderr);
        alarm(timeout_sec);
        func();
        _exit(0);
    }
    int status;
    waitpid(pid, &status, 0);
    if (WIFSIGNALED(status)) {
        int sig = WTERMSIG(status);
        if (sig == SIGALRM || sig == SIGKILL)
            return -2;
        return -1;
    }
    return WEXITSTATUS(status);
}

static const char *child_status_str(int r)
{
    if (r == 0)
        return "passed";
    if (r == -1)
        return "crashed (SIGSEGV)";
    if (r == -2)
        return "timed out (infinite loop)";
    return "assertion failed";
}

/*
 * Wrapper: run a shared test in fork, FAIL if child didn't exit 0.
 * This way the same 10 tests produce PASS/FAIL naturally.
 */
#define FORKED_TEST(test_func)                                        \
    void forked_##test_func(void)                                     \
    {                                                                 \
        int r = run_in_child(test_func, 2);                           \
        TEST_ASSERT_EQUAL_INT_MESSAGE(0, r, child_status_str(r));     \
    }

FORKED_TEST(test_FuncB_empty_list)
FORKED_TEST(test_FuncB_prepend)
FORKED_TEST(test_FuncC_empty_list)
FORKED_TEST(test_FuncC_value_not_found)
FORKED_TEST(test_FuncC_insert_after)
FORKED_TEST(test_bubble_sort_basic)
FORKED_TEST(test_bubble_sort_already_sorted)
FORKED_TEST(test_bubble_sort_reverse)
FORKED_TEST(test_bubble_sort_single)
FORKED_TEST(test_bubble_sort_duplicates)

int main(void)
{
    impl_FuncB = FuncB;
    impl_FuncC = FuncC;
    impl_bubble_sort = bubble_sort;

    UNITY_BEGIN();

    RUN_TEST(forked_test_FuncB_empty_list);
    RUN_TEST(forked_test_FuncB_prepend);
    RUN_TEST(forked_test_FuncC_empty_list);
    RUN_TEST(forked_test_FuncC_value_not_found);
    RUN_TEST(forked_test_FuncC_insert_after);
    RUN_TEST(forked_test_bubble_sort_basic);
    RUN_TEST(forked_test_bubble_sort_already_sorted);
    RUN_TEST(forked_test_bubble_sort_reverse);
    RUN_TEST(forked_test_bubble_sort_single);
    RUN_TEST(forked_test_bubble_sort_duplicates);

    return UNITY_END();
}
