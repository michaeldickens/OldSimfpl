/*
 *  interpreter.h
 *  Calculator
 *
 *  Created by Michael Dickens on 5/8/10.
 *
 */


#include "eval.h"

#define CHARTYPE_ALPHA 0
#define CHARTYPE_NUMBER 1
#define CHARTYPE_SYMBOL 2
#define CHARTYPE_SPACE 3
#define CHARTYPE_BRACKET 4
#define CHARTYPE_OTHER 5

#define WORD_LENGTH 64

// If the last statement was an if statement that was TRUE, this is TRUE. Otherwise, 
// it is FALSE. This is so, when reading a file, it can decide whether to evaluate 
// an else statement.
int previous_truth_value;
int previous_was_if_statement;

int assume_first_is_function;

// Call this function once before using the interpreter.
int init_interpreter();

int run_interpreter();

int stream_paren_balance, stream_bracket_balance, stream_curly_balance, stream_quote_balance, stream_regex_balance;

/* 
 * See value.h for a declaration of interpret_values(), values_to_statement(), 
 * line_enqueue_front(), line_enqueue(), line_dequeue(), get_next_word().
 */

value interpret_given_statement(value *variables, char *statement);
value compile_statement(char *statement);
int get_line(char *str, int length, FILE *stream);
int fix_up_line(char *str, int is_first);
char * get_statement();
value get_values();
void statement_to_words(char *words[], size_t wordcount, char *statement);
value statement_to_values(char *statement);

/* 
 * Compiles a list of words into an S-expression.
 */
value compile_values(value words[], size_t wordcount);
value prefix_words_to_sexp(value words[], size_t *i, size_t length);

/* 
 * Interprets a list of words. Calls compile_values(), then optimizations, then 
 * eval().
 */
value interpret_values(value *variables, value words[], size_t wordcount);
char *values_to_statement(value values[], size_t length);
int words_to_values(value values[], char *words[], size_t wordcount);
int infix_p(value words[], size_t wordcount);
int infix_to_prefix(value outwords[], value inwords[], size_t length);
void var_to_shell(value *body, value name, value shell);
size_t count_words(char *statement);

/* Deletes words (e.g. in parentheses), puts (res) in the proper index, 
 * and returns the new wordcount.
 */
size_t delete_from_words(value words[], size_t wordcount, value res, size_t i, size_t inx);

/* 
 * Reads through (statement) until the end  of the word is reached. 
 * Returns the string pointing to just after the end of the word.
 */ 
char * end_of_word(char *statement);
char * skip_whitespace(char *statement);
int is_same_type(char c, char orig);
int char_type(char c);

#define STATEMENT_INCOMPLETE_CURLY 3
#define STATEMENT_INCOMPLETE_QUOTE 2
#define STATEMENT_COMPLETE         1
#define STATEMENT_INCOMPLETE_OTHER 0
int is_statement_complete(char *statement);

