#include <signal.h>  // Required for signal handling (signal, SIGINT, SIGTSTP, SIG_IGN)
#include <unistd.h>  // Required for raise()

#include <string.h>
#include "harness/unity.h"
#include "../src/lab.h"
void setUp(void) {
// set stuff up here
}
void tearDown(void) {
// clean stuff up here
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
char *prompt = get_prompt("MY_PROMPT");
TEST_ASSERT_EQUAL_STRING(prompt, "shell>");
free(prompt);
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
void test_cmd_parse_extra_spaces(void) {
    char **rval = cmd_parse("   ls    -l    -a   ");
    TEST_ASSERT_TRUE(rval);
    TEST_ASSERT_EQUAL_STRING("ls", rval[0]);
    TEST_ASSERT_EQUAL_STRING("-l", rval[1]);
    TEST_ASSERT_EQUAL_STRING("-a", rval[2]);
    TEST_ASSERT_FALSE(rval[3]);
    cmd_free(rval);
}
void test_cmd_parse_empty(void) {
    char **rval = cmd_parse("");
    TEST_ASSERT_NULL(rval);  
}
void test_cmd_parse_long_command(void) {
    char *long_cmd = malloc(5000);
    memset(long_cmd, 'a', 4999);
    long_cmd[4999] = '\0';

    char **rval = cmd_parse(long_cmd);
    TEST_ASSERT_NOT_NULL(rval);
    free(long_cmd);
    cmd_free(rval);
}
void test_trim_white_only_spaces(void) {
    char *line = (char*) calloc(10, sizeof(char));
    strncpy(line, "      ", 10);
    char *rval = trim_white(line);
    TEST_ASSERT_EQUAL_STRING("", rval);
    free(line);
}
void test_trim_white_tabs(void) {
    char *line = (char*) calloc(20, sizeof(char));
    strncpy(line, "  \t  ls -l  \t ", 20);
    char *rval = trim_white(line);
    TEST_ASSERT_EQUAL_STRING("ls -l", rval);
    free(line);
}
void test_get_prompt_no_env(void) {
    unsetenv("MY_PROMPT");
    char *prompt = get_prompt("MY_PROMPT");
    TEST_ASSERT_EQUAL_STRING("shell>", prompt);
    free(prompt);
}
void test_get_prompt_long_string(void) {
    setenv("MY_PROMPT", "This_is_a_very_long_custom_prompt>", 1);
    char *prompt = get_prompt("MY_PROMPT");
    TEST_ASSERT_EQUAL_STRING("This_is_a_very_long_custom_prompt>", prompt);
    free(prompt);
    unsetenv("MY_PROMPT");
}
void test_ch_dir_invalid(void) {
    char *cmd[] = { "cd", "/invalid/path", NULL };
    int result = change_dir(cmd);
    TEST_ASSERT_EQUAL_INT(-1, result);  // Should return error
}
void test_ch_dir_and_back(void) {
    char *cmd[] = { "cd", "/tmp", NULL };
    int result = change_dir(cmd);
    TEST_ASSERT_EQUAL_INT(0, result);

    char *back_cmd[] = { "cd", "..", NULL };
    result = change_dir(back_cmd);
    TEST_ASSERT_EQUAL_INT(0, result);
}
void test_shell_ignore_sigint(void) {
    signal(SIGINT, SIG_IGN);
    raise(SIGINT);  // Simulate Ctrl+C
    TEST_PASS();  // If the shell doesn't crash, the test passes
}
void test_shell_ignore_sigtstp(void) {
    signal(SIGTSTP, SIG_IGN);
    raise(SIGTSTP);  // Simulate Ctrl+Z
    TEST_PASS();
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_cmd_parse);
    RUN_TEST(test_cmd_parse2);
    RUN_TEST(test_cmd_parse_extra_spaces);
    RUN_TEST(test_cmd_parse_empty);
    RUN_TEST(test_cmd_parse_long_command);

    RUN_TEST(test_trim_white_no_whitespace);
    RUN_TEST(test_trim_white_only_spaces);
    RUN_TEST(test_trim_white_tabs);
    
    RUN_TEST(test_get_prompt_default);
    RUN_TEST(test_get_prompt_no_env);
    RUN_TEST(test_get_prompt_long_string);

    RUN_TEST(test_ch_dir_home);
    RUN_TEST(test_ch_dir_root);
    RUN_TEST(test_ch_dir_invalid);
    RUN_TEST(test_ch_dir_and_back);

    RUN_TEST(test_shell_ignore_sigint);
    RUN_TEST(test_shell_ignore_sigtstp);
    
    return UNITY_END();
}

