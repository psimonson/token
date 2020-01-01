/**
 * @file token.c
 * @author Philip R. Simonson
 * @date 12/26/2019
 * @brief Process C source file into identifiers and preprocessor directives.
 *****************************************************************************
 * @details Just a simple C source file processing program, to display some
 * information about your C sources. Now there are a few bugs when I added
 * more code.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "file.h"

/* some enumerations */
enum { UNCOMMENT, IDENT, PREPROC, COMMENT, STRING, ESCAPES };

/* static variables to hold token and preprocessor info */
static char token[128];
static char preproc[128];
static int pos;

/* Gets tokens from C source file.
 */
long gettoken(file_t *f)
{
	extern char preproc[];
	extern char token[];
	extern int pos;
	static char comment = 0;
	int c;

	c = getc_file(f);
	if(c == '\n' || c == '\r')
		return c;

	if(comment) { /* ignore characters - comment */
		if(c == '*') {
			c = getc_file(f);
			if(c == '/') {
				comment = 0;
				return UNCOMMENT;
			} else {
				ungetc_file(f, c);
			}
		}
	} else if(!isalpha(c) && c == '#') { /* preprocessor directive */
		c = getc_file(f);
		if(isalpha(c)) {
			pos = 0;
			ungetc_file(f, c);
			while(c != ' ' && isalpha(c = getc_file(f)))
				preproc[pos++] = c;
			preproc[pos] = '\0';
			return PREPROC;
		} else {
			ungetc_file(f, c);
		}
	} else if(!comment && c == '/') { /* beginning comment */
		c = getc_file(f);
		if(c == '*') {
			comment = 1;
			return COMMENT;
		} else {
			ungetc_file(f, c);
		}
	} else if(!isalpha(c) && c == '\\') { /* escape characters */
		c = getc_file(f);
		if(c == '\'' || c == '\"')
			return ESCAPES;
	} else if(!isalpha(c) && c == '\"') { /* string */
		c = getc_file(f);
		while(c == '\"' && isprint(c = getc_file(f)));
		ungetc_file(f, c);
		return STRING;
	} else if(isalpha(c)) { /* identifier */
		pos = 0;
		ungetc_file(f, c);
		while(isalnum(c = getc_file(f)))
			token[pos++] = c;
		token[pos] = '\0';
		return IDENT;
	}
	return c; /* code */
}
/* Processes a file using gettokens function.
 */
void process(file_t *f)
{
	const char *pp_keywords[] = {"pragma", "include", "define",
				     "ifdef", "if", "else", "endif",
				     "ifndef", "elif", "undef", "error",
				     "warning", NULL};
	const char *keywords[] = {"int", "void", "char", "long", "short",
				  "float", "double", "return", "unsigned",
				  "signed", "const", "goto", "break", "if",
				  "switch", "continue", "else", "while",
				  "for", "case", "auto", "register", "static",
				  "enum", "struct", "typedef", "union",
				  "do", NULL};
	const char **keyword;
	int t, nl, ni, nc, np, nk, ns, nuncomm, ncomm;
	nl = ni = nc = np = nk = ns = nuncomm = ncomm = 0;
	while((t = gettoken(f)) != EOF) {
		switch(t) {
		case '\n':
			nl++;
#ifndef NDEBUG
			putchar(t);
#endif
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
		case ESCAPES:
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
		case UNCOMMENT:
			nuncomm++;
		break;
		default:
			nc++;
#ifndef NDEBUG
			putchar(t);
#endif
		break;
		}
	}
#ifndef NDEBUG
	putchar('\n');
#endif
	printf("Last identifier/keyword: %s\n", token);
	printf("Last preprocessor directive: %s\n", preproc);
	printf("Total code lines: %d/%d\n"
		"Total code chars: %d\n"
		"Total identifiers: %d\n"
		"Total keywords: %d\n"
		"Total strings: %d\n"
		"Total preprocessors: %d\n"
		"Total comments: %d/%d\n",
		nl-ncomm, nl, nc, (nk > 0 ? ni-nk : ni),
		nk, ns, np, nuncomm, ncomm);
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
