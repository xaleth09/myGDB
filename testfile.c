#include <stdio.h>
#include <stdlib.h>

void bar()
{
  printf("Hello from bar\n");
}

void foo(long foo2)
{
  long i;
  unsigned long x = 4231;
  for(i = 0; i < foo2; i++) {
    printf("Hello World: %ld\n", i);
    bar();
  }
} 

int main(int argc, char* argv[])
{
  long baz2 = 1324, baz3 = 75, baz4 = -9;
  long baz = 3;
  foo(baz);
  printf("Hello\n");
  if(argc > 1)
 	printf("%s\n", argv[1]);
  
  return EXIT_SUCCESS;
}
