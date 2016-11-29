#include <stdio.h>
#include <stdlib.h>

//CptS 360 HW 1
//Luke Darrow 11349190

int *FP;

main(int argc, char *argv[ ], char *env[ ])
{
  int a,b,c;
  printf("enter main\n");

  asm("movl %ebp, FP");    // CPU's ebp register is the FP
  printf("FP = %8x\n", FP);
  
  printf("&argc=%x argv=%x env=%x\n", &argc, argv, env);
  printf("&a=%8x &b=%8x &c=%8x\n", &a, &b, &c);

  a=1; b=2; c=3;
  A(a,b);
  printf("exit main\n");
}

int A(int x, int y)
{
  int d,e,f;
  printf("enter A\n");

  asm("movl %ebp, FP");    // CPU's ebp register is the FP
  printf("FP = %8x\n", FP);

  // PRINT ADDRESS OF d, e, f
  printf("&d=%x &e=%x &f=%x &x=%x, &y=%x\n", &d, &e, &f, &x, &y);
  d=4; e=5; f=6;
  B(d,e);
  printf("exit A\n");
}

int B(int x, int y)
{
  int g,h,i;
  printf("enter B\n");

  asm("movl %ebp, FP");    // CPU's ebp register is the FP
  printf("FP = %8x\n", FP);

  // PRINT ADDRESS OF g,h,i
  printf("&g=%x &h=%x &i=%x &x=%x, &y=%x\n", &g, &h, &i, &x, &y);
  g=7; h=8; i=9;
  C(g,h);
  printf("exit B\n");
}

int C(int x, int y)
{
  int u, v, w, i, *p;

  printf("enter C\n");
  // PRINT ADDRESS OF u,v,w;
  printf("&u=%x &v=%x &w=%x &x=%x &y=%x\n", &u, &v, &w, &x, &y);
  u=10; v=11; w=12;

  asm("movl %ebp, FP");    // CPU's ebp register is the FP
  printf("FP = %8x\n", FP);
  FP = &u;

  for(i = 0; i < 150; i++)
  {
      printf("address = %8x\n", FP);
      FP++;
  }
}
