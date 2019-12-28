#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "file.h"

/* some enumerations */
enum { IDENT, PREPROC, COMMENT };

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
	if(c == '#') {
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
	} else if(c == '/') {
		c = getc_file(f);
		if(c != '*')
			ungetc_file(f, c);
		else {
			while(isprint(getc_file(f)));
			return COMMENT;
		}
	} else if(c == '*') {
		c = getc_file(f);
		if(c != '/')
			ungetc_file(f, c);
	} else if(isalpha(c)) {
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
				  "for", "case", "auto", "register", NULL};
	const char **keyword;
	int t, l, ni, nc, np, nk, ncomm;
	l = ni = nc = np = nk = ncomm = 0;
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
		default:
			nc++;
		break;
		}
	}
	printf("Last identifier/keyword: %s\n", token);
	printf("Last preprocessor directive: %s\n", preproc);
	printf("Total lines: %d\n"
		"Total chars: %d\n"
		"Total identifiers: %d\n"
		"Total keywords: %d\n"
		"Total preprocessors: %d\n"
		"Total comments: %d\n",
		l, nc, ni, nk, np, ncomm);
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
