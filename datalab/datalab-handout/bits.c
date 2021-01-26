/* 
//done 17
//10 14  20
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
/* Copyright (C) 1991-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */
/* This header is separate from features.h so that the compiler can
   include it implicitly at the start of every compilation.  It must
   not itself include <features.h> or any other header that includes
   <features.h> because the implicit include comes before any feature
   test macros that may be defined in a source file before it first
   explicitly includes a system header.  GCC knows the name of this
   header in order to preinclude it.  */
/* glibc's intent is to support the IEC 559 math functionality, real
   and complex.  If the GCC (4.9 and later) predefined macros
   specifying compiler intent are available, use them to determine
   whether the overall intent is to support these features; otherwise,
   presume an older compiler has intent to support these features and
   define these macros by default.  */
/* wchar_t uses Unicode 10.0.0.  Version 10.0 of the Unicode Standard is
   synchronized with ISO/IEC 10646:2017, fifth edition, plus
   the following additions from Amendment 1 to the fifth edition:
   - 56 emoji characters
   - 285 hentaigana
   - 3 additional Zanabazar Square characters */
/* We do not support C11 <threads.h>.  */

//need
//done1
/* 
 * bitXor - x^y using only ~ and & 
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1

 *///(a&~b)+(~a&b)   (a&~b)|(~a&b)//4  10 left
int bitXor(int x, int y) {
  return ~((~(x & (~y))) & (~((~x) & y)));
}return (~ (x & y)) & (~ (~x & ~y)); 
//11 1  10 0  00 0
//11 0  10 0  00 1
//11 0  10 1  00 0

//done2
/* 
 * evenBits - return word with all even-numbered bits set to 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 8
 *   Rating: 1
 */
int evenBits(void) {
	int result = 85;
	result += (result << 8);
	result += (result << 16);
  return result;
}



//done3
/* 
 * fitsShort - return 1 if x can be represented as a 
 *   16-bit, two's complement integer.//16位 2进制整型补码 
 *   Examples: fitsShort(33000) = 0, fitsShort(-32768) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 8
 *   Rating: 1
 
 */// >=32768 或 <-32768 的就不行,即为2^(16-1)=32768,即为16位的tmax,tmin 
 //return (!((x>>15)^0))|((!((x>>16)^(~0)))&(!((x>>15)^(~0))));
 //想到用移位,把几个特点都表示出来,发现超了之后,发现其实有的特点可以不用表示,已经被涵盖了 
int fitsShort(int x) {
	return !(((x << 16) >> 16) ^ x);
  //return (!((x>>15)^0))|(!((x>>15)^(~0)));
}

//need6
//done 4
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
 //0x7FFFFFFF 
 //x+1==~x
 //x=-1时比较特殊,得另外弄 
 
int isTmax(int x) {
  return (!((x + 1) ^ (~x))) & (!!(x + 1));//其他数前面这些会等于0 
}

//need7
//done5
/* 
 * fitsBits - return 1 if x can be represented as an 
 *  n-bit, two's complement integer.
 *   1 <= n <= 32
 *   Examples: fitsBits(5,3) = 0, fitsBits(-4,3) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
 //return (!((x>>15)^0))|(!((x>>15)^(~0)));
int fitsBits(int x, int n) {
	return !(((x << (~n + 1)) >> (~n + 1)) ^ x);
  //return (!((x>>(n+31)^0))|(!(x>>(n+31)^(~0))));
}//32-n


//done6
/* 
 * upperBits - pads n upper bits with 1's
 *  You may assume 0 <= n <= 32
 *  Example: upperBits(4) = 0xF0000000
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 10
 *  Rating: 1
 *///前n个弄成1 

 //n=0怎么弄 ???
 //找到特殊之处,怎样把特殊的表示出来 
int upperBits(int n) {
	int x = 1;
	x = x << 31;
	x = x >> (n + 31); 
  	return x + (!n);
}




//done7
/* 
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
 //只要奇数位设为1，偶数为无所谓 
int allOddBits(int x) {//1010 1010 //2+8+32+128=170 
	int n = 170;
	n += (n << 8);
	n += (n << 16);
  	return !((x & n) ^ n);
}



//done8
/* 8
 * byteSwap - swaps the nth byte and the mth byte
 *  Examples: byteSwap(0x12345678, 1, 3) = 0x56341278
 *            byteSwap(0xDEADBEEF, 0, 2) = 0xDEEFBEAD
 *  You may assume that 0 <= n <= 3, 0 <= m <= 3
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 25
 *  Rating: 2
 *///27
 //8位一个字节,从右开始为0123  
int byteSwap(int x, int n, int m) {
	int mask1 = 255;
	int m8 = (m << 3), n8 = (n << 3); 

	
	int mask3 = ~((mask1 << n8) ^ (mask1 << m8));

	int x1 = ((x >> m8) << n8) & (mask1 << n8);
	int x2 = ((x >> n8) << m8) & (mask1 << m8);
	
	//怎么移位?//都移到0字节,然后再移到需要的	 
    return (x & mask3) + (x1 ^ x2);//mn0+
}



//done
/* 9
 * absVal - absolute value of x
 *   Example: absVal(-1) = 1.
 *   You may assume -TMax <= x <= TMax
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 10
 *   Rating: 4
 */
 //负数的是补码,补码取反,+1,就是对应的绝对值
 //正数就不变
 //直接都取反加1会变成相当于直接加个负号 
 //得利用正负数差异的左边第一位
 //利用移位？ 
int absVal(int x) {
	
	//(x>>31)//正数是0，负数是111111。。。。 
  return (((~x) + 1) & (x >> 31)) + (x & (~(x >> 31)));
}

int abs(int x)
{
    int res = x >> 31;//-  11111111  //+ 00000000
    res = (x + res) ^ res;
    return res;
}


/* 10
 * divpwr2 - Compute x/(2^n), for 0 <= n <= 30
 *  Round toward zero
 *   Examples: divpwr2(15,1) = 7, divpwr2(-33,4) = -2
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
 //6个 
int divpwr2(int x, int n) {
    int mask = (1 << n) + (~0);
    x = x + ((x >> 31) & mask);
    return x >> n; 
}

int divpwr2(int x, int n) {//补码右移本质是向负无穷取整
    int m = x >> 31;//x正数0x00000000,x负数0xffffffff
    int k = (1 << n) + (~0);//(2^n)-1
    //~0=0xffffffff //加完相当于设置从右前n位都是1
	//值就相当于是 (2^n)-1
    x = x + (m & k);//正数不变，负数变为 x+(2^n)-1
    return x >> n;
}//(x+(2^n)-1)/(2^n)


//done
/* 11
 * leastBitPos - return a mask that marks the position of the
 *               least significant 1 bit. If x == 0, return 0
 *   Example: leastBitPos(96) = 0x20
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2 
 */
 //只留下最后一位的1，其余都设成0 
 //只要用3个就可以做出来 
int leastBitPos(int x) {
	
	
	//原1 ~ 0  +1  1  &x  =1
	//原0 ~ 1  +1  0  &x  =0
	
	return ((~x) + 1) & x;
	 
}
//前面没有进位的没有影响到的都会变为0
//原来后面几位是0的，在~后会变成1，+1就会进位到第一个原来是1，的地方
//后面的还是会变为0
 

//done
/* 12  //跟18是一样的 
 * logicalNeg - implement the ! operator, using all of 
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
 //可以改成5个
  
//因为0取反+1还是0，所以考虑利用这点
	//因为最后要在不用！的情况下是0的时候得返回1，所以最后得0的结果得0然后与1亦或
	//所以在倒数第二步得让0的结果得0，其余的结果得1
  
  //(t^x)判断是否取反+1等于本身，是的话为0，不是就是其他七七八八的数
  //最后再+1的话，如果不是后面每一位都是1，是不会进位到前面的
  //所以第一位是0的话，取反+1后变成1，亦或完就是1 
  //再右移31位，相当于看左边第一位
  //再与1&，相当于把前面的31位全设为0，只看亦或后的左边第一位
  //再|k，相当于把亦或后左边第一位，与亦或前左边第一位比较 
  
  
  /*在不能用！的情况下，
  考虑到0在取反后+1还是等于0
  但是-2147483648在取反后+1也等于本身
  ( ((~x)+1)  ^ x )后，0和-2147483648的结果都等于0，
  其他数的结果不同，但是左边第一位一定等于1，
  所以将结果右移31位再&1来判断。
  所以(( ( ((~x)+1)  ^ x ) >>31 ) &1 )后结果是0的x是0或-2147483648，
  结果是1的是其他数
  0与-2147483648的区别在与左边第一位-2147483648是1，所以将x右移31位，再&1，
  此时只有原本是负数的结果才是1。
  再与(( ( ((~x)+1)  ^ x ) >>31 ) &1 )的结果进行或操作，
  结果是只有0的结果是0，其余数的结果都是1。所以此时再与1亦或即可得到结果。*/
  
  //就是要注意利用性质，有可能在某一操作下所有可能结果都不同，但是会有某一位
  //有共同的性质，此时就要进行移位操作，专门找到那一位来判断 
int logicalNeg(int x) {
  return ((((((~x) + 1) ^ x) >> 31) & 1) | ((x >> 31) & 1)) ^ 1;
}

int logicalNeg(int x) {
	int m = (~x) + 1;
  	return (~((x|m) >> 31)) & 1;
}


//done
/* 13 //????????????low>high怎么办 //把low的部份消掉 
 * bitMask - Generate a mask consisting of all 1's 
 *   lowbit and highbit
 *   Examples: bitMask(5,3) = 0x38
 *   Assume 0 <= lowbit <= 31, and 0 <= highbit <= 31
 *   If lowbit > highbit, then mask should be all 0's//这个怎么弄？ 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
 //在给出的最高位和最低位间全设为1 
 //因为没办法用循环，所以只能先用移位把对应的设成1,再考虑了左移右移后 
int bitMask(int highbit, int lowbit) {
	int mask = ~0;
	return ((mask << lowbit) ^ ((mask << highbit) << 1)) & (mask << lowbit);
}



/* 14
 * isLess - if x < y  then return 1, else return 0 
 *   Example: isLess(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
 //都是正的或者都是负的相减没问题
 //但是正减负、负减正 
 //x+  y+&x-y<0  1   y<0   0
 //x-  y-&x-y<0  1   y>0   1
int isLess(int x, int y) {
	int m=(~y)+1;
	//x+
	(!(x>>31))&(!(y>>31))&(!!((x+m)>>31))
	//x-
	(!!(x>>31))&((!(y>>31))|((!!(y>>31))&(!!((x+m)>>31))))
	
	
	
return ((!((x^y)>>31))&(!!((x+m)>>31)))|((!!(x>>31))&(!(y>>31)));
}
//xy同 0
((!((x^y)>>31))&(!!((x+m)>>31)))
//xy不同  
((!!(x>>31))&(!(y>>31)))
return ((!((x^y)>>31))&(!!((x+m)>>31)))|((!!(x>>31))&(!(y>>31)));



//done
/* 15????????n=0怎么办？ 
 * logicalShift - shift x to the right by n, using a logical shift
 *   Can assume that 0 <= n <= 31
 *   Examples: logicalShift(0x87654321,4) = 0x08765432
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3 
 */
 //把前面几位都设成0
 //但是在n=0的时候，第一位也会被弄成0 
int logicalShift(int x, int n) {
	int mask1 = (1 << 31);
	mask1 = mask1 >> n;//把前面几位都设成1，但是 n=0的时候，第一位得是0 
	//n=0,!n=1;n!=0,!n=0
	mask1 = mask1 + ((!n) << 31);
	mask1 = mask1 << 1;
	mask1 = ~mask1;
	return mask1 & (x >> n);
}

//done
/*16
 * satMul2 - multiplies by 2, saturating to Tmin or Tmax if overflow
 *   Examples: satMul2(0x30000000) = 0x60000000
 *             satMul2(0x40000000) = 0x7FFFFFFF (saturate to TMax)
 *             satMul2(0x60000000) = 0x80000000 (saturate to TMin)
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3
 */
 //7-10
int satMul2(int x) {
	int n = x << 1;
  return ( n & (~((n ^ x) >> 31)) ) | (((n ^ x) >> 31) & (~((x >> 31) ^ (1 << 31))));
}
//n^x=1 over (n^x)>>31  111111   ~  00000  -> 000000     x<0  |(1<<31)
//x>0  |(~(1<<31))
//1<<31   1000000000
//x<0 x>>31  11111111111   (~((x>>31)^(1<<31)))
//x>0 x>>31 0000000000
//n^x=0  >>31  0000000   ~  111111   ->不变 
//x>0,  0x7FFFFFFF  //x<0  0x80000000


//done
/*17 
 * subOK - Determine if can compute x-y without overflow
 *   Example: subOK(0x80000000,0x80000000) = 1,
 *            subOK(0x80000000,0x70000000) = 0, 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3
 */
 //8-12
 //+ - -或者- - +的才会over 
 //考虑符号
 //先加完了混完了再右移判断 
 //m=~y+1
 //x>0,m>0 y<0 >0
 //x<0,m<0 y>0 <0
 //x^y 0  1
 //得判断最后结果，还得判断xy之间 
int subOK(int x, int y) {
	int s = x + (~y) + 1;
  return !(((x ^ y) & (s ^ x)) >> 31);
}
// | ^ & +
//x^y 0  s^x 0/1 1!
//x^y 1  
	//s^x 0  1!
	//s^x 1  0
//(0 0/1)(1 0)
//(1 1)




//done
/* 18
 * bang - Compute !x without using !
 *   Examples: bang(3) = 0, bang(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
 //只有x=0的时候是1 
 //~x+1
int bang(int x) {
  	return ((((((~x) + 1) ^ x) >> 31) & 1) | ((x >> 31) & 1)) ^ 1;
}


int bang(int x) {
  int m = (~x) + 1;//the opposite
  return (~((x|m) >> 31)) & 1;
}//取相反数与原数比较 

//done 19
/*19
 * bitParity - returns 1 if x contains an odd number of 0's
 *   Examples: bitParity(5) = 0, bitParity(7) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 4
 */
 //如果有奇数个0,即有奇数个1 
 //之前的位可能会是1，所以在判断的时候得用0把前面的影响弄掉，就是不能直接用结果判断，得用1处理一下 
int bitParity(int x) {
	x = x ^ (x >> 16);
	x = x ^ (x >> 8);
	x = x ^ (x >> 4);
	x = x ^ (x >> 2);
	x = x ^ (x >> 1);
  return x & 1;
}




/*20
 * isPower2 - returns 1 if x is a power of 2, and 0 otherwise
 *   Examples: isPower2(5) = 0, isPower2(8) = 1, isPower2(0) = 0
 *   Note that no negative number is a power of 2.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 4
 *///8~11
 //首先得是正的，然后 只有1个1？ 
int isPower2(int x) {
  return (!(x >> 31)) & (~(!x)) & (!(x & (x + (~0))));
}
int isPower2(int x) {
 //判断是否为2的幂
 //要点：x+mask 为 x-1  x&(x-1) 相当于得到x去掉了最低的为1的那位后的数，
 //2的次幂数对应2进制应该只有1位1 同时还要保证x不为0和负数
  int mask = ~0;
  return (!(x & (x + mask)) & (!(x >> 31)) & (~(!x)));
}



/* 21
 * float_i2f - Return bit-level equivalent of expression (float) x
 *   Result is returned as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point values.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_i2f(int x) {
  return 2;
}



/*22
 * leftBitCount - returns count of number of consective 1's in
 *     left-hand (most significant) end of word.
 *   Examples: leftBitCount(-1) = 32, leftBitCount(0xFFF0F0F0) = 12
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 50
 *   Rating: 4
 */
int leftBitCount(int x) {
  return 2;
}
