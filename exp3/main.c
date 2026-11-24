/*
 * 实验3  程序的机器级表示——顺序、分支和循环结构
 * -----------------------------------------------------------------------------
 * 本文件把实验指导书（一）（二）（三）三个部分的全部源程序集中在一个 .c 里，
 * 便于用同一条编译命令生成 -O1 汇编，与指导书给出的汇编逐条对照。
 *
 * 编译（Debian 课程环境：32 位、可调试、非 PIE，汇编地址形如 0x0804xxxx）：
 *   gcc -g -m32 -O1 -fno-omit-frame-pointer -fno-pic -no-pie \
 *       -z execstack -fno-stack-protector exp3/main.c -o exp3/main
 * 生成汇编（与指导书逐条对照）：
 *   gcc -m32 -O1 -fno-omit-frame-pointer -fno-pic -S exp3/main.c -o exp3/main.s
 * 反汇编：
 *   objdump -d exp3/main           或   gdb 中 disas 函数名
 *
 * 本机（Windows + TDM-GCC 10.3.0）验证用命令，去掉 MinGW 链接器不支持的
 * -z execstack / -no-pie：
 *   gcc -g -m32 -O1 -fno-omit-frame-pointer exp3/main.c -o exp3/main.exe
 *
 * 必须加 -fno-omit-frame-pointer：否则 -O1 会省略帧指针，汇编里看不到
 * pushl %ebp / movl %esp,%ebp，参数也不再是 8(%ebp)/12(%ebp) 形式，
 * 无法与指导书给出的汇编逐条对照。
 *
 * 说明：__attribute__((noinline)) 只是禁止编译器把函数内联进 main，
 *       保证每个函数在目标代码中独立存在、可被 gdb 单独 disas，
 *       函数体本身与指导书完全一致。
 * -----------------------------------------------------------------------------
 */
#include <stdio.h>

#define NOINLINE __attribute__((noinline))

/* =============================================================================
 * （一）顺序结构
 * ========================================================================== */

/* 1. test —— 左移 2 位后算术右移 n 位
 *    C:  x <<= 2;  x >>= n;  return x;
 *    汇编关键：
 *      movl 8(%ebp), %eax    ; eax = x
 *      sall $2, %eax         ; x <<= 2
 *      movl 12(%ebp), %ecx   ; ecx = n
 *      sarl %cl, %eax        ; x >>= n（算术右移，保留符号位）
 *    注：C 语言对有符号数的 >> 由实现定义，GCC 采用算术右移（sarl）。
 */
NOINLINE int test(int x, int n) {
    x <<= 2;
    x >>= n;
    return x;
}

/* 2. suan1 —— 阅读汇编并加注释
 *    C:
 *      int t1 = x + y;
 *      int t2 = z * 48;
 *      int t3 = t1 & 0xFFFF;
 *      int t4 = t2 * t3;
 *      return t4;
 *    汇编关键（常数乘法优化，两次移位代替 imull $48）：
 *      movl 16(%ebp), %eax       ; eax = z
 *      leal (%eax,%eax,2), %eax  ; eax = z + 2z = 3z
 *      sall $4, %eax             ; eax = 3z * 16 = 48z
 *    注意（写进分析）：指导书汇编中 &0xFFFF 的对象是 y（12(%ebp)），
 *    并未出现 x（8(%ebp)），即 t1 实际应为 y；此处保留指导书左侧 C 原文。
 */
NOINLINE int suan1(int x, int y, int z) {
    int t1 = x + y;
    int t2 = z * 48;
    int t3 = t1 & 0xFFFF;
    int t4 = t2 * t3;
    return t4;
}

/* 3. suan2 —— 根据右侧汇编反填左侧 C 代码
 *    汇编：
 *      pushl %ebp
 *      movl  %esp, %ebp
 *      movl  12(%ebp), %eax      ; eax = y
 *      xorl  8(%ebp), %eax       ; eax = y ^ x
 *      notl  %eax                ; eax = ~(x ^ y)
 *      subl  16(%ebp), %eax      ; eax = ~(x ^ y) - z
 *      popl  %ebp
 *      ret
 *    把一条表达式拆成四个临时变量填入：
 */
NOINLINE int suan2(int x, int y, int z) {
    int t1 = x ^ y;   /* eax = y ^ x */
    int t2 = ~t1;     /* not %eax   */
    int t3 = t2 - z;  /* sub 16(%ebp) */
    int t4 = t3;      /* 返回值即 t4 */
    return t4;
}

/* =============================================================================
 * （二）分支结构
 * ========================================================================== */

/* cond —— if (p && a > 0) *p += a;
 *   O1 汇编用两个条件跳转实现 && 的短路求值：
 *      testl %eax, %eax   ; p == NULL ?
 *      je    .L4          ; 是则跳过
 *      testl %edx, %edx   ; a <= 0 ?
 *      jle   .L4          ; 是则跳过
 *      addl  %edx, (%eax) ; *p += a
 */
NOINLINE void cond(int a, int *p) {
    if (p && a > 0) {
        *p += a;
    }
}

/* =============================================================================
 * （三）循环结构
 * ========================================================================== */

/* dw_loop —— do-while 结构
 *   C:
 *      do { x += n; y *= n; n--; } while (n > 0 && y < n);
 *      return x;
 *   寄存器分配：
 *      %eax <-> x      %ecx <-> y      %edx <-> n
 *   汇编控制流：
 *      .L3: addl %edx,%eax   ; 循环体 body-statement
 *           imull %edx,%ecx
 *           subl $1,%edx
 *           testl %edx,%edx  ; test-expr: n > 0
 *           jle  .L5         ; n <= 0 则退出
 *           cmpl %edx,%ecx   ; test-expr: y < n
 *           jl   .L3         ; 继续循环
 *      .L5: popl %ebp; ret
 *   注：本机 GCC 10 在 -O1 下把 `n > 0 && y < n` 编成 cmov 无分支代码
 *       （cmovns/cmpl/jg），与指导书里老版本 gcc 生成的两次条件跳转不同；
 *       指令语义等价，这一点写入「实验分析与思考」作为对比。
 */
NOINLINE int dw_loop(int x, int y, int n) {
    do {
        x += n;
        y *= n;
        n--;
    } while (n > 0 && y < n);
    return x;
}

/* =============================================================================
 * main —— 依次调用三类结构，给出运行结果
 * ========================================================================== */
int main(void) {
    printf("============ 实验3 程序的机器级表示 ============\n");

    /* ---------- （一）顺序结构 ---------- */
    printf("\n[顺序结构 1] test(20, 2)   = %d\n", test(20, 2));
    printf("[顺序结构 1] test(-20, 2)  = %d   <- 算术右移，符号位扩展\n", test(-20, 2));
    printf("[顺序结构 1] test(-20, 4)  = %d\n", test(-20, 4));

    printf("[顺序结构 2] suan1(3, 5, 7)  = %d\n", suan1(3, 5, 7));
    printf("[顺序结构 2] suan1(1, 2, 3)  = %d\n", suan1(1, 2, 3));

    printf("[顺序结构 3] suan2(6, 3, 11) = %d\n", suan2(6, 3, 11));
    printf("[顺序结构 3] suan2(0, 0, 5)  = %d\n", suan2(0, 0, 5));

    /* ---------- （二）分支结构 ---------- */
    printf("\n[分支结构]   ---- 以下验证短路求值 ----\n");
    {
        int a = 5, x = 6;
        printf("  调用前: a = %d, x = %d\n", a, x);
        cond(a, &x);                       /* p != NULL 且 a > 0 -> x += 5 */
        printf("  cond(5, &x)   后: x = %d   (p 非空且 a>0，执行 *p += a)\n", x);

        a = 5;
        printf("  调用前: a = %d, x = %d\n", a, x);
        cond(0, &x);                       /* a <= 0 -> 不修改 */
        printf("  cond(0, &x)   后: x = %d   (a<=0，第一个分支不成立)\n", x);

        a = -3;
        printf("  调用前: a = %d, x = %d\n", a, x);
        cond(a, &x);                       /* a < 0 -> 不修改 */
        printf("  cond(-3, &x)  后: x = %d   (a<0，第二个分支不成立)\n", x);
    }

    /* ---------- （三）循环结构 ---------- */
    printf("\n[循环结构]   dw_loop(5, 4, 3) = %d\n", dw_loop(5, 4, 3));
    printf("[循环结构]   dw_loop(1, 1, 4) = %d\n", dw_loop(1, 1, 4));

    return 0;
}
