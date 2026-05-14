#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:6031)

/*
*	File:		  array_slicer.c
*	Project:	  Program 2
*	Assignment:	  Programming Project #2
*	Programmer:	  Phillip Benard	
*	Date:		  3/30/2026
*	University:	  McMurry University
*	Course:		  COSC–4310 Spring 2026
*	Instructor:	  Mr. Brozovic
*	Compiler:	  Visual Studio 2022 Community
*	Description:  Array Slicer
*/


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <assert.h>
//My function prototypes for declaring, deleting and filling an array...
double*** dec_array(int layer_low, int layer_hi, int row_low, int row_hi, int col_low, int col_hi);
void delete_array(double*** cube, int layer_low, int layer_hi, int row_low, int row_hi);
void fill_array(FILE* fileptr, double*** cube, int layer_low, int layer_hi, int row_low, int row_hi, int col_low, int col_hi);

int main(int argc, char* argv[])
{
	//Main
	//Opened my file ptrs and intialize variables...
	FILE* infileptr, * outfileptr;
	infileptr = fopen(argv[1], "r");
	outfileptr = fopen(argv[2], "w");
	int status = 0;
	int testnum = 0;
	int slicenum = 0;
	int cells = 0;
	double slicesum = 0;
	//Dimensions
	int layers_low = 0;
	int layers_hi = 0;
	int rows_low = 0;
	int rows_hi = 0;
	int cols_low = 0;
	int cols_hi = 0;
	//Slices
	int l_slice_low = 0;
	int l_slice_hi = 0;
	int r_slice_low = 0;
	int r_slice_hi = 0;
	int c_slice_low = 0;
	int c_slice_hi = 0;
	//Checking to make sure file opens correctly...
	if (!infileptr) {

		printf("ERROR! The input file %s could not be found!", argv[1]);
	}
	if (!outfileptr) {

		printf("ERROR! The output file %s could not be found!", argv[2]);
	}
	if (infileptr && outfileptr) {
		//Scan number of test cases incoming...
		fscanf(infileptr, "%d", &testnum);
		//Run a for loop based on number of incoming test cases...
		for (int t = 0; t < testnum; t++) {
			//Scan in the layers, rows and cols specifications...
			fscanf(infileptr, "%d:%d,%d:%d,%d:%d", &layers_low, &layers_hi, &rows_low, &rows_hi, &cols_low, &cols_hi);
			//Pass specifications to a function to manually allocate the 3D cube array
			double*** cube_array = dec_array(layers_low, layers_hi, rows_low, rows_hi, cols_low, cols_hi);
			//Pass the specifications and my new array to a function to fill the array with values...
			fill_array(infileptr, cube_array, layers_low, layers_hi, rows_low, rows_hi, cols_low, cols_hi);
			//printf("\n\n");//Debug code for checking if array is filling correctly...
			//Scan in the number of slices coming in from the data file...
			fscanf(infileptr, "%d", &slicenum);
			for (int i = 0; i < slicenum; i++) {
				//Scan in the slice value ranges...
				fscanf(infileptr, "%d:%d,%d:%d,%d:%d", &l_slice_low, &l_slice_hi, &r_slice_low, &r_slice_hi, &c_slice_low, &c_slice_hi);
				//Compute the total cells in a slice...
				cells = (1+l_slice_hi - l_slice_low) * (1+r_slice_hi - r_slice_low) * (1+c_slice_hi - c_slice_low);
				//Compute the sum of those slices via 3-layer nested for loop...
				for (int j = l_slice_low; j <= l_slice_hi; j++) {//Number of layers

					for (int k = r_slice_low; k <= r_slice_hi; k++) { //Number of rows

						for (int g = c_slice_low; g <= c_slice_hi; g++) {//Number of cols
							slicesum += cube_array[j-layers_low][k-rows_low][g-cols_low];
						}
					}
				}
				//Output the total cells and their sum to the data file, then reset slicesum to 0 for next loop...
				fprintf(outfileptr, "%d:%.2lf\n", cells, slicesum);
				slicesum = 0.0;
			}
			//Output divider for the next test case, unallocate array for next test case dimensions via a function...
			fprintf(outfileptr, "===============\n"); //15 equal signs after each test case...
			delete_array(cube_array, layers_low, layers_hi, rows_low, rows_hi);
		}
		//Close file pointers before ending program...
		fclose(infileptr);
		fclose(outfileptr);
	}
	//Put up an error if the file cannot be opened...
	if (argc != 3) {
		printf("\nError: program argument(s) missing!\n");
		status = -1;
	}

}



//Function to declare an array using the parameters passed to it by the file scanner...
double*** dec_array(int layer_low, int layer_hi, int row_low, int row_hi, int col_low, int col_hi) {
	//Declare my layers, rows, and cols variable, and my cube array...
	int nbr_layers = 1 + layer_hi - layer_low;
	int nbr_rows = 1 + row_hi - row_low;
	int nbr_cols = 1 + col_hi - col_low;
	double*** cube;
	//Begin allocating cube array using assertions and nested for loop...
	cube = (double***)malloc(nbr_layers * sizeof(double**));
	assert(cube != NULL);
	for (int d = layer_low; d <= layer_hi; d++) {
		cube[d - layer_low] = (double**)malloc(nbr_rows * sizeof(double*));
		assert(cube[d-layer_low] != NULL);
		for (int r = row_low; r <= row_hi; r++) {
			cube[d-layer_low][r-row_low] = (double*)malloc(nbr_cols * sizeof(double));
			assert(cube[d-layer_low][r - row_low] != NULL);
		}
	}
	//Return the allocated cube to main...
	return cube;

}
//A function to delete the cube array between tests, basically the above function but in reverse...using a nested for loop to free it piece by piece...
void delete_array(double*** cube,int layer_low, int layer_hi, int row_low, int row_hi) {

	assert(cube != NULL);
	for (int d = layer_low; d <= layer_hi; d++) {

		for (int r = row_low; r <= row_hi; r++) {
			free(cube[d-layer_low][r-row_low]);
		}
		free(cube[d-layer_low]);
	}
	free(cube);

}
//Function that uses nested for loops to scan data from the file and place it into the cube array allocated above...
void fill_array(FILE* fileptr, double*** cube, int layer_low, int layer_hi, int row_low, int row_hi, int col_low, int col_hi) {

	for (int d = layer_low; d <= layer_hi; d++) {



		for (int r = row_low; r <= row_hi; r++) {


			for (int c = col_low; c <= col_hi; c++) {

				fscanf(fileptr, "%lf", &cube[d-layer_low][r-row_low][c-col_low]);
				fgetc(fileptr);
				//printf("%lf\n", cube[d - layer_low][r - row_low][c - col_low]);//Debug Code, check if array fills properly...

			}


		}


	}


}