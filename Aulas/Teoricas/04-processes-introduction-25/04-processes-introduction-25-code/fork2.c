int main()
{
   int x;
   x = 0;
   printf("YYYY\n");
   int n = fork();
   if (n ==0) {	
      while (1){
	 printf("xxxx %d\n", x++);
      }
   }else{
      while (1){
	 printf("yyyy %d\n", x);
      }
   }
}
