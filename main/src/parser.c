#include <string.h>
#include "parser.h"

void strip_line_ending(char *input) {
    size_t len = strlen(input);
    while (len > 0 && (input[len - 1] == '\n' || input[len - 1] == '\r')) {
        input[--len] = '\0';
    }
} 

cmd_parse_result_t parse_command(char *input, parsed_cmd_t *output) {
    char *command = strtok(input, " ");
    if (command == NULL || strcmp(command, "http") != 0) {
        return CMD_ERR_UNKNOWN;
    }

    char *method = strtok(NULL, " ");
    if (method == NULL) {
        return CMD_ERR_NO_METHOD;
    }

    char *url = strtok(NULL, " ");
    if (url == NULL) {
        return CMD_ERR_NO_URL;
    }

    if (strcmp(method, "GET") == 0) {
        output->method = HTTP_GET;
        output->url = url;
        output->body = NULL;
        return CMD_OK;
    }

    if (strcmp(method, "POST") == 0) {
        char *body = strtok(NULL, "");
        output->method = HTTP_POST;
        output->url = url;
        output->body = body; // NULL if not provided — caller handles this
        return CMD_OK;
    }

    return CMD_ERR_BAD_METHOD;
}
