#ifndef HTTP_H
#define HTTP_H

#include "esp_err.h"

// Function prototypes for GET HTTP request
// @param url Full URL to send the GET request, example: "http://httpbin.org/get"
// @return ESP_OK on success, or an error code on failure 
esp_err_t http_get(const char *url);

// Function prototypes for POST HTTP request
// @param url Full URL to send the POST request, example: "http://httpbin.org/post"
// @param body The body of the POST request, nullable, example: "body_param" or ""
// @return ESP_OK on success, or an error code on failure
esp_err_t http_post(const char *url, const char *body);

#endif 
