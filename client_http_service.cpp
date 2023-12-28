#include <Arduino.h>
#include <esp_http_client.h>
#include "esp_camera.h"

#define BOUNDARY "----BoundaryString"

static const char *TAG = "camera_httpd";

esp_err_t capture_and_process_image();
esp_err_t send_post_request(const char *, const uint8_t *, size_t);
esp_err_t start_mjpeg_stream(const char *);
const char *url = "http://someIP:3000/api/v1/camera1"

// Function to capture an image and convert it to JPEG
esp_err_t capture_and_process_image()
{
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb)
    {
        ESP_LOGE(TAG, "Camera capture failed");
        return ESP_FAIL;
    }
    if (send_post_request(url, fb->buf, fb->len) != ESP_OK)
    {
        ESP_LOGE(TAG, "Camera capture failed");
        return ESP_FAIL;
    }
    esp_camera_fb_return(fb);
    return ESP_OK;
}

// Function to send POST request
esp_err_t send_post_request(const char *url, const uint8_t *data, size_t len)
{
    // Initialize the HTTP client
    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_POST,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    // Construct the multipart body
    char header[128];
    int header_len = snprintf(header, sizeof(header),
                              "--%s\r\nContent-Disposition: form-data; name=\"file\"; filename=\"image.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n",
                              BOUNDARY);

    char footer[64];
    int footer_len = snprintf(footer, sizeof(footer), "\r\n--%s--\r\n", BOUNDARY);

    int total_len = header_len + len + footer_len;
    char *body = (char *)malloc(total_len);
    if (!body)
    {
        ESP_LOGE(TAG, "Failed to allocate memory for POST body");
        return ESP_ERR_NO_MEM;
    }

    // Copy the header, data, and footer to the body
    memcpy(body, header, header_len);
    memcpy(body + header_len, data, len);
    memcpy(body + header_len + len, footer, footer_len);

    // Set headers and body
    esp_http_client_set_header(client, "Content-Type", "multipart/form-data; boundary=" BOUNDARY);
    esp_http_client_set_post_field(client, body, total_len);

    // Perform the request
    esp_err_t err = esp_http_client_perform(client);

    // Log the result and cleanup
    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %d",
                 esp_http_client_get_status_code(client),
                 esp_http_client_get_content_length(client));
    }
    else
    {
        ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
    }

    // Clean up
    esp_http_client_cleanup(client);
    free(body);

    return err;
}

esp_err_t start_mjpeg_stream(const char *url)
{
    // Initialize the HTTP client for streaming
    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_POST,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    // Headers to start the MJPEG stream
    esp_http_client_set_header(client, "Content-Type", "multipart/x-mixed-replace;boundary=frame");

    while (true)
    {
        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb)
        {
            ESP_LOGE(TAG, "Camera capture failed");
            break;
        }

        // Convert to JPEG if necessary
        size_t jpg_buf_len = 0;
        uint8_t *jpg_buf = NULL;
        bool converted = frame2jpg(fb, 80, &jpg_buf, &jpg_buf_len);
        esp_camera_fb_return(fb);

        if (!converted)
        {
            ESP_LOGE(TAG, "JPEG conversion failed");
            break;
        }

        // Send frame header
        char *header;
        asprintf(&header, "--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %d\r\n\r\n", jpg_buf_len);
        esp_http_client_write(client, header, strlen(header));
        free(header);

        // Send frame data
        esp_http_client_write(client, (const char *)jpg_buf, jpg_buf_len);
        free(jpg_buf);

        // Check for a stop condition or perform a delay as needed
        // ...
    }

    // Cleanup
    esp_http_client_cleanup(client);
    return ESP_OK;
}
