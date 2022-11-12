
int func1(int x, int y)
{
	return x + y;
}

int func2(int x, int y)
{
	return x * y;
}

int main()
{
	int x = func1(1,2);
	x = func2(x,x);
	while(1) ;
}
