// NewMDP.cpp: 定义控制台应用程序的入口点。

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <time.h>
#include <ctime>
#include <vector>
#include <math.h>
#pragma warning(disable:4996)
using namespace std;
#define HashNum 2
#define HashLength 10000000
#define Min(a,b) a<b?a:b
FILE *fp;

double set_time;
double start;
int **Qadj;//adjacency matrix
int *Gamma;
int *Degree;
int **q;//distance matrix
int diff;//fisrt index
int seed;
int VarNum;//elements num
int M;

int f;//current objective value
int f_best;//best objective value during tabu search
int global_f_best;//best objective value during search
int *best_solution;//best solution during search
double t_best;

int *TabuTable;
int *HashTable;
int *AddSet;
int AddsetSize;
int *DelSet;
int DelSetSize;
int *son1, *son2;
int *parent1, *parent2;
unsigned int *HashW;
unsigned int *THashValue;
unsigned int HashValue[HashNum];
double HashWeightP[HashNum] = { 1.8,1.3 };


void InitialGamma(int *solution)
{
	int diff;
	for (int i = 0; i <VarNum; i++)
		Gamma[i] = 0;

	for (int i = 0; i < VarNum; i++)
	{
		diff = q[i][i];
		for (int j = 0; j < Degree[i]; j++)
		{
			int k = Qadj[i][j];
			if (solution[k] == 1)
				diff = diff + q[i][k];
		}
		if ((1 - solution[i]) == 0)
			diff = 0 - diff;
		Gamma[i] = diff;
	}
}

void InitialTabuTable()
{
	memset(TabuTable, 0, sizeof(int)*VarNum);
}


int InitialObjectValue(int *solution)
{
	f = 0;
	for (int i = 0; i < VarNum; i++)
	{
		if (solution[i] == 1)
		{
			f = f + q[i][i];
			for (int j = 0; j< Degree[i]; j++)
			{
				int k = Qadj[i][j];
				if (solution[k] == 1)
					f = f + q[i][k] / 2;
			}
		}
	}
	return f;
}

int InitialObjectValue2(int *sol)
{
	int sol_value = 0;
	for (int i = 0; i < VarNum; i++)
	{
		if (sol[i] == 1)
		{
			sol_value = sol_value + q[i][i];
			for (int j = 0; j< Degree[i]; j++)
			{
				int k = Qadj[i][j];
				if (sol[k] == 1)
					sol_value = sol_value + q[i][k] / 2;
			}
		}
	}
	return sol_value;
}



void UpdateGamma(int bestflip, int oldvalue, int *solution)
{
	Gamma[bestflip] = 0 - Gamma[bestflip];

	for (int i = 0; i< Degree[bestflip]; i++)
	{
		int k = Qadj[bestflip][i];
		if (oldvalue == solution[k])
			Gamma[k] = Gamma[k] + q[bestflip][k];
		else
			Gamma[k] = Gamma[k] - q[bestflip][k];
	}
}


void InitData(char* filename)
{
	ifstream FIC;
	FIC.open(filename);
	if (FIC.fail())
	{
		cout << "### File Open Error " << filename << endl;
		exit(0);
	}

	FIC >> VarNum >> M;
	//if(FIC.eof())
	//{
	//     cout << "### File Open Error " << fichier << endl;
	//     exit(0);
	//}

	q = new int*[VarNum];
	for (int x = 0; x<VarNum; x++)
		q[x] = new int[VarNum];


	for (int i = 0; i < VarNum; i++)
		for (int j = 0; j< VarNum; j++)
		{
			q[i][j] = 0;
			
		}
	while (!FIC.eof())
	{
		int x1, x2, x3;
		FIC >> x1 >> x2 >> x3;
		x1 = x1 - diff;
		x2 = x2 - diff;
		if ( x1<0 || x2<0 || x1>=VarNum || x2 >=VarNum )
		{
		      cout << "### Read Date Error : line = "<< x1 << ", column = " << x2 << endl;
		      exit(0);
		}
		if (x1 == x2) {
			q[x1][x2] = 0;
		}

		else
		{
			q[x1][x2] = 2 * x3;
			q[x2][x1] = 2 * x3;
		}
	}
	FIC.close();
}

void CheckSolution(int *solution)
{
	int i, c = 0;
	for (i = 0; i < VarNum; i++) {
		if (solution[i] == 1)
			c++;
	}
	if (c != M)
	{
		printf("error solution %d", c);
		system("pause");
	}
	printf("%d", InitialObjectValue(solution));
}

void Construct_Qadj()
{
	for (int i = 0; i < VarNum; i++)
	{
		int k = 0;
		for (int j = 0; j < VarNum; j++)
		{
			if (i != j && q[i][j] != 0)
			{
				Qadj[i][k] = j;
				k++;
			}
		}
		Degree[i] = k;
	}
}



void initialHashValue(int *solution)
{
	int i, j;
	for (i = 0; i < HashNum; i++) {
		HashValue[i] = 0;
	}
	for (j = 0; j < VarNum; j++) {
		for (i = 0; i < HashNum; i++) {
			HashValue[i] += solution[j] * HashW[j*HashNum + i];
		}
	}
	for (i = 0; i < HashNum; i++) {
		//HashTable[(HashValue[i] % HashLength)*HashNum + i] = 1;
		HashTable[(HashValue[i] % HashLength)*HashNum + i] ++;
	}
}


void disturb3(int *solution)
{
	int i;
	int perturb = (int)0.3*M;
	int *finaldelete = new int[perturb];
	int *finaladd = new int[perturb];
	int Hdelete = 0;
	int Hadd = 0;
	int Hone = 0;
	int Hzero = 0;
	for (i = 0; i < VarNum; i++) {
		if (solution[i] == 1) {
			if (rand() % (M - Hone) < perturb - Hdelete) {
				finaldelete[Hdelete++] = i;
			}
			Hone++;
		}
		else {
			if (rand() % (VarNum - M - Hzero) < perturb - Hadd) {
				finaladd[Hadd++] = i;
			}
			Hzero++;
		}
		if (Hdelete == perturb && Hadd == perturb)
			break;
	}
	for (i = 0; i < Hdelete; i++) {
		solution[finaldelete[i]] = 0;
	}
	for (i = 0; i < Hadd; i++) {
		solution[finaladd[i]] = 1;
	}
	delete[] finaldelete;
	delete[] finaladd;
}

void AdjustHeap(int *set, int parenti, int bound)
{
	int temp = set[parenti];
	int child = parenti * 2 + 1;
	while (child < bound) {
		if (child + 1 < bound&&Gamma[set[child]]>Gamma[set[child + 1]]) {
			++child;
		}
		if (Gamma[set[child]] < Gamma[temp]) {
			set[parenti] = set[child];
			parenti = child;
			child = parenti * 2 + 1;
		}
		else break;
	}
	set[parenti] = temp;
}

void BuildHeap(int *set, int bound)
{
	for (int i = (bound >> 1) - 1; i >= 0; i--)
		AdjustHeap(set, i, bound);
}

int HeapSort(int *set, int setSize, int inserti, int bound)
{
	if (setSize < bound - 1) {
		set[setSize] = inserti;
		return 1;
	}
	if (setSize == bound - 1) {
		set[setSize] = inserti;
		BuildHeap(set, bound);
		return 1;
	}
	if (Gamma[inserti]>Gamma[set[0]]) {
		set[0] = inserti;
		AdjustHeap(set, 0, bound);
		return 0;
	}
	return 0;
}

void MyHeapSort(int addbound, int delbound, int *solution)
{
	AddsetSize = 0;
	DelSetSize = 0;
	int i;
	for (i = 0; i < VarNum; i++) {
		if (solution[i] == 0) {
			AddsetSize += HeapSort(AddSet, AddsetSize, i, addbound);
		}
		else DelSetSize += HeapSort(DelSet, DelSetSize, i, delbound);
	}
}

int TabuSearch3(int *Rsolution, int *Isolution)
{
	int iter;
	int indexi, indexj;
	int LocalOptimum = 1;
	int i, j, k;
	int Dsetsetting[3];
	int Asetsetting[3];
	int kk = (int)pow(M, 0.5) + 2;
	int Max_reset = Min(M / 100, 5);
	int reset = 0;
	Asetsetting[0] = Dsetsetting[0] = kk / 2;
	Asetsetting[1] = Dsetsetting[1] = kk;
	Asetsetting[2] = Dsetsetting[2] = Asetsetting[0] * 3;
	int searchLength = 40 * M;
	int TabuL[] = { 15,25,35 };
	int settingIter = 0;
	f_best = InitialObjectValue(Isolution);
	if (f > global_f_best) {
		global_f_best = f;
		for (k = 0; k < VarNum; k++) {
			best_solution[k] = Isolution[k];
		}
		t_best = (clock() - start) / CLOCKS_PER_SEC;
	}
	InitialGamma(Isolution);
	initialHashValue(Isolution);
	InitialTabuTable();
	

	for (i = 0; i < VarNum; i++) {
		Rsolution[i] = Isolution[i];
	}

	int rflag = 0;
	for (iter = 1; ; iter++)
	{
		int num = 0;
		int tabu_num = 0;
		int Wr_num = 0;
		int best_diff = -INT_MAX;//best diff based on aspiration criterion 
		int tabu_best_diff = -INT_MAX;//best diff based on tabu
		int Wr_best_diff = -INT_MAX;//best diff based on solution tabu

		int best_delete = -INT_MAX;
		int best_add = -INT_MAX;
		int tabu_best_delete;
		int tabu_best_add;
		int Wr_best_delete;
		int Wr_best_add;

		int final_delete;
		int final_add;
		int final_diff;
		int oldvalue;
		int Temp_Gamma;
		int stabu;
		int Addflag = 0;
		int Delflag = 0;
		int minVisit = INT_MAX;
		int LocVisit = INT_MAX;

		MyHeapSort(Asetsetting[settingIter], Dsetsetting[settingIter], Isolution);
	
		for (i = 0; i < DelSetSize; i++)
		{
			indexi = DelSet[i];

			for (j = 0; j <AddsetSize; j++) {
				indexj = AddSet[j];

				stabu = 0;
				if (rflag != 0) {
					LocVisit = INT_MAX;
					for (k = 0; k < HashNum; k++) {
						if (HashTable[((HashValue[k] - HashW[indexi*HashNum + k] + HashW[indexj*HashNum + k]) % HashLength)*HashNum + k] <LocVisit)
							LocVisit = HashTable[((HashValue[k] - HashW[indexi*HashNum + k] + HashW[indexj*HashNum + k]) % HashLength)*HashNum + k];
						//break;
					}
					//if (k == HashNum)
					//	stabu = 1;
				}
				Temp_Gamma = Gamma[indexi] + Gamma[indexj] - q[indexi][indexj];
				if (TabuTable[indexi] <= iter
					&& TabuTable[indexj] <= iter && rflag == 0) {
					if (Temp_Gamma > tabu_best_diff)
					{
						tabu_best_diff = Temp_Gamma;
						tabu_best_delete = indexi;
						tabu_best_add = indexj;
						tabu_num = 1;
					}
					else if (Temp_Gamma == tabu_best_diff)
					{
						tabu_num++;
						if (rand() % tabu_num == 1) {
							tabu_best_delete = indexi;
							tabu_best_add = indexj;
						}
					}
				}
				else if (rflag != 0 && LocVisit == 0) {
					if (LocVisit < minVisit) {
						minVisit = LocVisit;
						Wr_best_diff = Temp_Gamma;
						Wr_best_delete = indexi;
						Wr_best_add = indexj;
						Wr_num = 1;
					}
					else if (Temp_Gamma > Wr_best_diff)
					{
						Wr_best_diff = Temp_Gamma;
						Wr_best_delete = indexi;
						Wr_best_add = indexj;
						Wr_num = 1;
					}
					else if (Temp_Gamma == Wr_best_diff)
					{
						Wr_num++;
						if (rand() % Wr_num == 1) {
							Wr_best_delete = indexi;
							Wr_best_add = indexj;
						}
					}
				}
				else if (Temp_Gamma + f > f_best) {
					if (Temp_Gamma> best_diff)
					{
						best_diff = Temp_Gamma;
						best_delete = indexi;
						best_add = indexj;
						num = 1;
					}
					else if (Temp_Gamma == best_diff)
					{
						num++;
						if (rand() % num == 1) {
							best_delete = indexi;
							best_add = indexj;
						}
					}
				}
			}
		}
		if (best_diff > tabu_best_diff&&best_diff > Wr_best_diff)
		{
			final_delete = best_delete;
			final_add = best_add;
			final_diff = best_diff;
		}
		else if (Wr_best_diff>tabu_best_diff) {
			final_delete = Wr_best_delete;
			final_add = Wr_best_add;
			final_diff = Wr_best_diff;
		}
		else {
			final_delete = tabu_best_delete;
			final_add = tabu_best_add;
			final_diff = tabu_best_diff;
		}
		if (rflag != 0)
			rflag = (rflag + 1) % (M / 2);
		if (final_diff != -INT_MAX) {

			oldvalue = Isolution[final_delete];
			Isolution[final_delete] = 1 - Isolution[final_delete];
			UpdateGamma(final_delete, oldvalue, Isolution);
			TabuTable[final_delete] = iter + TabuL[settingIter] + rand() % 11;

			oldvalue = Isolution[final_add];
			Isolution[final_add] = 1 - Isolution[final_add];
			UpdateGamma(final_add, oldvalue, Isolution);
			TabuTable[final_add] = iter + TabuL[settingIter] + rand() % 11;

			f = f + final_diff;

			for (k = 0; k < HashNum; k++) {
				HashValue[k] = HashValue[k] - HashW[final_delete*HashNum + k] + HashW[final_add*HashNum + k];
				//HashTable[(HashValue[k] % HashLength)*HashNum + k] = 1;
				HashTable[(HashValue[k] % HashLength)*HashNum + k] ++;
			}
		}

		if (f > f_best)
		{
			if (f > global_f_best) {
				global_f_best = f;
				for (k = 0; k < VarNum; k++) {
					best_solution[k] = Isolution[k];
				}
				t_best = (clock() - start) / CLOCKS_PER_SEC;
			}
			f_best = f;
			for (k = 0; k < VarNum; k++) {
				Rsolution[k] = Isolution[k];
			}		
			LocalOptimum = 0;
			reset = 0;
			searchLength = 40 * M;
		}
		else
		{
			LocalOptimum++;
			if (LocalOptimum % M == 0) {
				settingIter = (settingIter + 1) % 3;
			}
			if (LocalOptimum == searchLength) {
				settingIter = 0;
				reset++;
				for (k = 0; k < VarNum; k++) {
					Isolution[k] = Rsolution[k];
				}
				rflag = 1;
				InitialObjectValue(Isolution);
				InitialGamma(Isolution);
				searchLength = 6 * M;
				initialHashValue(Isolution);
				InitialTabuTable();
				LocalOptimum = 0;
			}

			if (reset > (M/100))
				break;
		}
	}
	double ntime = (clock() - start) / CLOCKS_PER_SEC;
	//printf("%d %d %lf %lf\n", iter, f_best / 2, t_best, ntime);
	return f_best;
}


void Distribute_Memory_PR()
{
	int i, j;
	Degree = new int[VarNum];

	Gamma = new int[VarNum];

	TabuTable = new int[VarNum];
	for (int x = 0; x<VarNum; x++)
		TabuTable[x] = 0;

	Qadj = new int*[VarNum];
	for (int x = 0; x<VarNum; x++)
		Qadj[x] = new int[VarNum];

	for (i = 0; i < VarNum; i++) {
		for (j = 0; j < VarNum; j++) {
			Qadj[i][j] = -1;
		}
	}
	Construct_Qadj();


	son1 = new int[VarNum];
	son2 = new int[VarNum];
	parent1 = new int[VarNum];
	parent2 = new int[VarNum];
	best_solution = new int[VarNum];


	HashTable = new int[HashLength*HashNum]();
	HashW = new unsigned int[VarNum*HashNum];
	for (int i = 0; i < VarNum; i++) {
		for (int j = 0; j < HashNum; j++) {
			HashW[i*HashNum + j] = (unsigned int)pow(i + 1, HashWeightP[j]);
		}
	}
	AddSet = new int[VarNum];
	DelSet = new int[VarNum];
}

void CopySol(int *Dsol, int *Osol)
{
	for (int i = 0; i < VarNum; i++) {
		Dsol[i] = Osol[i];
	}
}


int selectPath(int *Delta, int Num)
{
	int left = Num / 2;
	int max_diff = -INT_MAX;
	int i;
	int max_num = 1;
	int index;
	for (i = left; i < Num; i++) {
		if (Delta[i] > max_diff) {
			max_diff = Delta[i];
			index = i;
			max_num = 1;
		}
		else if (Delta[i] == max_diff) {
			max_num++;
			if (rand() % max_num == 0) {
				index = i;
			}
		}
	}
	return index;
}

void Construc_Path(int *init_sol, int Num, int *Path, int indexi)
{
	int oldvalue;
	//printf("%d %d\n", Num, indexi);	
	while (--Num > indexi) {
		oldvalue = init_sol[Path[Num * 2]];
		init_sol[Path[Num * 2]] = 1 - init_sol[Path[Num * 2]];
		UpdateGamma(Path[Num * 2], oldvalue, init_sol);

		oldvalue = init_sol[Path[Num * 2 + 1]];
		init_sol[Path[Num * 2 + 1]] = 1 - init_sol[Path[Num * 2 + 1]];
		UpdateGamma(Path[Num * 2 + 1], oldvalue, init_sol);
	}
}

//internal path relinking
void PRN(int *OutS, int Numco, int *Unique1, int *Unique2, int *tag)
{
	int NumQ = M - Numco;
	int *Path = new int[NumQ * 2];
	int *Delta = new int[NumQ]();
	int LimitNum = NumQ * 2 / 3;
	int i, j, k;
	int indexi, indexj;
	int best_diff;
	InitialGamma(OutS);
	int oldvalue;
	int rindex;
	for (k = 0; k < LimitNum; k++) {
		best_diff = -INT_MAX;
		for (i = 0; i < NumQ; i++) {
			indexi = Unique1[i];
			if (tag[indexi] == 1)
				continue;
			for (j = 0; j < NumQ; j++) {
				indexj = Unique2[j];
				if (tag[indexj] == 1)
					continue;
				if (Gamma[indexi] + Gamma[indexj] - q[indexi][indexj] > best_diff) {
					best_diff = Gamma[indexi] + Gamma[indexj] - q[indexi][indexj];
					Path[k * 2] = indexi;
					Path[k * 2 + 1] = indexj;
				}
			}
		}
		tag[Path[k * 2]] = 1;
		tag[Path[k * 2 + 1]] = 1;

		oldvalue = OutS[Path[k * 2]];
		OutS[Path[k * 2]] = 1 - OutS[Path[k * 2]];
		UpdateGamma(Path[k * 2], oldvalue, OutS);

		oldvalue = OutS[Path[k * 2 + 1]];
		OutS[Path[k * 2 + 1]] = 1 - OutS[Path[k * 2 + 1]];
		UpdateGamma(Path[k * 2 + 1], oldvalue, OutS);

		if (k == 0) {
			Delta[k] = best_diff;
		}
		else Delta[k] = Delta[k - 1] + best_diff;
	}
	rindex = selectPath(Delta, LimitNum);
	Construc_Path(OutS, LimitNum, Path, rindex);
	
	delete[] Path;
	delete[] Delta;
}

//external path relinking
void PRW(int *OutS, int NumCo, int *Common, int *Unique1, int *Unique2, int *tag)
{
	int LimitNum = NumCo * 2 / 3;
	int *Path = new int[NumCo * 2];
	int *Delta = new int[NumCo]();
	int best_diff;
	int i, j, k;
	int indexi;
	int oldvalue;
	int rindex;
	for (k = 0; k < LimitNum; k++) {
		best_diff = -INT_MAX;
		for (i = 0; i < NumCo; i++) {
			indexi = Common[i];
			if (tag[indexi] != 1)
				continue;

			for (j = 0; j < VarNum; j++) {
				if (tag[j] != 0)
					continue;

				if (Gamma[indexi] + Gamma[j] - q[indexi][j] > best_diff) {
					best_diff = Gamma[indexi] + Gamma[j] - q[indexi][j];
					Path[k * 2] = indexi;
					Path[k * 2 + 1] = j;
				}
			}
		}
		tag[Path[k * 2]] = 2;
		tag[Path[k * 2 + 1]] = 2;

		oldvalue = OutS[Path[k * 2]];
		OutS[Path[k * 2]] = 1 - OutS[Path[k * 2]];
		UpdateGamma(Path[k * 2], oldvalue, OutS);

		oldvalue = OutS[Path[k * 2 + 1]];
		OutS[Path[k * 2 + 1]] = 1 - OutS[Path[k * 2 + 1]];
		UpdateGamma(Path[k * 2 + 1], oldvalue, OutS);

		if (k == 0) {
			Delta[k] = best_diff;
		}
		else Delta[k] = Delta[k - 1] + best_diff;
	}

	rindex = selectPath(Delta, LimitNum);
	Construc_Path(OutS, LimitNum, Path, rindex);

	delete[] Path;
	delete[] Delta;
}

void PathR(int *parent1, int *parent2, int *son1, int *son2)
{
	int i;
	int similarity = 0;
	int *Unique1 = new int[M];//Unique elements in solution 1
	int *Unique2 = new int[M];//Unique elements in solution 2
	int *Common = new int[M];//Common elements
	int *tag1 = new int[VarNum]();
	int *tag2 = new int[VarNum]();
	int NumU1 = 0;
	int NumU2 = 0;
	int NumCo = 0;//Number of Common elements 
	
	for (i = 0; i < VarNum; i++) {
		if (parent1[i] == parent2[i]) {
			if (parent1[i] == 1) {
				Common[NumCo++] = i;
				tag1[i] = 1;
				tag2[i] = 1;
			}

		}
		else if (parent1[i] == 1) {
			Unique1[NumU1++] = i;
			tag1[i] = 2;
			tag2[i] = 2;
		}
		else if (parent2[i] == 1) {
			Unique2[NumU2++] = i;
			tag1[i] = 2;
			tag2[i] = 2;
		}
	}

	if (NumCo < M * 0.4)
	{
		CopySol(son1, parent1);
		PRN(son1, NumCo, Unique1, Unique2, tag1);
		CopySol(son2, parent2);
		PRN(son2, NumCo, Unique2, Unique1, tag2);
	}
	else if (NumCo<M*0.9) {
		CopySol(son1, parent1);
		PRW(son1, NumCo, Common, Unique1, Unique2, tag1);
		CopySol(son2, parent2);
		PRW(son2, NumCo, Common, Unique2, Unique1, tag2);
	}
	else disturb3(son2);

	delete[]Unique1;
	delete[]Unique2;
	delete[]Common;
	delete[]tag1;
	delete[]tag2;
}

void initial_solution(int *solution)
{
	int NM;
	NM = M;
	int c;
	for (int i = 0; i < VarNum; i++) {
		if ((c = rand() % (VarNum - i)) < NM) {
			solution[i] = 1;
			NM--;
		}
		else solution[i] = 0;
	}

	return;
}

void checkSimilarity(int *sol1, int *sol2)
{
	int same = 0;
	for (int i = 0; i < VarNum; i++) {
		if (sol1[i] && sol1[i] == sol2[i]) {
			same++;
		}
	}
	if (same >= M * 0.9) {
		initial_solution(sol2);
	}
}

//MAE framework
void HY_Procedure()
{
	int cycle = 0;
	int Gen = 0;
	int Iter_Cycle = 5;
	int T_value1;
	int T_value2;
	int Ncycle_value;
	int *Ncycle_solution = new int[VarNum];
	int *LNcycle_solution = new int[VarNum];
	start = clock();
	initial_solution(parent1);
	initial_solution(parent2);
	initial_solution(Ncycle_solution);
	initial_solution(LNcycle_solution);
	Ncycle_value = InitialObjectValue(Ncycle_solution);
	double ntime;
	while (1) {
		ntime = (clock() - start) / CLOCKS_PER_SEC;
		if (ntime > set_time)
			break;
		PathR(parent1, parent2, son1, son2);
		T_value1 = TabuSearch3(parent1, son1);
		T_value2 = TabuSearch3(parent2, son2);
		if (T_value1 > Ncycle_value&& T_value1 >= T_value2) {
			CopySol(Ncycle_solution, parent1);
			Ncycle_value = T_value1;
		}
		else if (T_value2 > Ncycle_value&& T_value2 > T_value1) {
			CopySol(Ncycle_solution, parent2);
			Ncycle_value = T_value1;
		}
		checkSimilarity(parent1, parent2);
		if (Gen % Iter_Cycle == 0) {
			CopySol(parent1, LNcycle_solution);
			CopySol(LNcycle_solution, Ncycle_solution);
			initial_solution(Ncycle_solution);
			Ncycle_value = InitialObjectValue(Ncycle_solution);
			cycle++;
		}
		Gen++;
	}
	delete[] Ncycle_solution;
	delete[] LNcycle_solution;
}

void initial_Setting(char *filename)
{
	global_f_best = 0;
	seed = time(NULL) % 1000000;
	//seed = 10000000;
	srand(seed);
	printf("%d\n", seed);
	InitData(filename);
	Distribute_Memory_PR();
}

int calMDP(char *filename, char *outfilename)
{
	cout << "Name of Input File = " << filename << endl;
	cout << "Name of Output File = " << outfilename << endl;
	printf("************************\n");

	initial_Setting(filename);
	HY_Procedure();

	fp = fopen(outfilename, "a+");
	fprintf(fp, "%d,%d,%lf\n", seed, global_f_best / 2, t_best);
	fprintf(fp, "Best solution:\n");
	for (int i = 0; i < VarNum; i++) {
		if (best_solution[i]) {
			fprintf(fp, "%d ", i+diff);
		}
	}
	fprintf(fp, "\n");
	
	return 1;
}


int main(int argv, char*argc[])
{
	char filename[200] = "small_n5_m3.txt";
	char outfilename[200] = "bigRe.txt";
	set_time = 20;
	diff = 1;
	if (argv == 5) {
		strcpy(filename, argc[1]);
		strcpy(outfilename, argc[2]);
		set_time = atof(argc[3]);
		diff = atoi(argc[4]);
	}
	calMDP(filename, outfilename);
	return 0;
}