#include <string.h>
#include "harness/unity.h"
#include "../src/lab.h"

void setUp(void) {
    setenv("MY_PROMPT", "foo>", 1);
}
void tearDown(void) {
    unsetenv("MY_PROMPT");
}
void test_cmd_parse2(void)
{
//The string we want to parse from the user.
//foo -v
char *stng = (char*)malloc(sizeof(char)*7);
strcpy(stng, "foo -v");
char **actual = cmd_parse(stng);
//construct our expected output
size_t n = sizeof(char*) * 6;
char **expected = (char**) malloc(sizeof(char*) *6);
memset(expected,0,n);
expected[0] = (char*)malloc(sizeof(char)*4);
expected[1] = (char*)malloc(sizeof(char)*3);
expected[2] = (char*)NULL;
strcpy(expected[0], "foo");
strcpy(expected[1], "-v");
TEST_ASSERT_EQUAL_STRING(expected[0],actual[0]);
TEST_ASSERT_EQUAL_STRING(expected[1],actual[1]);
TEST_ASSERT_FALSE(actual[2]);
free(expected[0]);
free(expected[1]);
free(expected);
cmd_free(actual);
free(stng);
}
void test_cmd_parse(void)
{
char **rval = cmd_parse("ls -a -l");
TEST_ASSERT_TRUE(rval);
TEST_ASSERT_EQUAL_STRING("ls", rval[0]);
TEST_ASSERT_EQUAL_STRING("-a", rval[1]);
TEST_ASSERT_EQUAL_STRING("-l", rval[2]);
TEST_ASSERT_EQUAL_STRING(NULL, rval[3]);
TEST_ASSERT_FALSE(rval[3]);
cmd_free(rval);
}
void test_trim_white_no_whitespace(void)
{
char *line = (char*) calloc(10, sizeof(char));
strncpy(line, "ls -a", 10);
char *rval = trim_white(line);
TEST_ASSERT_EQUAL_STRING("ls -a", rval);
free(line);
}
void test_trim_white_start_whitespace(void)
{
char *line = (char*) calloc(10, sizeof(char));
strncpy(line, " ls -a", 10);
char *rval = trim_white(line);
TEST_ASSERT_EQUAL_STRING("ls -a", rval);
free(line);
}
void test_trim_white_end_whitespace(void)
{
char *line = (char*) calloc(10, sizeof(char));
strncpy(line, "ls -a ", 10);
char *rval = trim_white(line);
TEST_ASSERT_EQUAL_STRING("ls -a", rval);
free(line);
}
void test_trim_white_both_whitespace_single(void)
{
char *line = (char*) calloc(10, sizeof(char));
strncpy(line, " ls -a ", 10);
char *rval = trim_white(line);
TEST_ASSERT_EQUAL_STRING("ls -a", rval);
free(line);
}
void test_trim_white_both_whitespace_double(void)
{
char *line = (char*) calloc(10, sizeof(char));
strncpy(line, " ls -a ", 10);
char *rval = trim_white(line);
TEST_ASSERT_EQUAL_STRING("ls -a", rval);
free(line);
}
void test_trim_white_all_whitespace(void)
{
char *line = (char*) calloc(10, sizeof(char));
strncpy(line, " ", 10);
char *rval = trim_white(line);
TEST_ASSERT_EQUAL_STRING("", rval);
free(line);
}
void test_trim_white_mostly_whitespace(void)
{
char *line = (char*) calloc(10, sizeof(char));
strncpy(line, " a ", 10);
char *rval = trim_white(line);
TEST_ASSERT_EQUAL_STRING("a", rval);
free(line);
}
void test_get_prompt_default(void)
{
unsetenv("MY_PROMPT");
char *prompt = get_prompt("MY_PROMPT");
TEST_ASSERT_EQUAL_STRING(prompt, "shell>");
// free(prompt);
}
void test_get_prompt_custom(void)
{
const char* prmpt = "MY_PROMPT";
if(setenv(prmpt,"foo>",true)){
TEST_FAIL();
}
char *prompt = get_prompt(prmpt);
TEST_ASSERT_EQUAL_STRING(prompt, "foo>");
free(prompt);
unsetenv(prmpt);
}
void test_ch_dir_home(void)
{
char *line = (char*) calloc(10, sizeof(char));
strncpy(line, "cd", 10);
char **cmd = cmd_parse(line);
char *expected = getenv("HOME");
change_dir(cmd);
char *actual = getcwd(NULL,0);
TEST_ASSERT_EQUAL_STRING(expected, actual);
free(line);
free(actual);
cmd_free(cmd);
}
void test_ch_dir_root(void)
{
char *line = (char*) calloc(10, sizeof(char));
strncpy(line, "cd /", 10);
char **cmd = cmd_parse(line);
change_dir(cmd);
char *actual = getcwd(NULL,0);
TEST_ASSERT_EQUAL_STRING("/", actual);
free(line);
free(actual);
cmd_free(cmd);
}

//This tests to ensure the cmd_parse function can handle en empty input
void test_cmd_parse_empty(void)
{
    char **rval = cmd_parse("");
    TEST_ASSERT_NULL(rval);
    cmd_free(rval);
}

// This checks if the command correctly parses a single arg
void test_cmd_parse_single(void)
{
    char **rval = cmd_parse("ls");
    TEST_ASSERT_EQUAL_STRING("ls", rval[0]);
    TEST_ASSERT_NULL(rval[1]);
    cmd_free(rval);
}

// This checks if the command can correctly parse multiple args
void test_cmd_parse_multiple(void)
{
    char **rval = cmd_parse("ls -l -a -h");
    TEST_ASSERT_EQUAL_STRING("ls", rval[0]);
    TEST_ASSERT_EQUAL_STRING("-l", rval[1]);
    TEST_ASSERT_EQUAL_STRING("-a", rval[2]);
    TEST_ASSERT_EQUAL_STRING("-h", rval[3]);
    TEST_ASSERT_NULL(rval[4]);
    cmd_free(rval);
}

//This checks that trim_white works when there's only whitespace in the string
void test_trim_white_only_whitespace(void)
{
    char *line = (char*)calloc(10, sizeof(char));
    strncpy(line, "     ", 10);
    char *rval = trim_white(line);
    TEST_ASSERT_EQUAL_STRING("", rval);
    free(line);
}

//This tests that when the env variable is not set the default prompt is used
void test_get_prompt_default_empty_env(void)
{
    unsetenv("MY_PROMPT");
    char *prompt = get_prompt("MY_PROMPT");
    TEST_ASSERT_EQUAL_STRING(prompt, "shell>");
    free(prompt);
}

// This makes sure custom prompts work
void test_get_prompt_custom_env(void)
{
    const char* prmpt = "MY_PROMPT";
    setenv(prmpt, "foo>", 1);
    char *prompt = get_prompt(prmpt);
    TEST_ASSERT_EQUAL_STRING(prompt, "foo>");
    free(prompt);
    unsetenv(prmpt);
}

//This checks to see if change_dir handles an invalid dir
void test_ch_dir_invalid(void)
{
    char *line = (char*)calloc(30, sizeof(char));
    strncpy(line, "cd /cheeseAndCrackers", 30);
    char **cmd = cmd_parse(line);
    int result = change_dir(cmd);
    TEST_ASSERT_EQUAL_INT(-1, result);  
    free(line);
    cmd_free(cmd);
}

// Testing cd with no arg
void test_ch_dir_empty(void)
{
    char *line = (char*)calloc(10, sizeof(char));
    strncpy(line, "cd", 10); 
    char **cmd = cmd_parse(line);
    char *expected = getenv("HOME");
    int result = change_dir(cmd);
    TEST_ASSERT_EQUAL_INT(0, result);  
    char *actual = getcwd(NULL, 0);
    TEST_ASSERT_EQUAL_STRING(expected, actual);
    free(line);
    free(actual);
    cmd_free(cmd);
}



int main(void) {
UNITY_BEGIN();
RUN_TEST(test_cmd_parse);
RUN_TEST(test_cmd_parse2);
RUN_TEST(test_trim_white_no_whitespace);
RUN_TEST(test_trim_white_start_whitespace);
RUN_TEST(test_trim_white_end_whitespace);
RUN_TEST(test_trim_white_both_whitespace_single);
RUN_TEST(test_trim_white_both_whitespace_double);
RUN_TEST(test_trim_white_all_whitespace);
RUN_TEST(test_get_prompt_default);
RUN_TEST(test_get_prompt_custom);
RUN_TEST(test_ch_dir_home);
RUN_TEST(test_ch_dir_root);
RUN_TEST(test_cmd_parse_empty);
RUN_TEST(test_cmd_parse_single);
RUN_TEST(test_cmd_parse_multiple);
RUN_TEST(test_trim_white_only_whitespace);
RUN_TEST(test_get_prompt_default_empty_env);
RUN_TEST(test_get_prompt_custom_env);
RUN_TEST(test_ch_dir_invalid);
RUN_TEST(test_ch_dir_empty);
return UNITY_END();
}
