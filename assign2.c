////////////////////////////////////////////////////////////////////////
// Solution to assignment #2 for ECE1733.
////////////////////////////////////////////////////////////////////////

/**********************************************************************/
/*** HEADER FILES *****************************************************/
/**********************************************************************/

typedef int bool;
#define false 0
#define true 1

#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <conio.h>
#include <stdio.h>
#include <assert.h>
#include <malloc.h>
#include <string.h>
#include "common_types.h"
#include "blif_common.h"
#include "cubical_function_representation.h"
#include <process.h>
#include <Windows.h>

/**********************************************************************/
/*** DATA STRUCTURES DECLARATIONS *************************************/
/**********************************************************************/
typedef struct _TRow {
    int var;
    int low;
    int high;
	int treeNum;

    bool swapped; 
} TRow;
/**********************************************************************/
/*** DEFINE STATEMENTS ************************************************/
/**********************************************************************/
#define INIT_SIZE 1024
/**********************************************************************/
/*** GLOBAL VARIABLES *************************************************/
/**********************************************************************/
int curNodeNum, preNodeNum;
int printed[100]={0};
int totalNumInputs = 0;
int numInputs = 0;
int oriNodeNum=0;
int curTree;
TRow *T;
TRow *newT;
int map[100][100]={-1};
t_blif_cubical_function *curFunc;
int nodeNum = 0;
int maxNodeIdx=-1;
int *outRoot;
 //   int *printed;
/**********************************************************************/
/*** FUNCTION DECLARATIONS ********************************************/
/**********************************************************************/

/**********************************************************************/
/*** BODY *************************************************************/
/**********************************************************************/
void printSubGraph(t_blif_signal **inputs, int u)
{

    int nameIdx = T[u].var;
	int childnameIdx;
	if (printed[u]== 1) return; 
        printed[u] = 1;

    if( (T[u].low == 0)||(T[u].low == 1) ) 
    {
        printf("\t%d [%s] -0-> %d \n", u,inputs[nameIdx]->data.name, T[u].low);
		
       
    }
    else
    {
        
		if( printed[T[u].low]==0)
;
            printSubGraph(inputs, T[u].low);
		
		
		childnameIdx=T[T[u].low].var;
        
		printf("\t%d [%s] -0-> %d [%s];\n", u,inputs[nameIdx]->data.name, T[u].low,inputs[childnameIdx]->data.name );
       
    }

    //--------------------------
    
    if( (T[u].high == 0)||(T[u].high == 1) )
    {
         printf("\t%d [%s] -1-> %d \n", u,inputs[nameIdx]->data.name, T[u].high);
        
    }
    else
    {
      
        if( printed[T[u].high] == 0)
            printSubGraph(inputs, T[u].high);

			childnameIdx=T[T[u].high].var;
     
			printf("\t%d [%s] -1-> %d [%s];\n", u,inputs[nameIdx]->data.name, T[u].high,inputs[childnameIdx]->data.name );
       
       
    }

    
   
}

void drawGraph(t_blif_signal **inputs, int rootNode, int fidx)
{
 
	
    memset(printed, 0, sizeof(printed));
	
	//printf("there are %d inputs and %d nodes in total\n", numInputs, curNodeNum);

    printf("digraph %d {\n", fidx);
  
   
    
    printSubGraph(inputs, rootNode);


    printf("}\n");
  
}

void DestroyT()
{
    free(T);
}

void initT()
{
    
    T = (TRow*) malloc(INIT_SIZE * sizeof(TRow));
    
    T[0].var = totalNumInputs; 
    T[0].low = -1;
    T[0].high = -1;
	T[0].treeNum=0;
    T[1].var = totalNumInputs+1; 
    T[1].low = -1;
    T[1].high = -1;
	T[1].treeNum=0;
    nodeNum = 2;
	if (nodeNum>maxNodeIdx )
		maxNodeIdx=nodeNum;
}
/**********************************************************************/
/*** MAIN FUNCTION ****************************************************/
/**********************************************************************/





int cal_cubes_result(t_blif_cube **cubes, int num_input,int num_cubes){
	int cubes_or_result=0;
	int i,j,var;

	for (i = 0; i<num_cubes ; i++){
		int cube_and_result=1;
		for (j = 0; j < num_input; j++){
			
			
			

			var = read_cube_variable(&cubes[i]->signal_status, j);
			if (var == LITERAL_DC){
				continue;
			}
			else
			if (var == LITERAL_0){
				cube_and_result = 0;
				break;
			}
			else if (var == LITERAL_1){
				cube_and_result &= 1;
			}
			
		}
		cubes_or_result = cube_and_result | cubes_or_result;
		if (cubes_or_result == 1)
			return 1;
	}
	return cubes_or_result;
}


t_blif_cube **cubes_copy(t_blif_cube** Src_cubes, int cube_count){
	int i;
	t_blif_cube** cubes = (t_blif_cube**)malloc(cube_count*sizeof(t_blif_cube*));

	for (i = 0; i <cube_count; i++){
		cubes[i] = (t_blif_cube*)malloc(sizeof(t_blif_cube));
		memcpy(cubes[i], Src_cubes[i], sizeof(t_blif_cube));
	}
	return cubes;
}

void updateCube(t_blif_cube **cubes, int i, int num_cubes, int val){
	int j;
	for ( j = 0; j < num_cubes; j++){
		

		int tmp = read_cube_variable(&cubes[j]->signal_status, i);
		
		if (val == LITERAL_1){
			if (tmp == LITERAL_0){
				write_cube_variable(&cubes[j]->signal_status, i, LITERAL_0);
			}
			else if (tmp == LITERAL_1){
				write_cube_variable(&cubes[j]->signal_status, i, LITERAL_1);
			}
		}
		if (val == LITERAL_0) {
			if (tmp == LITERAL_0){
				write_cube_variable(&cubes[j]->signal_status, i, LITERAL_1);
			}
			else if (tmp == LITERAL_1){
				write_cube_variable(&cubes[j]->signal_status, i, LITERAL_0);
			}
		}

		
	}
}

void removeNode(int father, int i){
	int low, high,k;
	low=T[i].low;
	high=T[i].high;
	T[i].var=-1;
	T[i].low=-1;
	T[i].high=-1;
	for (k=0; k<maxNodeIdx; k++)
	  if (T[k].var>=0){
	  
				if (T[k].high==i) T[k].high=high;
				if (T[k].low==i) T[k].low=low;
			}
	
	nodeNum--;
	//	printf("delete fahter %d, i %d, numnode %d\n", father, i, nodeNum);
	

}

void clearRedundant(int father,int i){
	  int low, high, j,k, maxnodenum;
	 
	  if ((i==0)||(i==1))return;
	  
	low=T[i].low;
	high=T[i].high;
	
	//printf("i:%d l:%d, h %d\n",nodeNum, low, high);
	if (low < 0  || high < 0 || low>maxNodeIdx || high > maxNodeIdx) return;
	if (father>0){
	  if ((low == high) && (low >= 0)){
		  removeNode(father, i);
		  clearRedundant(father,low);
	      
		 return;
	  }
	 
	}

	 clearRedundant(i,low);
	 clearRedundant(i,high);
	  
	
}


void deleteSame(){
	  int i,low, high, j,k, maxnodenum;


 for (i=2; i< maxNodeIdx; i++){
      for (j = i+1; j < maxNodeIdx; j++) 
	  if ((i!=j) && (T[j].var>=0) && (T[i].var>=0)){
		  if ((T[i].var == T[j].var) && (T[i].low == T[j].low) && (T[i].high == T[j].high)) {
			T[i].var=-1;
			T[i].low=-1;
			T[i].high=-1;
			nodeNum--;
			for (k=0; k<maxNodeIdx; k++)
			  if (T[k].var>=0){
				if (T[k].high==i) T[k].high=j;
				if (T[k].low==i) T[k].low=j;
			}
		//	printf("delete fahter %d, i %d,j %d, numnode %d\n", k, i,j, nodeNum);
	      
		  
		  }
	    } 
  }

}

int add(int i, int l, int h)
{
    int idx = maxNodeIdx;
    nodeNum++;
	maxNodeIdx++;
	if (nodeNum>maxNodeIdx )
		maxNodeIdx=nodeNum;

    T[idx].var = i;
    T[idx].low = l;
    T[idx].high = h;
	T[idx].treeNum=curTree;
    T[idx].swapped = false;
    return idx;
}


//---------------------------------------------------------------------

//---------------------------------------------------------------------
int find(int i, int l, int h)
{
   
    int idx=-1;
    for(idx=0; (idx < maxNodeIdx); idx++)
    {
        if( (T[idx].var == i) && (T[idx].low == l) && (T[idx].high == h) )
        {
            return idx;
        }
    }
   return -1; 
}

int MK(int i, int l, int h)
{
	int j;
    if(l==h)
        return l;
	j=find(i, l, h);
	if (j>=0) return j;
    else
    {
        int u = add(i, l, h);
        return u;
    }
}
//---------------------------------------------------------------------

//---------------------------------------------------------------------
int build(t_blif_cube **setOfCubes, int cubeCount, int i)
{
 
	int v1, v0;
	int varIdx;
	t_blif_cube **cubes_zero, **cubes_one;
    if(i == numInputs)
    {
       if (cal_cubes_result(setOfCubes, totalNumInputs, cubeCount)){
			return 1;
		}
		else{
			return 0;
		}
    }
    else
    {
       cubes_zero = cubes_copy(setOfCubes, cubeCount);
        
       updateCube(cubes_zero, i, cubeCount, LITERAL_0);

        v0 = build(cubes_zero, cubeCount, i+1);

        

	  cubes_one = cubes_copy(setOfCubes, cubeCount);
        
       updateCube(cubes_one, i, cubeCount, LITERAL_1);


       
        v1 = build(cubes_one, cubeCount, i+1);



        varIdx = curFunc->inputs[i]->data.index;
      //  printf("varIdx = %d\n", varIdx);
        return MK(varIdx, v0, v1);
    }
}



int perfOp(int op, int u1, int u2)
{
	int u=-1;
    switch (op)
		{
		case 1:
			u = u1 & u2;
			break;
		case 2:
			u = u1 | u2;
			break;
		case 3:
			u = u1 ^ u2;
			break;
		case 4:
			u = !(u1 ^ u2);
			break;
		default:
			printf("Unknown OP\n");
			break;
		}
	return u;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int doApply(int op, int u1, int u2)
{
    int u, left, right, low1,low2, high1, high2;

    if(map[u1][u2] != -1)
		return map[u1][u2];
    else if ( ((u1 == 0)||(u1 == 1)) && ((u2 == 0)||(u2 == 1)) )
        return perfOp(op, u1, u2);

	else{

		low1=T[u1].low;
		low2=T[u2].low;
		high1=T[u1].high;
		high2=T[u2].high;
		if (T[u1].var == T[u2].var){
			left=doApply(op, low1, low2);
			right=doApply(op, high1, high2);
			u = MK(T[u1].var, left , right);
		}
		else if (T[u1].var > T[u2].var){

			left=doApply(op, u1, low2);
			right=doApply(op, u1, high2);
			u = MK(T[u2].var, left , right);
        
		}
		else if (T[u1].var < T[u2].var){
			left=doApply(op, low1, u2);
			right=doApply(op, high1, u2);
			u = MK(T[u1].var, left , right);
       
		}

	}
    map[u1][u2] = u;

    return u;
}


void readOP(t_blif_logic_circuit *circuit)
{
    char applyCmd[100];
	char response[100];
    int op = -1;
	int applyRoot;
	int duration;
   LARGE_INTEGER frequency;
			LARGE_INTEGER start;
			LARGE_INTEGER end;

	printf("The BLIF has 2 functions. Do you want to run Apply algorithm to combie it? [y/n]\n");
	
	scanf("%s", response);
	
	
	if (strcmp(response, "y") == 0){
			
		while (1){
			 printf("Enter Apply OP (i.e. and, or, xor, xnor) or \"exit\":\n");
      
			scanf("%s", applyCmd);
			//printf("ur : %s\n", applyCmd);
    
		    if(strcmp("exit", applyCmd)==0)
			   break;
		
			if (strcmp(applyCmd, "or") == 0){
				op = 2;
			}
			else if (strcmp(applyCmd, "and") == 0){
				op = 1;
			}
			else if (strcmp(applyCmd, "xor") == 0){
				op = 3;
			}
			else if (strcmp(applyCmd, "xnor") == 0){
				op = 4;
			}
			else{
				printf("invalid OP\n");
				continue;
			}
 
      //  clearG();
		memset(map, -1, sizeof(map));
		
			QueryPerformanceFrequency(&frequency);
			QueryPerformanceCounter(&start);
		
        applyRoot = doApply(op, outRoot[0], outRoot[1]);
		curNodeNum=nodeNum;
       
        printf("applyRoot=%d\n", applyRoot);
        QueryPerformanceCounter(&end);
			duration = (int)(((double)(end.QuadPart - start.QuadPart) / frequency.QuadPart) * 1000000);
			printf("Runtime for apply alogrithm in our program : %d us\n", duration);
       // curNodeNum=nodeNum-preNodeNum;
		curTree++;
        drawGraph(circuit->primary_inputs, applyRoot, curTree);
		}
    }
}

int numValidNode(TRow *T)
{
    int i;
    int tmp = 2;
    for (i = 2; i < maxNodeIdx; i++) {
        if (T[i].var >= 0 && T[i].var < numInputs && T[i].low >= 0 && T[i].low < maxNodeIdx
            && T[i].high >= 0 && T[i].high < maxNodeIdx) tmp++;
    }
    return tmp;
}

int swap( int var1, int var2, TRow *T) {
	int i, j, flag, tmpl, tmph;
	int table[4] = {-1, -1, -1, -1};;
    for (i = 0; i < maxNodeIdx; i++) {
        T[i].swapped = false;
    }
  
    for (i = 0; i < maxNodeIdx; i++) {
        if (T[i].var == var1 && !T[i].swapped && T[i].low >= 0 && T[i].low < maxNodeIdx && T[i].high >= 0 && T[i].high < maxNodeIdx) {

			 T[i].swapped = true;
			if ((T[T[i].low].var == var2)||(T[T[i].high].var == var2)){
				for (j=2; j<maxNodeIdx; j++)
				  if (T[j].var!=var1){
					 
					  if (T[T[j].high].var==var2) {
						 // printf("high father %d", j);
						  T[j].high=add(var2, T[T[j].high].low,T[T[j].high].high );
					  }
					  if (T[T[j].low].var==var2) { 
						  //printf("low father %d", j );
						  T[j].low=add(var2, T[T[j].low].low,T[T[j].low].high );
					  }
				  }
			}

           

			 T[i].var = var2;
           
			if ( (T[T[i].low].var == var2) && (T[T[i].high].var == var2)){
				//printf("2you %d\n", i);
				T[T[i].high].var = var1;
		        T[T[i].high].low = T[T[i].low].high;
		        T[T[i].high].swapped = true;
				T[T[i].low].var = var1;
		        T[T[i].low].high = T[T[i].high].low;
		        T[T[i].low].swapped = true;
			
			}
			else if (  !(T[T[i].low].var == var2)  && (T[T[i].high].var == var2)){
				T[i].low = add(var1, T[i].low, T[T[i].high].low);
                T[T[i].low].swapped = true;
				T[T[i].high].var = var1;
				T[T[i].high].low = T[i].low;
				T[T[i].high].swapped = true;
			
			}
			else if (  (T[T[i].low].var == var2) && !(T[T[i].high].var == var2) ){
				
				T[i].high = add(var1, T[T[i].low].high, T[i].high);
                T[T[i].high].swapped = true;
				T[T[i].low].var = var1;
		        T[T[i].low].high = T[i].high;
		        T[T[i].low].swapped = true;
			}
			else
			if ( !(T[T[i].low].var == var2) && !(T[T[i].high].var == var2)  ){
				//printf("2no %d\n", i);
				
				 T[i].low = add(var1, T[i].low, T[i].high);
                 T[T[i].low].swapped = true;
				 T[i].high = add(var1, T[i].low, T[i].high);
                 T[T[i].high].swapped = true;
			}
          
        }
    }

    

    return numValidNode(T);
}


void doSift( t_blif_logic_circuit *circuit, int i, int j){
	int tmpTRows;
	int newNumTRows;
	 newNumTRows = nodeNum;
	newT = (TRow *) malloc (INIT_SIZE * sizeof(TRow));
    memcpy(newT, T, nodeNum * sizeof(TRow));
	
	
	tmpTRows = swap(i, j, T);

	//drawGraph(circuit->primary_inputs, outRoot[curTree] , curTree);

	deleteSame();
	
	clearRedundant(-1, outRoot[curTree]);

	printf("There are %d nodes in this modified BDD, here is the graph:\n",tmpTRows  );
	drawGraph(circuit->primary_inputs, outRoot[curTree] , curTree);


	 nodeNum = newNumTRows;
        memcpy(T, newT, nodeNum * sizeof(TRow));

}


void askSift(t_blif_logic_circuit *circuit){
	
	char response[100];
	int Cmd1, Cmd2;
	int duration;
   LARGE_INTEGER frequency;
			LARGE_INTEGER start;
			LARGE_INTEGER end;

	printf("Do you want to run sift algorithm? [y/n]\n");
	
	scanf("%s", response);
	
	
	if (strcmp(response, "y") == 0){
		while (1){

			 printf("Enter 2 indexs <from 0 to %d>  or <-1 -1> to exit\n", numInputs-1);
      
			scanf("%d %d/n", &Cmd1, &Cmd2);
			if (Cmd1==-1 && Cmd2==-1) break;
			if (Cmd1>( numInputs-1)) continue;
			if (Cmd2>( numInputs-1)) continue;
			QueryPerformanceFrequency(&frequency);
			QueryPerformanceCounter(&start);
			doSift(circuit, Cmd1, Cmd2);
			QueryPerformanceCounter(&end);
			duration = (int)(((double)(end.QuadPart - start.QuadPart) / frequency.QuadPart) * 1000000);
			printf("Runtime for sift alogrithm in our program : %d us\n", duration);

		}

	
	}
			


}

int main(int argc, char* argv[])
{
	int i;
	
	t_blif_logic_circuit *circuit = NULL;

	if (argc != 2)
	{
		printf("Usage: %s <source BLIF file>\r\n", argv[0]);
		return 0;
	}

	/* Read BLIF circuit. */
	printf("Reading file %s...\n",argv[1]);
	circuit = ReadBLIFCircuit(argv[1]);
	

	if (circuit != NULL)
	{
		int index;
		
		

		totalNumInputs = circuit->primary_input_count;
		outRoot = (int *) malloc(totalNumInputs * sizeof(int));
        initT();
	
		

		for (index = 0; index < circuit->function_count; index++)
		{
			t_blif_cubical_function *function = circuit->list_of_functions[index];
			curFunc = function;
			curTree=index;
			numInputs = function->input_count;
			

			preNodeNum=nodeNum;
			outRoot[index] = build(function->set_of_cubes, function->cube_count, 0);

			oriNodeNum=nodeNum;
			deleteSame();
			clearRedundant(-1,outRoot[index]);
		
			curNodeNum=nodeNum-preNodeNum;
			printf("outroot %d\n", outRoot[index]);
			drawGraph(circuit->primary_inputs, outRoot[index] , index);
			
			if (index==0) askSift(circuit);
			

		}

		
		if (circuit->function_count>1) readOP(circuit);

	
		printf("Done.\r\n");
		DestroyT();
		DeleteBLIFCircuit(blif_circuit);
	}
	else
	{
		printf("Error reading BLIF file. Terminating.\n");
	}
	return 0;
}

