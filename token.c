/**
 * @file token.c
 * @author Philip R. Simonson
 * @date 12/26/2019
 * @brief Process C source file into identifiers and preprocessor directives.
 *****************************************************************************
 * @details Just a simple C source file processing program, to display some
 * information about your C sources. No bugs yet, that I've found anyways.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "file.h"

/* some enumerations */
enum { IDENT, PREPROC, COMMENT, STRING };

/* static variables to hold token and preprocessor info */
static char token[128];
static char preproc[128];
static int pos;

/* Gets tokens from C source file.
 */
int gettoken(file_t *f)
{
	extern char preproc[];
	extern char token[];
	extern int pos;
	int c;

	c = getc_file(f);
	if(c == '\n' || c == '\r')
		return c;
	if(c == '#') { /* preprocessor directive */
		c = getc_file(f);
		if(isalpha(c)) {
			ungetc_file(f, c);
			pos = 0;
			while(isalnum(c = getc_file(f)))
				preproc[pos++] = c;
			preproc[pos] = '\0';
			return PREPROC;
		} else {
			ungetc_file(f, c);
		}
	} else if(c == '/') { /* beginning comment */
		c = getc_file(f);
		if(c != '*')
			ungetc_file(f, c);
		else
			return COMMENT;
	} else if(c == '*') { /* ending comment */
		c = getc_file(f);
		if(c != '/')
			ungetc_file(f, c);
	} else if(c == '\\') { /* escape characters */
		c = getc_file(f);
		switch(c) {
		case '\"':
			c = getc_file(f);
		break;
		default:
			ungetc_file(f, c);
		break;
		}
	} else if(c == '\"') { /* string */
		c = getc_file(f);
		if(c == '\"')
			ungetc_file(f, c);
		else
			return STRING;
	} else if(isalpha(c)) { /* identifier */
		ungetc_file(f, c);
		pos = 0;
		while(isalnum(c = getc_file(f)))
			token[pos++] = c;
		token[pos] = '\0';
		return IDENT;
	}
	return c;
}
/* Processes a file using gettokens function.
 */
void process(file_t *f)
{
	const char *pp_keywords[] = {"pragma", "include", "define",
				     "ifdef", "if", "else", "endif",
				     NULL};
	const char *keywords[] = {"int", "void", "char", "long", "short",
				  "float", "double", "return", "unsigned",
				  "signed", "const", "goto", "break", "if",
				  "switch", "continue", "else", "while",
				  "for", "case", "auto", "register", "static",
				  NULL};
	const char **keyword;
	int t, l, ni, nc, np, nk, ns, ncomm;
	l = ni = nc = np = nk = ns = ncomm = 0;
	while((t = gettoken(f)) != EOF) {
		switch(t) {
		case '\n':
			l++;
		break;
		case IDENT:
			for(keyword = &keywords[0];
				*keyword != NULL;
				keyword++) {
				if(strcmp(*keyword, token) == 0) {
					nk++;
					break;
				}
			}
			ni++;
		break;
		case COMMENT:
			ncomm++;
		break;
		case PREPROC:
			for(keyword = &pp_keywords[0];
				*keyword != NULL;
				keyword++) {
				if(strcmp(*keyword, preproc) == 0) {
					np++;
					break;
				}
			}
		break;
		case STRING:
			ns++;
		break;
		default:
			nc++;
		break;
		}
	}
	printf("Last identifier/keyword: %s\n", token);
	printf("Last preprocessor directive: %s\n", preproc);
	printf("Total code lines: %d\n"
		"Total code chars: %d\n"
		"Total identifiers: %d\n"
		"Total keywords: %d\n"
		"Total strings: %d\n"
		"Total preprocessors: %d\n"
		"Total comments: %d\n",
		l, nc, (nk > 0 ? ni-nk : ni), nk, ns, np, ncomm);
}
/* Program for testing tokenising functions.
 */
int main()
{
	file_t *f;
	f = open_file("token.c", "rt");
	if(get_error_file() != FILE_ERROR_OKAY)
		return 1;
	process(f);
	close_file(f);
	putchar('\n');
	return 0;
}
