/* 实验3 程序的机器级表示：顺序、分支和循环结构 */
#include <stdio.h>

/* 禁止内联，保证每个函数在目标代码中独立出现 */
#define NOINLINE __attribute__((noinline))

/* 顺序结构1：左移 2 位后算术右移 n 位 */
NOINLINE int test(int x, int n) {
    x <<= 2;
    x >>= n;
    return x;
}

/* 顺序结构2：z*48 与低 16 位掩码相乘 */
NOINLINE int suan1(int x, int y, int z) {
    int t1 = x + y;
    int t2 = z * 48;
    int t3 = t1 & 0xFFFF;
    int t4 = t2 * t3;
    return t4;
}

/* 顺序结构3：按汇编反填，等价于 ~(x ^ y) - z */
NOINLINE int suan2(int x, int y, int z) {
    int t1 = x ^ y;
    int t2 = ~t1;
    int t3 = t2 - z;
    int t4 = t3;
    return t4;
}

/* 分支结构：p 非空且 a>0 时把 a 累加到 *p */
NOINLINE void cond(int a, int *p) {
    if (p && a > 0) {
        *p += a;
    }
}

/* 循环结构：do-while，循环体至少执行一次 */
NOINLINE int dw_loop(int x, int y, int n) {
    do {
        x += n;
        y *= n;
        n--;
    } while (n > 0 && y < n);
    return x;
}

int main(void) {
    printf("============ 实验3 程序的机器级表示 ============\n");

    printf("\n[顺序结构1] test(20, 2)   = %d\n", test(20, 2));
    printf("[顺序结构1] test(-20, 2)  = %d\n", test(-20, 2));
    printf("[顺序结构2] suan1(3, 5, 7)  = %d\n", suan1(3, 5, 7));
    printf("[顺序结构3] suan2(6, 3, 11) = %d\n", suan2(6, 3, 11));

    printf("\n[分支结构] ---- 三种参数，覆盖两条分支 ----\n");
    int a = 5, x = 6;
    cond(a, &x);
    printf("[分支结构] cond(5, &x)   -> x = %d\n", x);
    cond(0, &x);
    printf("[分支结构] cond(0, &x)   -> x = %d\n", x);
    cond(-3, &x);
    printf("[分支结构] cond(-3, &x)  -> x = %d\n", x);

    printf("\n[循环结构] dw_loop(5, 4, 3) = %d\n", dw_loop(5, 4, 3));
    printf("[循环结构] dw_loop(1, 1, 4) = %d\n", dw_loop(1, 1, 4));

    return 0;
}
