/**
 * @file token.c
 * @author Philip R. Simonson
 * @date 12/28/2019
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

#define MAXTOKEN 128

/* some enumerations */
enum { UNCOMMENT, IDENT, PREPROC, COMMENT, STRING, ESCAPES, UNKNOWN };

/* static variables to hold token and preprocessor info */
static char token[MAXTOKEN];
static char preproc[MAXTOKEN];

/* Check token to see if it's an escape character.
 */
int isescape(int c)
{
	if(c == '\'' || c == '\"' || c == 'n' || c == 'r' || c == '\\'
		|| c == 't' || c == 'v' || c == 'f')
		return 1;
	return 0;
}
/* Gets tokens from C source file.
 */
int gettoken(file_t *f)
{
	extern char preproc[];
	extern char token[];
	static char comment = 0;
	int c, i;

	while((c = getc_file(f)) == ' ' || c == '\t');
	if(c == '#') { /* pre-processor directives */
		for(i = 0; isalpha(c = getc_file(f)); i++)
			preproc[i] = c;
		preproc[i] = '\0';
		return PREPROC;
	} else if(c == '\\') { /* escape sequences */
		c = getc_file(f);
		if(isescape(c))
			return ESCAPES;
		else
			ungetc_file(f, c);
	} else if(c == '/') { /* comment */
		c = getc_file(f);
		if(c == '*') {
			comment = 1;
			return COMMENT;
		} else {
			ungetc_file(f, c);
		}
	} else if(comment && c == '*') { /* inside comment */
		c = getc_file(f);
		if(c == '/') {
			comment = 0;
			return UNCOMMENT;
		} else {
			ungetc_file(f, c);
		}
	} else if(c == '\"') { /* string */
		c = getc_file(f);
		if(c != '\"')
			return STRING;
		else
			ungetc_file(f, c);
	} else if(c == '_' || isalpha(c)) { /* identifier */
		ungetc_file(f, c);
		for(i = 0; isalnum(c = getc_file(f)) || c == '_'; i++)
			token[i] = c;
		token[i] = '\0';
		return IDENT;
	}
	return c; /* other token */
}
/* Processes a file using gettokens function.
 */
void process(file_t *f)
{
#define NDEBUG 0 /* 0 = show tokens/processors, 1 = show other tokens */
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
				  "do", "extern", "volatile", "default",
				  "sizeof", NULL};
	const char **keyword;
	int t, nl, ni, nc, np, ne, nk, ns, nuncomm, ncomm;
	nl = ni = nc = np = nk = ne =  ns = nuncomm = ncomm = 0;
	while((t = gettoken(f)) != EOF) {
		switch(t) {
		case '\n':
			nl++;
#if NDEBUG
			putchar(t);
#endif
		break;
		case IDENT:
			for(keyword = &keywords[0];
				*keyword != NULL;
				keyword++) {
				if(strcmp(*keyword, token) == 0) {
					nk++;
#if !NDEBUG
					printf("Token: %s\n", token);
#endif
					break;
				}
			}
			ni++;
		break;
		case COMMENT:
			ncomm++;
		break;
		case ESCAPES:
			ne++;
		break;
		case PREPROC:
			for(keyword = &pp_keywords[0];
				*keyword != NULL;
				keyword++) {
				if(strcmp(*keyword, preproc) == 0) {
					np++;
#if !NDEBUG
					printf("Preproc: %s\n", preproc);
#endif
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
#if NDEBUG
			putchar(t);
#endif
		break;
		}
	}
#if NDEBUG
	putchar('\n');
#endif
	printf("Last identifier/keyword: %s\n", token);
	printf("Last preprocessor directive: %s\n", preproc);
	printf("Total code lines: %d\n"
		"Total code chars: %d\n"
		"Total identifiers: %d\n"
		"Total keywords: %d\n"
		"Total strings: %d\n"
		"Total escape chars: %d\n"
		"Total preprocessors: %d\n"
		"Total comments: %d/%d\n",
		nl, nc, (nk > 0 ? ni-nk : ni),
		nk, ns, ne, np, nuncomm, ncomm);
#undef NDEBUG
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
	printf("Last build: [%s:%s]\n", __DATE__, __TIME__);
	return 0;
}
