#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:6031)

/*
*	File:		  lexScan.c
*	Project:	  Program 1
*	Assignment:	  Programming Project #1
*	Programmer:	  Phillip Benard	
*	Date:		  2/10/2026
*	University:	  McMurry University
*	Course:		  COSC–4310 Spring 2026
*	Instructor:	  Mr. Brozovic
*	Compiler:	  Visual Studio 2022 Community
*	Description:  Lexeme Scanner
*/


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#define MAXINPUTLINE 128
//A 2D Array of keywords to check for...
char keywd_def[32][10] = { {"auto"},{"double"},{"int"},{"struct"},{"break"},{"else"},
	{"long"},{"switch"},{"case"},{"enum"},{"register"},{"typedef"},{"char"},{"extern"},
	{"return"},{"union"},{"const"},{"float"},{"short"},{"unsigned"},{"continue"},{"for"},
	{"signed"},{"void"},{"default"},{"goto"},{"sizeof"},{"volatile"},{"do"},{"if"},{"static"},{"while"}
};
/*A special flag meant to determine whether I'm on a single line or multi-line comment 
if it's multi-line, this will flip on and prevent the program from counting those extra lines
as anything more than a single comment...
*/
int multiline_flag = 0;
//My enumerated types...
typedef enum name_t {
	PPDIR,			/*  Preprocessor Directive */
	IDENT,			/*  Identifier  */
	KEYWD,			/*  Keyword     */
	INTGR,			/*  Integer		*/
	FLOAT,			/*  Float		*/
	STRNG,			/*  String		*/
	CHAR,			/*  Char	    */
	CMMNT,			/*  Comment	    */
	TEOF,			/*  End of File	*/
	NOTOKEN,        /*  Not a Token */
} name_t;
//My token data type...
typedef struct token_t {
	char lexeme[MAXINPUTLINE + 1];
	int rownum;
	int colnum;
	enum name_t name;			/* Token type name */

} token_t;
//Prototypes...

//The Lex Function
token_t lex(FILE *fileptr);
//Check for a Keyword Function...
int check_keywd(char lexeme[]);
//Print out the enumerated type name...
void printname(int num, FILE* fileptr);

int main(int argc, char* argv[])
{
	//Main
	//Initialized my variables...mostly counters and the status/novar for the arguments...and my token...
	int status = 0;
	int novar = 0;
	int ppdir = 0;
	int ident = 0;
	int keywd = 0;
	int intgr = 0;
	int flpt = 0;
	int str = 0;
	int chr = 0;
	int cmnt = 0;

	token_t current_token;

	//Opened my file ptrs...
	FILE* infileptr, * outfileptr;
	infileptr = fopen(argv[1], "r");
	outfileptr = fopen(argv[2], "w");
	//Checked for valid input/output files...
	if (infileptr == NULL) {

		novar = 1;
		printf("ERROR! The input file %s could not be found!", argv[1]);
	}
	if (outfileptr == NULL) {
		novar = 1;
		printf("ERROR! The output file %s could not be found!", argv[2]);
	}
	if (!novar) {
		//Splash Screen...
		printf("     Lexical Scanner\n");
		printf("  COSC-4310 Spring 2026\n");
		printf("    by Phillip Benard\n\n\n");
		printf("Input File: %s\n\n", argv[1]);
		//Create a token...
		current_token = lex(infileptr);
		//While the current_token is not classified as TEOF...
		while (current_token.name != TEOF) {
			//Check if token is valid amidst defined enumerated names...
			if (current_token.name != NOTOKEN) { 
				//If so, print out the token's data to the file for output...
				fprintf(outfileptr,"(%3d,", current_token.rownum);
				fprintf(outfileptr, "%4d)\t", current_token.colnum);
				printname(current_token.name, outfileptr);
				fprintf(outfileptr, "%s", current_token.lexeme);
				fprintf(outfileptr,"\n");
				//Check what kind of token is being output so I can tally the score for the final output...
				if (current_token.name == PPDIR) {
					ppdir++;
				}
				else if (current_token.name == IDENT) {
					ident++;
				}
				else if (current_token.name == KEYWD) {
					keywd++;
				}
				else if (current_token.name == INTGR) {
					intgr++;
				}
				else if (current_token.name == FLOAT) {
					flpt++;
				}
				else if (current_token.name == STRNG) {
					str++;
				}
				else if (current_token.name == CHAR) {
					chr++;
				}
				else if (current_token.name == CMMNT) {
					if (!multiline_flag) { cmnt++;}

				}
			}//END OF VALID TOKEN IF...

			//Run lex so I can get a new token...
			current_token = lex(infileptr);
		}//END OF WHILE LOOP...

		//DISPLAY SUMMARY REPORT TO OUTPUT FILE AND CONSOLE...
		//CONSOLE
		printf("    Scanned %3d lines\n", current_token.rownum);
		printf("%15d PPDIRs\n", ppdir);
		printf("%15d IDENTs\n", ident);
		printf("%15d KEYWDs\n", keywd);
		printf("%15d INTGRs\n", intgr);
		printf("%15d FLOATs\n", flpt);
		printf("%15d STRNGs\n", str);
		printf("%15d CHARs\n", chr);
		printf("%15d CMMNTs\n", cmnt);

		//OUTPUT FILE
		fprintf(outfileptr, "\n\n");
		fprintf(outfileptr, "    Scanned %3d lines\n", current_token.rownum);
		fprintf(outfileptr, "%15d PPDIRs\n", ppdir);
		fprintf(outfileptr, "%15d IDENTs\n", ident);
		fprintf(outfileptr, "%15d KEYWDs\n", keywd);
		fprintf(outfileptr, "%15d INTGRs\n", intgr);
		fprintf(outfileptr, "%15d FLOATs\n", flpt);
		fprintf(outfileptr, "%15d STRNGs\n", str);
		fprintf(outfileptr, "%15d CHARs\n", chr);
		fprintf(outfileptr, "%15d CMMNTs\n", cmnt);

		return status;


	}

	if (argc != 3) {
		printf("\nError: program argument(s) missing!\n");
		status = -1;
	}
}

token_t lex(FILE* fileptr) {
	/*Initialize my variables, some static so I can keep track of my position in the file,
	what the last character I worked on was, whether I'm in the middle of a multi-line comment,
	what my previous character was...etc.
	*/
	int lexpos = 0;
	static int row = 1, col = 1;
	static char charact = ' ';
	static int flag = 1;
	static int multiline = 0;
	static char prev = ' ';
	token_t token;
	token.name = NOTOKEN;
	//Initial grabbing of character, only time I don't increment column since we are starting at position 1...
	if (flag == 1) { 
		token.rownum = row;
		token.colnum = col;
		charact = (char)fgetc(fileptr); 
		flag = 0; }
	/*
	This is a special if condition for when you are returning to a multiline comment that's still in progress, basically
	runs the same code as below but with a few minor changes to account for the fact that this is a multi-line in progress.
	Will turn on the flag that keeps the output from counting the multiline comments as more than one comment and will turn
	itself off once it finishes...
	*/
	else if (multiline) {
		token.rownum = row;
		token.colnum = col;
		token.name = CMMNT;
		multiline_flag = 1;
		while (!(prev == '*' && charact == '/') && charact != '\n') {//Keep scanning comment until closing star/slash or newline is encountered
			prev = charact;
			token.lexeme[lexpos] = charact; lexpos++;
			charact = (char)fgetc(fileptr);
			col++;
			
		}
		/*
		If this multiline ends in a star/slash, set the multi-line flag to off so this code won't be activated again on the next call to lex()
		*/
		if (prev == '*' && charact == '/') {
			token.lexeme[lexpos] = charact; lexpos++;
			charact = (char)fgetc(fileptr);
			col++;
			multiline = 0;
			prev = ' ';
		}
		/*
		If this multiline ends in another newline, then the multiline continues on, we keep the token activated which prevents the comment counter
		from being incremented and come back again until it finds the star/slash...
		*/
		else if (charact == '\n') {
			multiline = 1;
			prev = charact;
			row++;
			col = 0;
			charact = (char)fgetc(fileptr);
			col++;
			//printf("MULTILINE COMMENT FOUND\n");//Debug
		}
		token.lexeme[lexpos] = '\0';
	}
	/*
	If this isn't the first run of lex, nor a returning multi-line comment that we are finishing, then we can move onto the main while loop...
	*/
	else {

		while (token.name == NOTOKEN && !feof(fileptr)) {//KEEP LOOPING UNTIL A VALID TOKEN IS FOUND...or end of file...
			//printf("LEXSCAN BEGINS\n");//Debug Code...
			/*
			Newline code, it increments the row position, resets the column to zero, discards the \n and sets the token's rownumber to
			match the current row...
			*/
			if (charact == '\n') {
				row++;
				//printf("ROW INCREMENTED: %d", row);//Debug Code
				col = 0;
				charact = (char)fgetc(fileptr);
				col++;
				//printf("NEWLINE FOUND\n");//Debug Code
				token.rownum = row;
			} //Newline
			/*
			Small block of code to handle tab character...
			*/
			else if (charact == '\t') {
				charact = (char)fgetc(fileptr);
				col += 4 - ((col - 1) % 4);
			} //Tab

			/*
			Checking for an alphabetical or underscore for an Identifier...
			*/
			else if (isalpha(charact) || charact == '_') { //scan identifier (or reserved word)
				token.rownum = row;
				token.colnum = col;
				token.name = IDENT;
				//printf("IDENT FOUND\n");//Debug
				//Keep scanning lexeme while next character is alpha-numeric or underscore)
				while (isalnum(charact) || charact == '_') {
					//printf("SEARCHING FOR IDENTS\n");//Debug
					token.lexeme[lexpos] = charact; lexpos++;
					charact = (char)fgetc(fileptr);col++;
				}
				token.lexeme[lexpos] = '\0';
				if (check_keywd(token.lexeme)) {
					token.name = KEYWD;
				}
			
			} //Letter

			else if (isdigit(charact)) {//scan numeric literal (integer or float)
				token.rownum = row;
				token.colnum = col;
				//printf("DIGIT FOUND\n");//Debug
				token.name = INTGR;
				//Keep scanning integer literal until a non-digit character is encountered
				while (isdigit(charact)) {
					//printf("SEARCHING FOR DIGITS\n");//Debug
					token.lexeme[lexpos] = charact; lexpos++;
					charact = (char)fgetc(fileptr);
					col++;
				}//INTEGER

				if (charact == '.') {//If character that stops the integer scan is a decimal point
					//printf("DECIMAL PT FOUND\n");//Debug
					token.name = FLOAT;
					token.lexeme[lexpos] = charact; lexpos++;
					charact = (char)fgetc(fileptr); col++;
					while (isdigit(charact)) {//Keep scanning float literal until a non-digit character is encountered
						//printf("SEARCHING FOR FLOATS\n");//Debug
						token.lexeme[lexpos] = charact; lexpos++;
						charact = (char)fgetc(fileptr);
						col++;
					}
				}//FLOAT
				//Note: I set a terminator character here at the end of the completed lexeme, otherwise I get a bunch of messy output from leftover characters...
				token.lexeme[lexpos] = '\0';
			} // Digit
			/*
			Checking for a preprocesor directive, if I find one, I just read the whole line until I hit a newline...
			*/
			else if (charact == '#') {//scan preprocessor directive
				token.rownum = row;
				token.colnum = col;
				//printf("PPDIR FOUND\n");//Debug
				token.name = PPDIR;
				token.lexeme[lexpos] = charact; lexpos++;
				charact = (char)fgetc(fileptr); col++;
				//Keep scanning lexeme until a newline character is encountered
				while (charact != '\n') {
					//printf("SEARCHING FOR PPDIR\n");//Debug
					token.lexeme[lexpos] = charact; lexpos++;
					charact = (char)fgetc(fileptr);
					col++;
				}
				token.lexeme[lexpos] = '\0';
			} // # Pound sign
			/*
			Scanning for a character literal...
			*/
			else if (charact == '\'') {//scan character literal
				token.rownum = row;
				token.colnum = col;
				//printf("SINGLE QUOTATION FOUND\n");//Debug
				token.name = CHAR;
				token.lexeme[lexpos] = charact; lexpos++;
				charact = (char)fgetc(fileptr); col++;
				//Keep scanning lexeme until a closing single quote, not an escape quote, is encountered
				while (charact != '\'') {
					//printf("SEARCHING FOR CHARACTER LITERALS\n");//Debug
					//Checking for tabs in character literals...or else my column values will get all messed up...
					if (charact == '\t'){
						token.lexeme[lexpos] = charact; lexpos++;
						charact = (char)fgetc(fileptr);
						col += 4 - ((col - 1) % 4);
					}
					//If not a tab character, check for an escape character and then pull the next character...
					else {
						if (charact == '\\') {
							//printf("ESCAPE KEY FOUND\n");//Debug
							token.lexeme[lexpos] = charact; lexpos++;
							charact = (char)fgetc(fileptr);
							col++;
						}
						token.lexeme[lexpos] = charact; lexpos++;
						charact = (char)fgetc(fileptr);
						col++;
					}
				}
				token.lexeme[lexpos] = charact; lexpos++;
				charact = (char)fgetc(fileptr); col++;

			token.lexeme[lexpos] = '\0';
			}// ' single quote
			/*
			Scanning for a string literal, basically same thing as above...
			*/
			else if (charact == '\"') {//scan string literal
				token.rownum = row;
				token.colnum = col;
				//printf("DOUBLE QUOTE FOUND\n");//Debug
				token.name = STRNG;
				token.lexeme[lexpos] = charact; lexpos++;
				charact = (char)fgetc(fileptr); col++;
				while (charact != '\"') {//Keep scanning lexeme until a closing double quote, not an escape double quote, is encountered
					//printf("SEARCHING FOR STRING LITERALS\n");//Debug
					//Checking for tabs in string literals...
					if (charact == '\t') {
						token.lexeme[lexpos] = charact; lexpos++;
						charact = (char)fgetc(fileptr);
						col += 4 - ((col - 1) % 4);
					}
					else {
						if (charact == '\\') {
							//printf("ESCAPE KEY FOUND\n");//Debug
							token.lexeme[lexpos] = charact; lexpos++;
							charact = (char)fgetc(fileptr);
							col++;
						}
						token.lexeme[lexpos] = charact; lexpos++;
						charact = (char)fgetc(fileptr);
						col++;
					}
				}
				token.lexeme[lexpos] = charact; lexpos++;
				charact = (char)fgetc(fileptr); col++;
			token.lexeme[lexpos] = '\0';
			} // " double quote
			/*
			Scanning for a comment...
			I've tested it and it seems to handle both // comments and star/slash comments...
			First checks the forward slash but doesn't classify it as a comment right away...
			*/
			else if (charact == '/') {//scan comment? See item in notes list below.
				token.rownum = row;
				token.colnum = col;
				prev = charact; // I store the forwardslash here for now, in case I need it for the comment and grab the next character...
				charact = (char)fgetc(fileptr);
				col++;
				//Confirm that this is a double slash, single line comment...go until a newline...
				if (charact == '/') {//If next character is another forward slash, scan single line comment...
					token.name = CMMNT;
					token.lexeme[lexpos] = prev; lexpos++; //It's a comment, so go ahead and add that forwardslash and the incoming forward slash...
					token.lexeme[lexpos] = charact; lexpos++;
					charact = (char)fgetc(fileptr);
					col++;
					while (charact != '\n') {
						//Checking for tabs in string literals...
						if (charact == '\t') {
							token.lexeme[lexpos] = charact; lexpos++;
							charact = (char)fgetc(fileptr);
							col += 4 - ((col - 1) % 4);
						}
						else {
							token.lexeme[lexpos] = charact; lexpos++;
							charact = (char)fgetc(fileptr);
							col++;
						}
					}
				}
				//Confirm that this is a star/slash style comment, I've got to check for a proper star/slash ending or a \nl, which means multiline!
				else if (charact == '*') {//If next character is a *, scan multi-line comment
					if (multiline_flag) { multiline_flag = 0; } //Turn off flag so it can begin marking down comments again...
					token.name = CMMNT;
					token.lexeme[lexpos] = prev; lexpos++; //It's a comment, so go ahead and add that forwardslash and the incoming star...
					token.lexeme[lexpos] = charact; lexpos++;
					charact = (char)fgetc(fileptr);
					col++;
					/*
					I use the initialized prev variable and the charact variable to determine whether I have the winning combo of star and slash
					simultaneously to end the line...If I do, I've got my lexeme...
					
					If I hit a newline, that will also break the loop, but will lead to a different codeblock to handle it...
					
					*/
					while (!(prev == '*' && charact == '/') && charact != '\n') {//Keep scanning comment until closing star/slash or newline is encountered
						//Checking for tabs in comments...
						if (charact == '\t') {
							token.lexeme[lexpos] = charact; lexpos++;
							charact = (char)fgetc(fileptr);
							col += 4 - ((col - 1) % 4);
						}
						else {
							prev = charact;
							token.lexeme[lexpos] = charact; lexpos++;
							charact = (char)fgetc(fileptr);
							col++;
						}
						
					}
					//If we have the star/slash, then this is a single line comment and we can end it here...
					if (prev == '*' && charact == '/') {
						token.lexeme[lexpos] = charact; lexpos++;
						charact = (char)fgetc(fileptr);
						col++;
						multiline = 0;
						prev = ' ';
					}
					/*
					If it's a newline, then that means this is a multiline comment, I set a flag and the next time I call lex, it will go
					to a special code section defined above for dealing with multiline comments in a way that doesn't increment comments...
					*/
					else if (charact == '\n') {
						multiline = 1;
						prev = charact;
						row++;
						col = 0;
						charact = (char)fgetc(fileptr);
						col++;
						//printf("MULTILINE COMMENT FOUND\n");//Debug
					}

				}
				token.lexeme[lexpos] = '\0';
				//Wasn't a comment, so that forward slash I saved ends up being irrelevant, so it doesn't appear later on like with sqrt...
			}//  / scan comment? (NOTE: I need to start with single line //, then move up to multiline

			/*
			Code meant to deal with random characters that don't fall into any of the above characters,
			I just discard them and increment the column...
			*/
			else {
				//printf("RANDOM JUNK: %c\n", charact);//Debug
				charact = (char)fgetc(fileptr);
				col++;
			} //ANY OTHER CHARACTER...
			//END OF IF STRUCTURE...

		}//END OF WHILE LOOP...

	}

	if (feof(fileptr)) {
		token.name = TEOF;
	}
	//printf("RETURNING TOKEN\n");//Debug
	return token;



}
//Terminate Lexeme so I can use char search index of to find nl, place incoming character there, and then increment pos...strchr
//When it says reset posiition, it means columns...

int check_keywd(char lexeme[]) {
	int result = 0;
	//printf("%s\n", lexeme);//Debug
	for (int i = 0; i < sizeof(keywd_def) / sizeof(keywd_def[0]); i++) {
		//printf("%s\n", lexeme);//Debug
		//printf("%s\n", keywd_def[i]);//Debug
		if (!strcmp(lexeme, keywd_def[i])) {
			result = 1;
			//printf("MATCH FOUND\n");//Debug
		}
	}
	return result;
}

void printname(int num, FILE* fileptr) {
	//2D Array of names 
	char names[8][10] = { {"PPDIR"},{"IDENT"},{"KEYWD"},{"INTGR"},{"FLOAT"},{"STRNG"},{"CHAR"},{"CMMNT"}};
	//I'm printing out the corresponding name associated with the desired number...
	fprintf(fileptr, "%-5s\t", names[num]);

}


