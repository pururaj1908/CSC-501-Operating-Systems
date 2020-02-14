
#include <conf.h>
#include <math.h>

double pow(double x, int y)
{
	int i = 1;
	double ans = 1.0;
	if (x == 0)
	{ return 0; }
	if (y == 0)
	{ return 1;  }
	
	for (i=1;i<=y;i++)
	{
		ans = ans * x;
	}
	return ans;
}

double log(double x)
{

	int i = 1;
	double ans = 0;
	for (i=1;i<=20;i++)
	{
		if (i % 2 == 1)
		{
			ans = ans + (pow(x-1,i)/i);
		}
		else
		{
			ans = ans - (pow(x-1,i)/i);	
		}
	} 
	return ans;
}

double expdev(double lambda) {
    double dummy;
    do
        dummy= (double) rand() / 077777;
    while (dummy == 0.0);
    return -log(dummy) / lambda;
}


