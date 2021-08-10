#ifndef CENTIPEDE_CPP_
#define CENTIPEDE_CPP_
// #include "Board.h"
#include "util.h"
#include<iomanip>
#include <iostream>
#include<string>
#include<semaphore.h>
#include<cmath> 
#include<cstdlib>
#include<pthread.h>
#include<ctime>
using namespace std;

// Changes goes here 

// This is simple point structure with a fixed route index for each box in table
#define gridsize  15
#define Players    4
#define RouteSize 58
#define StopSize 8
#define Max_Con_turns 3    // this is for totoal consective Rolls
#define TurnConsective 3  // this is Max limit for player who got cancel if he does not rolls six or don't kill other Player

// Global Object Goes here

int TotalTokens= 4;
pthread_t Master;
pthread_t Player[Players];
bool check=True;
int Dice;
int CurScore;
int playerNo=-1; // This will indicate which player is rolling dice currently to show Animation Accordingly
string playerName[4];
sem_t Dice_Roll, Grid_Acess;
sem_t RoundTurns;
sem_t winCheck;
int countturn=0;
bool To_Cancel[Players]= {false, false, false, false};
int CurScreen=0;

// Coordinate structure for Grid
struct Cordinate
{
	int x;
	int y; 
	int route;
};

// Tokens for each player to show movement
struct Token
{
	Cordinate* Position; 
	int Index;
	bool Finish;
	Token()
	{
		Index=-1;
		Finish =false; 
	}
	
};

// Class Containing the Four Players
struct player
{
	int HitRates;
	int Noofturns;
	int SixCount;
	int cancelstate;
	int Position;
	Token* Tokens;
	player()
	{
		Position=0;
		cancelstate=0;
		SixCount=0; 
		Noofturns=0;
		Tokens= new Token[TotalTokens];
		HitRates=0;
	}
	bool compare(Token t, int j){
		for(int i=0;i<TotalTokens;i++){
			if(j != i && t.Index == Tokens[i].Index){
				return true;
			}
		}
		return false;
	}

	~player()
	{
		delete []Tokens;	
	}
}; 

// Board That will contains the logic of full Game
struct Board
{

	Cordinate grid[gridsize][gridsize];
	player Player_s[Players];
	int Routes[Players][RouteSize];
	int stops[StopSize];

	// Default Constructor for the board Class
	Board():Routes{{20, 21, 22, 23, 24, 16, 13, 10, 07, 04, 01, 02, 03, 06,  9, 12, 15, 18,
					25, 26, 27, 28, 29, 30, 42, 54, 53, 52, 51, 50, 49, 57, 60, 63, 66, 69, 
					72, 71, 70, 67, 64, 61, 58, 55, 48, 47, 46, 45, 44, 43, 31, 32, 33, 34, 35, 36,0,19},
				   { 6,  9, 12, 15, 18, 25, 26, 27, 28, 29, 30, 42, 54, 53, 52, 51, 50, 49,
				    57, 60, 63, 66, 69, 72, 71, 70, 67, 64, 61, 58, 55, 48, 47, 46, 45, 44, 
				    43, 31, 19, 20, 21, 22, 23, 24, 16, 13, 10, 7, 4, 1, 2, 5, 8, 11, 14, 17,0,3},
				   {53, 52, 51, 50, 49, 57, 60, 63, 66, 69, 72, 71, 70, 67, 64, 61, 58, 55,
				    48, 47, 46, 45, 44, 43, 31, 19, 20, 21, 22, 23, 24, 16, 13, 10, 7, 4, 1,
				     2,  3, 06,  9, 12, 15, 18, 25, 26, 27, 28, 29, 30, 42, 41, 40, 39, 38, 37,0,54},
				    {67, 64, 61, 58, 55, 48, 47, 46, 45, 44, 43, 31, 19, 20, 21, 22, 23, 24,
				    16, 13, 10, 7, 4, 1, 2, 3, 06,  9, 12, 15, 18, 25, 26, 27, 28, 29, 30, 42,
				    54, 53, 52, 51, 50, 49, 57, 60, 63, 66, 69, 72, 71, 68, 65, 62, 59, 56,0,70}},stops{20,7,6,28,53,66,67,45}
	{
		intializeGrid();
		intialize_Token_Pos();
	}

	// This function simple intialize the x,y location of each box
	// Assign the weight of each location in the board
	void intializeGrid()
	{
		int x= 100; 
		int y= 750;
		int count=1;
		for(int i=0; i<gridsize; i++ )
		{
			x=100;
			for(int j=0; j<gridsize; j++ )
			{
				grid[i][j].x= x;
				grid[i][j].y= y;
				if(i==j)
				{
					grid[i][j].route=0;
				}
				else if(i<6 && j<6)
				grid[i][j].route=-1;
				else if(i<6 && j>8)
				grid[i][j].route=-2;
				else if(i>8 && j<6)
				grid[i][j].route=-3;
				else if(i>8 && j>8)
				grid[i][j].route=-4;
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
				grid[i][j].route=0;

				cout<<setw(2)<<grid[i][j].route<<"  ";
				x+=50;
			}
			cout<<endl;
			y-=50;
		}
	}

	void intialize_Token_Pos()
	{
		int I_index[]= {1,1,10,10};
		int J_index[]= {2,11,11,2};
		int tem1,tem2;
		int check=0;
		for(int i=0; i<Players; i++ )
		{
			tem1=I_index[i];
			tem2=J_index[i];
			check=0;
			for(int j=0 ; j<TotalTokens ;j++ )
			{
				Player_s[i].Tokens[j].Position= &grid[tem1][tem2];
				check++;
				tem1+=2;
				if(check==2)
				{
					tem1=I_index[i];
					tem2+=2;
				}
			}
		}
	}

	void MenuDisplay()
	{
		DrawRectangle(0,0,1020,840,colors[WHITE]);
		DrawRectangle(20,20,980,800,colors[BLACK]);
		static int x=100,state=0;static float* ptr=colors[ORANGE];
		DrawRectangle(x,20,45,50,ptr);
		switch(state)
		{
			case 0:
			{   
				if(x<960)
				x+=5;
				else 
				{
					state=1;
					ptr=colors[BLUE_VIOLET];
				}
				break;
			}
			case 1:
			{
				if(x>10)
				x-=5;
				else
				{
					state=0;
					ptr=colors[ORANGE];
				}
				break;
			}
		}
		int x1=100;
		//Drawing Manu
		DrawRoundRect(150+x1,690,500,40,colors[WHITE]);
		DrawString(260,700,"LODU GAME  USING  MULTI_THREADING  ", colors[RED]);
		DrawRoundRect(150+x1,630,500,40,colors[WHITE]);
		DrawString(160+x1,640,"1) START NEW GAME       Press  S ", colors[BLACK]);
			
		DrawRoundRect(150+x1,570,500,40,colors[WHITE]);
		DrawString(160+x1,580,"2) PLAYER INFO                  Press  I ",colors[BLACK]);
		
		DrawRoundRect(150+x1,510,500,40,colors[WHITE]);
		DrawString(160+x1,520,"3) SELECT MODE                Press  M ",colors[BLACK]);
			
		DrawRoundRect(150+x1,450,500,40,colors[WHITE]);
		DrawString(160+x1,460,"4) EXIST                                Press Esc  ",colors[BLACK]);

	}
	void DrawBoard()
	{
		DrawBoardAstetic();
		DrawStop(6, 1);
		DrawTokens();
		DrawDice();
		DrawSideManu();
	}
	void DisplayPlayerInfo()
	{
		glClearColor(1/*Red Component*/, 1,	//148.0/255/*Green Component*/,
			1/*Blue Component*/, 1 /*Alpha component*/); // Red==Green==Blue==1 --> White Colour
		glClear (GL_COLOR_BUFFER_BIT);   
		DrawRectangle(100,200,700,500,colors[RED]);
		DrawRectangle(120,220,660,460,colors[BLACK]);
		DrawString(200,600,"PLAYER NAMES:: ",colors[RED]);
		DrawString(200,300,"PRESS B TO BACK TO MANU ",colors[RED]);
	}



	void DrawSideManu()
	{
		if(playerNo!=-1)
		{
			DrawString(880,640,playerName[playerNo]+"'s Turn",colors[WHITE]);
			DrawString(880,500,"Hit Rates", colors[WHITE] );
			int y=450;
			for(int i=0; i<Players; i++)
			{
				DrawString(880,y,playerName[i]+" ::"+Num2Str(Player_s[i].HitRates),colors[WHITE]);
				y-=40;
			}
		}
	}
	void DrawDice()             
	{
		int x=880;
		int y=700;
		DrawRoundRect(x,y,60,60,colors[WHITE], 10);
		if(Dice == 1)
		{
			DrawCircle(x+30,y+30,5,colors[BLACK]);
		}
		else if(Dice == 2)
		{
			for(int i=15;i<=45;i+=30){
				DrawCircle(x+i,y+i,5,colors[BLACK]);
			}
		}
		else if(Dice == 3)
		{
			for(int i=15;i<=45;i+=15){
				DrawCircle(x+i,y+i,5,colors[BLACK]);
			}			
		}
		else if(Dice == 4)
		{
			DrawCircle(x+15,y+15,5,colors[BLACK]);
			DrawCircle(x+45,y+45,5,colors[BLACK]);
			DrawCircle(x+15,y+45,5,colors[BLACK]);
			DrawCircle(x+45,y+15,5,colors[BLACK]);
			
		}
		else if(Dice == 5)
		{
			DrawCircle(x+15,y+15,5,colors[BLACK]);
			DrawCircle(x+45,y+45,5,colors[BLACK]);
			DrawCircle(x+15,y+45,5,colors[BLACK]);
			DrawCircle(x+45,y+15,5,colors[BLACK]);
			DrawCircle(x+30,y+30,5,colors[BLACK]);
		}
		else if(Dice == 6)
		{
			DrawCircle(x+15,y+15,5,colors[BLACK]);
			DrawCircle(x+15,y+30,5,colors[BLACK]);
			DrawCircle(x+15,y+45,5,colors[BLACK]);
			DrawCircle(x+45,y+15,5,colors[BLACK]);
			DrawCircle(x+45,y+30,5,colors[BLACK]);
			DrawCircle(x+45,y+45,5,colors[BLACK]);
		}
	}

	// Drawing stops using hard coding approach
	// This function Draw stops of respective color
	void DrawStop(int i,int j)
	{
		DrawSquare( grid[i][j].x, grid[i][j].y ,49,colors[GREEN_YELLOW]);
		DrawSquare( grid[i+2][j+1].x, grid[i+2][j+1].y ,49,colors[GREEN_YELLOW]);
		DrawSquare( grid[i][j+11].x, grid[i][j+11].y ,49,colors[NAVY]);
		DrawSquare( grid[i+2][j+12].x, grid[i+2][j+12].y ,49,colors[NAVY]);
		DrawSquare( grid[j+1][i].x, grid[j+1][i].y ,49,colors[ORANGE]);
		DrawSquare( grid[j][i+2].x, grid[j][i+2].y ,49,colors[ORANGE]);
		DrawSquare( grid[i+7][j+5].x, grid[i+7][j+5].y ,49,colors[RED]);
		DrawSquare( grid[i+6][j+7].x, grid[i+6][j+7].y ,49,colors[RED]);
	}


	
	//####################################################################### 
	// This function Will Draw Each Gotie according to it's Position Value
	// the index is kept to specify Each value at which currently goti it
	// ######################################################################
	void DrawTokens()
	{
		float* HomeCol[]= {colors[GREEN], colors[ORANGE], colors[BLUE], colors[RED]};
		float* Goticol2[]= {colors[GREEN_YELLOW], colors[DARK_ORANGE], colors[NAVY], colors[DARK_RED]};
		int x,y;
		for(int i=0 ; i<Players; i++)
		{
			for(int j=0; j<TotalTokens ; j++)
			{
				x= Player_s[i].Tokens[j].Position->x;
				y= Player_s[i].Tokens[j].Position->y;
				DrawCircle(x,y,30,HomeCol[i]);
				if(Player_s[i].Tokens[j].Index== -1)
				{
					DrawCircle(x,y,20,colors[BLACK]);
					DrawCircle(x,y,18,colors[DARK_GRAY]);
					DrawCircle(x,y,15,Goticol2[i]);

					DrawString(x-5,y-10,Num2Str(j), colors[WHITE]);
				}
				else
				{
					Cordinate* p=Compute_Index(Routes[i][Player_s[i].Tokens[j].Index]);
					x= p->x+25;
					y= p->y+25;
					
					if(Player_s[i].compare(Player_s[i].Tokens[j],j)){
						x= p->x+5;
						y= p->y+5;
						DrawRoundRect(x,y,40,40,Goticol2[i]);
						DrawString(x+15,y+10,Num2Str(j), colors[WHITE]);
					}
					else{
						if(Player_s[i].Tokens[j].Finish==true)
						{
							if(i==0)
							{
								x= grid[7][6].x;
								y= grid[7][6].y;
							}
							else if(i==1)
							{
								x= grid[6][7].x;
								y= grid[6][7].y;	
							}
							else if(i==2)
							{
								x= grid[7][8].x;
								y= grid[7][8].y;
							}
							else if(i==3)
							{	
								x= grid[8][7].x;
								y= grid[8][7].y;
							}
							x+=25;
							y+=25;
						}
						DrawCircle(x,y,20,colors[BLACK]);
						DrawCircle(x,y,18,colors[DARK_GRAY]);
						DrawCircle(x,y,15,Goticol2[i]);
						DrawString(x-5,y-10,Num2Str(j), colors[WHITE]);
					}
				}
			}
		}
	}

	// This function map integer route to the Global grid index
	// Calculate int ith and jth index for the given route value
	Cordinate* Compute_Index(int num)
	{
		int i=0, j=0;
		if(num<19)
		{
			i=num/3;
			j=num%3+5;
			if(num%3==0)
			{ 
				j+=3;
				i--;
			}
		}
		else if(num<55)
		{
			i= (num-19)/12+6; 
			j= (num-grid[i][0].route)%15; 
			if(j>5)
			j+=3; 
		}
		else
		{
			i=(num-54)/3+9;
			j=num%3+5;
			if(num%3==0)
			{ 
				j+=3;
				i--;
			}
		}
		return &grid[i][j];
	}

	void DrawEndScreen()
	{
		glClearColor(1/*Red Component*/, 1,	//148.0/255/*Green Component*/,
		1/*Blue Component*/, 1 /*Alpha component*/); // Red==Green==Blue==1 --> White Colour
		glClear (GL_COLOR_BUFFER_BIT);   
		DrawRectangle(100,200,700,500,colors[RED]);
		DrawRectangle(120,220,660,460,colors[BLACK]);
		int y=600;
		string str= "Player ";
		for(int i=0; i<Players; i++)
		{
			str+= playerName[i];
			str+= "   :: ";
			if(Player_s[i].cancelstate==1)
			str+= "Thread Was Cancelled";
			else
			{
				int c=0;
				for(int j=0; j<Players; j++)
				{
					if(Player_s[i].cancelstate==1)
					c++;	
				}
				str+= Num2Str(Player_s[i].Position-c);
			}
			DrawString(200,y,str,colors[RED]);
			y-=50;
			str= "Player ";
		}
		static bool b=true;
		if(b)
		{
			cout<<endl;
			for(int i=0; i<Players; i++)
			cout<<playerName[i]<<"  ::"<<Player[i]<<"-> "<<Player_s[i].cancelstate<<endl;
			b=false;
			cout<<endl;
		}
	}

	// This Finction Draw all the non moving section of the board
	void DrawBoardAstetic()
	{		
		// Green Home fixed		
		DrawSquare( 100 , 500 ,300,colors[GREEN_YELLOW]); //BOARD PATTERN
		DrawRoundRect(150,550,200,200,colors[WHITE],0);
		DrawString(210,760,playerName[0],colors[WHITE]);
		// Yellow Home fixed
		DrawSquare(550 , 500 ,300,colors[ORANGE]); 
		DrawRoundRect(600,550,200,200,colors[WHITE],0);
		DrawString(660,760,playerName[1],colors[WHITE]);
		// Blue Home fixed
		DrawSquare( 550 , 50 ,300,colors[NAVY]); //BOARD PATTERN
		DrawRoundRect(600,100,200,200,colors[WHITE],0);
		DrawString(660,310,playerName[2],colors[WHITE]);
		// Red home Fixed  
		DrawSquare( 100 ,50 ,300,colors[RED]); 
		DrawRoundRect(150,100,200,200,colors[WHITE],0);
		DrawString(210,310,playerName[3],colors[WHITE]);

		//CENTER BOX
		DrawSquare( 400 , 350 ,150,colors[FLORAL_WHITE]);
		DrawTriangle( 400, 350 , 475, 425 , 400 , 500, colors[GREEN_YELLOW] );
		DrawTriangle( 400, 500 , 475, 425 , 550 , 500, colors[ORANGE] );
		DrawTriangle( 550, 350 , 475, 425 , 550 , 500, colors[NAVY] );
		DrawTriangle( 550, 350 , 475, 425 , 400 , 350, colors[RED] );

		/* boarder */
		DrawLine( 95 , 45 ,  95 , 802 , 5 , colors[MISTY_ROSE] );    
		DrawLine( 93 , 804 ,  856 , 804 , 5 , colors[MISTY_ROSE] );
		DrawLine( 854 , 804 ,  854 , 45 , 5 , colors[MISTY_ROSE] );
		DrawLine( 93 , 44 ,  856 , 44 , 5 , colors[MISTY_ROSE] );

		// This is drawing the inner grid pattern 
		for(int i=0 ; i<gridsize ; i++ )
		{
			for(int j=0; j<gridsize ; j++)
			{
				if(grid[i][j].route>0 && grid[i][j].route<73)
				{
					DrawSquare( grid[i][j].x, grid[i][j].y ,49,colors[FLORAL_WHITE]);
				}
			}
		}
		// Centeral Colors
		for(int i=1;i<gridsize-1 ; i++ )
		{
			if(i==6 || i==7 || i==8)
			continue;
			if(grid[i][9].route==-2)
			DrawSquare( grid[i][7].x, grid[i][7].y ,49,colors[ORANGE]);
			else 
			DrawSquare( grid[i][7].x, grid[i][7].y ,49,colors[RED]);
			
			if(grid[9][i].route==-3)
			DrawSquare( grid[7][i].x, grid[7][i].y ,49,colors[GREEN_YELLOW]);
			else
			DrawSquare( grid[7][i].x, grid[7][i].y ,49,colors[NAVY]);
		}
	}

};


// This section Contains the declared Global Object
Board b; 

// This function Will Check the Dice for Hit Of Token With Rival Tokens
// This will not kill the player present on the save spot
void HitCheck(int Num)
{
	int index1, index2;
	for(int i=0; i<Players; i++)
	{
		if(i==Num)
		continue;
		for(int j=0; j<TotalTokens; j++)
		{
			index1= b.Player_s[Num].Tokens[j].Index;
			if(index1!=-1)
			{
				for(int k=0; k<TotalTokens; k++ )
				{
					index2=b.Player_s[i].Tokens[k].Index;
					if(b.Player_s[i].compare(b.Player_s[i].Tokens[k],k)){

					}
					if(index2!=-1 && b.Routes[Num][index1]==b.Routes[i][index2])
					{
						// this is check for the certain Save Zone on the Games
						bool c1=true;
						for (int i = 0; i<StopSize ;i++)
						{
							if(b.stops[i]==b.Routes[Num][index1])
							c1=false;
						}
						if(c1)
						{
							b.Player_s[Num].HitRates++; 
							b.Player_s[i].Tokens[k].Index= -1;
							sleep(1);
						}
					}
				}
			}
		}
	}	

}



// Threading start heres
// this Function Rolls the Dice
int RollDice()
{
	int score=0;
	for(int i=0; i<Max_Con_turns; i++)
	{
		score+=GetRandInRange(1,7);
		if(score%6!=0)
		break;
	}
	return score; 	
}


void GridAccessChecks(int score)
{
	while(score>0)
	{
		if(score<6)
		{
			Dice=score;
			cout<<"Player "<<playerName[playerNo]<<" ::"<<Dice<<endl;
			int count=0,index=0;
			for(int i=0; i<TotalTokens;  i++) 
			{
				if(b.Player_s[playerNo].Tokens[i].Index==-1)
				count++;
				else
				index=i;
			}
			if(count==TotalTokens-1)
			{	
				CurScore=Dice; 
				int temp= b.Player_s[playerNo].Tokens[index].Index+CurScore;
				if(temp<51 || b.Player_s[playerNo].HitRates!=0)
				{
					if(temp<56)
					{
						while(CurScore!=0)
						{
							b.Player_s[playerNo].Tokens[index].Index++;
							CurScore--;
						}
					}
					else if(temp==56)
					{
						b.Player_s[playerNo].Tokens[index].Finish=true;
						CurScore=0;	
					}
				}
				else if(b.Player_s[playerNo].HitRates==0)// this is check for double role of same goti if it has hitrate= 0
				{
					b.Player_s[playerNo].Tokens[index].Index=(temp+6)%RouteSize;
					CurScore=0;
				}
				HitCheck(playerNo);
			}
			else if(count==TotalTokens)
			{
				cout<<"Dice Roll Ignored All Token at Home"<<endl;

			}
			else// what to do When we have 2 or more token on the board
			{
				CurScore=Dice;
				cout<<"Enter a token Number To Move it"<<endl;
				while(CurScore!=0){}
				HitCheck(playerNo);
			}
		}
		else if(score<18)
		{
			Dice= 6;
			b.Player_s[playerNo].SixCount++;
			cout<<"Player "<<playerName[playerNo]<<" ::"<<Dice<<endl;
			CurScore=Dice;
			cout<<"Enter a Token(0-3) Number to Move or Place"<<endl;
			while(CurScore!=0){}
			HitCheck(playerNo);
		}
		else
		{
			Dice  = 6;
			sleep(1);
			Dice  = 0;
			sleep(1);
			Dice  =6;
			sleep(1);
			Dice  = 0;
			sleep(1);
			Dice=6;
			score-=18;
		}
		score-=Dice;
	}
	return;
}

bool c1=false;
// this function will check wheather a player Win or Not
// Also this will checks wheather a player get six 
bool Cancellation_Check(int No)
{
	bool check=true;
	for(int i=0; i<TotalTokens; i++)
	{
		if(b.Player_s[No].Tokens[i].Finish==false)
		check=false;
	}
	if(check==true)
	{
		c1=true;
		return true;
	}
	if(b.Player_s[No].Noofturns==TurnConsective)
	{
		if(b.Player_s[No].SixCount==0 && b.Player_s[No].HitRates==0)
		check=true;
	}	
	if(check)
	{
		b.Player_s[No].cancelstate=1;
		c1=true;
	}
	return check;
}


int totalp=4;
// this Section Contain master other threads
void* PlayerTurn(void* arg)
{
	int score;
	int p1=0;
	while(true)
	{
		// Roll the Dice
		sem_wait(&Dice_Roll);
			score=RollDice();
		sem_post(&Dice_Roll);

		// Move the token oven the Grid
		sem_wait(&Grid_Acess);
		playerNo=*(int*)arg;
		sleep(1);
		GridAccessChecks(score);
		p1=playerNo;
		playerNo=-1;
		sem_post(&Grid_Acess);

		// Make all number of player currently playing wait untill everyone completes
		sem_wait(&RoundTurns);
		countturn++; 
		if(countturn==totalp)
		{
			cout<<"\nAll Player Have Got turn in this Round\n\n";
			countturn=0;
			sleep(1);
		}
		sem_post(&RoundTurns);
		while(countturn!=0){}

		// Check wheather a thread Has satisfy Win Condition or not
		sem_wait(&winCheck);
			p1 = *(int*)arg;
			b.Player_s[p1].Noofturns++;
			To_Cancel[p1]= Cancellation_Check(p1);
			if(b.Player_s[p1].Noofturns==TurnConsective)
			{
				b.Player_s[p1].Noofturns=0;
				b.Player_s[p1].SixCount=0;
			}
			if(To_Cancel[p1])
			totalp--;
		sem_post(&winCheck);
		while(c1){}

	}
	pthread_exit(NULL);
}


// This is Master thread who can cancel and end game at any stage
void* Master_Thread(void* arg)
{
	int count=0; 
	int Arr[]  = {0,1,2,3};
	bool flag[]= {true, true, true, true};
	//threads created 
	for(int i=0; i< Players; i++ )
	pthread_create(&Player[i], NULL,&PlayerTurn,&Arr[i]);
	for(int i=0; i<Players ; i++)
	pthread_detach(Player[i]);
	while(true)
	{
		for(int i=0; i<Players; i++)
		{
			if(To_Cancel[i] && flag[i])
			{
				sem_wait(&winCheck);
					cout<<"Cancellation Signal To Master thread from Player ::"<<playerName[i];
					cout<<"\nVerification of the Request By Master thread\nThread Cancel Sucessfully\n";
					pthread_cancel(Player[i]);
					b.Player_s[i].Position=count +1;
					count++;
					flag[i]=false;
					c1=false;
					sleep(1);
				sem_post(&winCheck);
			}
		}
		if(count==3)
		break;

	}
	for(int i=0; i<Players; i++)
	{
		if(flag[i]==true)
		{

			cout<<"Cancellation Signal To Master thread from Player ::"<<playerName[i];
			cout<<"\nVerification of the Request By Master thread\nThread Cancel Sucessfully\n";
			b.Player_s[i].Position=count +1;
			pthread_cancel(Player[i]);
		}
	}
	// Distory all the semaphores
	sem_destroy(&Dice_Roll);
	sem_destroy(&Grid_Acess);
	sem_destroy(&RoundTurns);
	sem_destroy(&winCheck);
	CurScreen=4;
	pthread_exit(NULL);
}





void GameDisplay()/**/
{
	// set the background color using function glClearColor.
	// to change the background play with the red, green and blue values below.
	// Note that r, g and b values must be in the range [0,1] where 0 means dim rid and 1 means pure red and so on.

	glClearColor(0, 0,0, .3 ); // Red==Green==Blue===1 ,Alpha=0 --> White Colour
	glClear (GL_COLOR_BUFFER_BIT); //Update the colors
	if(CurScreen==0)
	{
		b.MenuDisplay();
	}
	else if(CurScreen==1)
	{
		b.DrawBoard();
		if(check)
		{
			check=false;
			pthread_create(&Master,NULL,&Master_Thread, NULL);
		}
	}
	else if(CurScreen==3)
	{
		b.DisplayPlayerInfo();
	}
	else if(CurScreen==4)
	{
		b.DrawEndScreen();
	}
	glutSwapBuffers(); // do not modify this line..
	glutPostRedisplay();
}



// seed the random numbers generator by current time (see the documentation of srand for further help)...
/* Function sets canvas size(drawing area) in pixels...
 *  that is what dimensions (x and y) your game will have
 *  Note that the bottom-left coordinate has value (0,0) and top-right coordinate has value (width-1,height-1)
 * */

void SetCanvasSize(int width, int height) {
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width, 0, height, -1, 1); // set the screen size to given width and height.
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity();
}



/*This function is called (automatically) whenever any non-printable key (such as up-arrow, down-arraw)
 * is pressed from the keyboard
 *
 * You will have to add the necessary code here when the arrow keys are pressed or any other key is pressed...
 *
 * This function has three argument variable key contains the ASCII of the key pressed, while x and y tells the
 * program coordinates of mouse pointer when key was pressed.
 *
 * */

void NonPrintableKeys(int key, int x, int y) 
{
	if (key
			== GLUT_KEY_LEFT /*GLUT_KEY_LEFT is constant and contains ASCII for left arrow key*/) {
		// what to do when left key is pressed...

	} else if (key
			== GLUT_KEY_RIGHT /*GLUT_KEY_RIGHT is constant and contains ASCII for right arrow key*/) {

	} else if (key
			== GLUT_KEY_UP/*GLUT_KEY_UP is constant and contains ASCII for up arrow key*/) {

	}

	else if (key
			== GLUT_KEY_DOWN/*GLUT_KEY_DOWN is constant and contains ASCII for down arrow key*/) {

	}

	/* This function calls the Display function to redo the drawing. Whenever you need to redraw just call
	 * this function*/

	glutPostRedisplay();

}

/*This function is called (automatically) whenever any printable key (such as x,b, enter, etc.)
 * is pressed from the keyboard
 * This function has three argument variable key contains the ASCII of the key pressed, while x and y tells the
 * program coordinates of mouse pointer when key was pressed.
 */
void performCheck(int gotiNo)
{
	int index= b.Player_s[playerNo].Tokens[gotiNo].Index;
	if(index==-1 && CurScore==6)
	{
		b.Player_s[playerNo].Tokens[gotiNo].Index=0;
		CurScore=0;
		return;
	}
	if(index==-1 && CurScore!=6)
	cout<<"Invalid Move-> You Cann't move token at Home Without Six"<<endl;

	if(index!=-1)
	{
		if(index+CurScore<51 || b.Player_s[playerNo].HitRates!=0)
		{
			if(index+CurScore<56)
			{
				while(CurScore!=0)
				{
					b.Player_s[playerNo].Tokens[gotiNo].Index++;
					CurScore--;
				}
				
			}
			else if(index+CurScore==56)
			{
				b.Player_s[playerNo].Tokens[gotiNo].Finish=true;
				CurScore=0;
				cout<<"This token Finished, Enter a diffent token"<<endl;
			}
			else
			{
				int count=0; 
				for(int i=0; i<TotalTokens; i++)
				{
					if(b.Player_s[playerNo].Tokens[gotiNo].Index==-1)
					count++;
				}
				if(count==0 )
				{
					CurScore=0;	
				}
			}
		}
		else if(b.Player_s[playerNo].HitRates==0)// This if make the token to complete round if hitrate is zero 
		{
			b.Player_s[playerNo].Tokens[gotiNo].Index=(index+6+CurScore)%RouteSize;
			CurScore=0;
		}
		// cout<<"Don't know why the shit if not runned::"<<index+CurScore<<endl;
		return;
	}
	return;
}

void PrintableKeys(unsigned char key, int x, int y) 
{
	if(CurScreen==1)
	{
		if(playerNo>-1)
		{
			if(key=='0')
			{
				performCheck(0);	
			}
			else if(key=='1' && TotalTokens>=2)
			{
				performCheck(1);
			}
			else if(key=='2' && TotalTokens>=3)
			{
				performCheck(2);	
			}
			else if(key=='3' && TotalTokens>=4)
			{
				performCheck(3);
			}
		}
	}

	if (key == 27) 
	{
		exit(1); // exit the program when escape key is pressed.
	}
	if(CurScreen==0)
	{
		if(key=='S' || key=='s')
		CurScreen=1;
		else if(key=='i' || key=='I')
		CurScreen=3;
	}
	if(CurScreen==2 || CurScreen==3)
	{
		if(key=='b' || key=='B')
		{
			CurScreen=0;
		}
	}
	glutPostRedisplay();
}


/*
 * This function is called after every 1000.0/FPS milliseconds
 * (FPS is defined on in the beginning).
 * You can use this function to animate objects and control the
 * speed of different moving objects by varying the constant FPS.
 *
 * */
void Timer(int m) {

	// implement your functionality here

	// once again we tell the library to call our Timer function after next 1000/FPS
	glutPostRedisplay();
	glutTimerFunc(1000.0, Timer, 0);
}

/*This function is called (automatically) whenever your mouse moves witin inside the game window
 *
 * You will have to add the necessary code here for finding the direction of shooting
 *
 * This function has two arguments: x & y that tells the coordinate of current position of move mouse
 *
 * */
void MousePressedAndMoved(int x, int y) 
{
	// cout << x << " " << y << endl;
	glutSwapBuffers();
	
}
void MouseMoved(int x, int y) 
{
	// cout<<"X,Y::"<<x<<" , "<<y<<endl;
	glutPostRedisplay();
}

/*This function is called (automatically) whenever your mouse button is clicked witin inside the game window
 *
 * You will have to add the necessary code here for shooting, etc.
 *
 * This function has four arguments: button (Left, Middle or Right), state (button is pressed or released),
 * x & y that tells the coordinate of current position of move mouse
 *
 * */
void MouseClicked(int button, int state, int x, int y) 
{
	y=840-(y*825/655);
	if (button == GLUT_LEFT_BUTTON && state==0) // dealing only with left button
	{
	}
	glutPostRedisplay();
}

int main(int argc, char*argv[]) {

	//b = new Board(60, 60); // create a new board object to use in the Display Function ...
	int width = 1020, height = 840; // i have set my window size to be 800 x 600
	sem_init(&Dice_Roll,0, 1);
	sem_init(&Grid_Acess,0,1);
	sem_init(&RoundTurns,0,1);
	sem_init(&winCheck, 0, 1);
	cout<<"\n\n\t\tWELCOME TO C++ LUDO \n\n";
	cout<<"Enter 1st Player Name : ";
	cin>>playerName[0];
	cout<<"Enter 2nd Player Name : ";
	cin>>playerName[1];
	cout<<"Enter 3rd Player Name : ";
	cin>>playerName[2];
	cout<<"Enter 4th Player Name : ";
	cin>>playerName[3];
	cout<<"Enter total Number for Tokens::";
	cin>>TotalTokens;
	while(TotalTokens <1 || TotalTokens>4)
	{
		cout<<"Enter Number of Token in the Range:: ";
		cin>>TotalTokens;
	}

	
	//b->InitalizeBoard(width, height);
	InitRandomizer(); // seed the random number generator...
	glutInit(&argc, argv); // initialize the graphics library...
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // we will be using color display mode
	glutInitWindowPosition(50, 50); // set the initial position of our window
	glutInitWindowSize(width, height); // set the size of our window
	glutCreateWindow("OS Project By Anas & Hunaid"); // set the title of our game window
	SetCanvasSize(width, height); // set the number of pixels...

	// Register your functions to the library,
	// you are telling the library names of function to call for different tasks.
	//glutDisplayFunc(display); // tell library which function to call for drawing Canvas.
	glutMouseFunc(MouseClicked);
	
	glutDisplayFunc(GameDisplay); // tell library which function to call for drawing Canvas.
	glutSpecialFunc(NonPrintableKeys); // tell library which function to call for non-printable ASCII characters
	glutKeyboardFunc(PrintableKeys); // tell library which function to call for printable ASCII characters
	// This function tells the library to call our Timer function after 1000.0/FPS milliseconds...
	glutTimerFunc(1000.0, Timer, 0);

	
	//glutPassiveMotionFunc(MouseMoved); // Mouse
	//glutMotionFunc(MousePressedAndMoved); // Mouse

	// now handle the control to library and it will call our registered functions when
	// it deems necessary...
	glutMainLoop();
	return 1;
}
#endif /* AsteroidS_CPP_ */
