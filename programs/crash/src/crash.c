//crash.c
//Tests exception handling
//Bryan E. Topp <betopp@betopp.com> 2021

int main(int argc, const char **argv)
{
	(void)argc;
	(void)argv;
	
	*(volatile int*)(0) = 0x1337;
	return 0;
}
