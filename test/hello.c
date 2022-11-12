
int A;

int plus10(int c0, int c1, int c2, int c3, int c4, int c5, int c6, int c7, int c8, int c9)
{
	int x = c0+c1+c2+c3+c4+c5+c6+c7+c8+c9;
	return x;
}

void callplus()
{
	int a0 = A;
	int a1 = 1;
	int a2 = 2;
	int res = plus10(a0,a1,a2,3,4,5,6,7,8,9);
}

void testopt(int X, int Y, int Z)
{
	int a = X;
	int b = Y;
	int c = Z;
	int d = a + b;
	int e = a + b;
	int f = b + c;
	int g = c + b;

	int h = d + f;
	int i = e + g;
	int j = 2 * d;
	plus10(a,b,c,d,e,f,g,h,i,j);
}

int add(int x[][5], int y)
{
	int s = 0;
	while(y > 0) {
		y = y - 1;
		s = s + x[0][y];
	}
	return s;
}

int a[1][5];

int main()
{
	return add(a, 5);
}
