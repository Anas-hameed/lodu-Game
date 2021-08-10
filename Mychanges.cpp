#include<iostream>
#include<iomanip>
#include"Mychanges.h"
using namespace std;

void intializeGrid(int width, int height)
{

	int x= 100; 
	int y= 800;
	int count=1;
	for(int i=0; i<gridsize; i++ )
	{
		x=100;
		for(int j=0; j<gridsize; j++ )
		{
			grid[i][j].x= 100;
			grid[i][j].y= 800;
			if(i==j)
			{
				grid[i][j].route=0;
			}
			else if(i==6 && (j==7||j==8))
			{
				grid[i][j].route=0;
			}
			else if(i==7 && (j==6||j==8))
			{
				grid[i][j].route=0;
			}
			else if(i==8 && (j==6||j==7))
			{
				grid[i][j].route=0;
			}
			else if(i==6 || j==6 || i==7 || j==7 || i==8 || j==8)
			{

				grid[i][j].route=count;
				count++;
				
			}
			else
			grid[i][j].route=10;
			x+=50;
			cout<<setw(2)<<grid[i][j].route<<"  ";
		}
		cout<<endl;
		y+=50;
	}
}