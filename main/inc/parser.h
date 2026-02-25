#ifndef PARSER_H
#define PARSER_H

typedef enum {
    CMD_OK, // Command parsed successfully
    CMD_ERR_UNKNOWN, // Unknown command (not "http")
    CMD_ERR_NO_METHOD, // Method missing
    CMD_ERR_NO_URL, // URL missing
    CMD_ERR_BAD_METHOD, // Method not GET or POST
} cmd_parse_result_t;

typedef enum {
    HTTP_GET,
    HTTP_POST,
} cmd_method_t;

typedef struct {
    cmd_method_t method;
    char *url;
    char *body;
} parsed_cmd_t;

// Parses the input command string and fills the output structure.
// Returns a cmd_parse_result_t indicating the result of parsing.
cmd_parse_result_t parse_command(char *input, parsed_cmd_t *output);

// Utility function to strip trailing newline characters from the input string.
void strip_line_ending(char *input);

#endif
