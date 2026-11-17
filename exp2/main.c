#include <stdio.h>

int absVal(int x) {
    int m=(x>>31);
    return ((x^m)-m);
}

int addOK(int x,int y) {
    int sx=(x>>31);
    int sy=(y>>31);
    int sum=x +y;
    int ss=(sum>>31);
    int same_xy=~(sx^sy);
    int diff_xs=sx^ss;
    int over=same_xy&diff_xs;
    return !over;
}

int conditional(int x,int y,int z)
{
    int m=(!x)<<31>>31;
    return ((~m&y)|(m&z));
}

unsigned float_abs(unsigned f)
{
    unsigned sign=(f>>31);
    unsigned exp=(f>>23)&0xFF;
    unsigned frac=f&0x7FFFFF;
    int is_nan=(!!(exp == 0xFF))&(!!frac);
    int mask_nan=is_nan<<31>>31;
    unsigned abs_f=f&(~(1U<<31));
    return (mask_nan&f)|(~mask_nan&abs_f);
}

int main() {
    printf(">> absVal(-12)=%d\n",absVal(-12));
    printf(">> absVal(9)=%d\n",absVal(9));
    
    int a1 = 0x80000000;
    int a2 = 0x80000000;
    int a3 = 0x70000000;
    printf(">> addOK(0x80000000,0x80000000)=%d\n",addOK(a1,a2));
    printf(">> addOK(0x80000000,0x70000000)=%d\n",addOK(a1,a3));

    printf(">> conditional(3,4,5)=%d\n",conditional(3,4,5));
    printf(">> conditional(2,4,5)=%d\n",conditional(0,4,5));

    unsigned f_neg = 0xBF800000;
    unsigned f_nan = 0x7FC00001;
    printf(">> float_abs(0x%08X) -> 0x%08X\n",f_neg,float_abs(f_neg));
    printf(">> float_abs(0x%08X) -> 0x%08X\n",f_nan,float_abs(f_nan));
    
    return 0;
}
