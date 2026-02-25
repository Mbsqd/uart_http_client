#include <string.h>
#include "unity.h"
#include "parser.h"

// TEST: strip_line_ending
TEST_CASE("strip_line_ending: removes \\r\\n", "[parser]")
{
    char input[] = "http GET http://httpbin.org/get\r\n";
    strip_line_ending(input);
    TEST_ASSERT_EQUAL_STRING("http GET http://httpbin.org/get", input);
}

TEST_CASE("strip_line_ending: removes only \\n", "[parser]")
{
    char input[] = "http GET http://httpbin.org/get\n";
    strip_line_ending(input);
    TEST_ASSERT_EQUAL_STRING("http GET http://httpbin.org/get", input);
}

TEST_CASE("strip_line_ending: no line ending, string unchanged", "[parser]")
{
    char input[] = "http GET http://httpbin.org/get";
    strip_line_ending(input);
    TEST_ASSERT_EQUAL_STRING("http GET http://httpbin.org/get", input);
}

TEST_CASE("strip_line_ending: empty string", "[parser]")
{
    char input[] = "";
    strip_line_ending(input);
    TEST_ASSERT_EQUAL_STRING("", input);
}

// TEST: parse_command - success cases
TEST_CASE("parse_command: GET returns CMD_OK", "[parser]")
{
    char input[] = "http GET http://httpbin.org/get";
    parsed_cmd_t cmd;
    cmd_parse_result_t result = parse_command(input, &cmd);

    TEST_ASSERT_EQUAL(CMD_OK, result);
    TEST_ASSERT_EQUAL(HTTP_GET, cmd.method);
    TEST_ASSERT_EQUAL_STRING("http://httpbin.org/get", cmd.url);
    TEST_ASSERT_NULL(cmd.body);
}

TEST_CASE("parse_command: POST with body returns CMD_OK", "[parser]")
{
    char input[] = "http POST http://httpbin.org/post my_body";
    parsed_cmd_t cmd;
    cmd_parse_result_t result = parse_command(input, &cmd);

    TEST_ASSERT_EQUAL(CMD_OK, result);
    TEST_ASSERT_EQUAL(HTTP_POST, cmd.method);
    TEST_ASSERT_EQUAL_STRING("http://httpbin.org/post", cmd.url);
    TEST_ASSERT_EQUAL_STRING("my_body", cmd.body);
}

TEST_CASE("parse_command: POST body with divider", "[parser]")
{
    char input[] = "http POST http://httpbin.org/post key=value&key2=value2";
    parsed_cmd_t cmd;
    cmd_parse_result_t result = parse_command(input, &cmd);

    TEST_ASSERT_EQUAL(CMD_OK, result);
    TEST_ASSERT_EQUAL_STRING("key=value&key2=value2", cmd.body);
}

TEST_CASE("parse_command: POST without body, body is NULL", "[parser]")
{
    char input[] = "http POST http://httpbin.org/post";
    parsed_cmd_t cmd;
    cmd_parse_result_t result = parse_command(input, &cmd);

    TEST_ASSERT_EQUAL(CMD_OK, result);
    TEST_ASSERT_EQUAL(HTTP_POST, cmd.method);
    TEST_ASSERT_NULL(cmd.body);
}

// parse_command — error cases
TEST_CASE("parse_command: unknown command returns CMD_ERR_UNKNOWN", "[parser]")
{
    char input[] = "test GET http://httpbin.org/get";
    parsed_cmd_t cmd;
    cmd_parse_result_t result = parse_command(input, &cmd);

    TEST_ASSERT_EQUAL(CMD_ERR_UNKNOWN, result);
}

TEST_CASE("parse_command: empty input returns CMD_ERR_UNKNOWN", "[parser]")
{
    char input[] = "";
    parsed_cmd_t cmd;
    cmd_parse_result_t result = parse_command(input, &cmd);

    TEST_ASSERT_EQUAL(CMD_ERR_UNKNOWN, result);
}

TEST_CASE("parse_command: missing method returns CMD_ERR_NO_METHOD", "[parser]")
{
    char input[] = "http";
    parsed_cmd_t cmd;
    cmd_parse_result_t result = parse_command(input, &cmd);

    TEST_ASSERT_EQUAL(CMD_ERR_NO_METHOD, result);
}

TEST_CASE("parse_command: missing url returns CMD_ERR_NO_URL", "[parser]")
{
    char input[] = "http GET";
    parsed_cmd_t cmd;
    cmd_parse_result_t result = parse_command(input, &cmd);

    TEST_ASSERT_EQUAL(CMD_ERR_NO_URL, result);
}

TEST_CASE("parse_command: unsupported method returns CMD_ERR_BAD_METHOD", "[parser]")
{
    char input[] = "http DELETE http://httpbin.org/get";
    parsed_cmd_t cmd;
    cmd_parse_result_t result = parse_command(input, &cmd);

    TEST_ASSERT_EQUAL(CMD_ERR_BAD_METHOD, result);
}

TEST_CASE("parse_command: lowercase method returns CMD_ERR_BAD_METHOD", "[parser]")
{
    char input[] = "http get http://httpbin.org/get";
    parsed_cmd_t cmd;
    cmd_parse_result_t result = parse_command(input, &cmd);

    TEST_ASSERT_EQUAL(CMD_ERR_BAD_METHOD, result);
}

void app_main(void)
{
    UNITY_BEGIN();
    unity_run_all_tests();
    UNITY_END();
}
