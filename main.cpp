#include <Arduino.h>
#include <driver/i2s.h>
#include "soss.h"
#include "voice.h"
#include "esp_camera.h"
#include <WiFi.h>

unsigned long previous_milisec = 0;
const long interval = 5000;
boolean start_timer = false;
const char *ssid = "xxxxxxx";          // input your wifi name
const char *password = "xxxxxxxx"; // input your wifi passwords
camera_config_t config;

// void configureI2S();
// void playVoiceNote();
void detectMovement();
void config_init();
esp_err_t capture_and_process_image();
esp_err_t send_post_request(const char *, const uint8_t *, size_t);

void setup()
{
  // Initialize Serial for debugging
  Serial.begin(115200);

  // Camera Init
  config_init();
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK)
  {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  sensor_t *s = esp_camera_sensor_get();
  s->set_vflip(s, 0);       // 1-Upside down, 0-No operation
  s->set_hmirror(s, 0);     // 1-Reverse left and right, 0-No operation
  s->set_brightness(s, 1);  // up the blightness just a bit
  s->set_saturation(s, -1); // lower the saturation

  // Configure I2S
  // configureI2S();

  // Wifi Connection
  WiFi.begin(ssid, password);
  unsigned long prev_mili = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    unsigned long current_mills = millis();
    if (current_mills - prev_mili >= 500)
    {
      prev_mili = current_mills;
      Serial.print(".");
    }
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");

  // PIR interrupt
  pinMode(MOTION_SENSOR, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(MOTION_SENSOR), detectMovement, RISING);
}

void loop()
{
  unsigned long current_mills = millis();

  if (start_timer && (current_mills - previous_milisec >= interval))
  {
    // playVoiceNote();
    Serial.println("Before taking the capture....");
    // capture_and_process_image();
    Serial.println("After capture was taken....");
    start_timer = false;
  }
}

// // I2S Configuration
// void configureI2S()
// {
//   i2s_config_t i2sConfig = {
//       .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
//       .sample_rate = 22050,
//       .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
//       .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // Mono configuration
//       .communication_format = I2S_COMM_FORMAT_I2S_MSB,
//       .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
//       .dma_buf_count = 8,
//       .dma_buf_len = 64};

//   i2s_pin_config_t pinConfig = {
//       .bck_io_num = I2S_BCLK,
//       .ws_io_num = I2S_LRC,
//       .data_out_num = I2S_DOUT,
//       .data_in_num = I2S_PIN_NO_CHANGE};

//   i2s_driver_install((i2s_port_t)0, &i2sConfig, 0, NULL);
//   i2s_set_pin((i2s_port_t)0, &pinConfig);
//   i2s_set_clk((i2s_port_t)0, 22050, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO); // Set for mono
// }
// // I2S Write -> play note
// void playVoiceNote()
// {
//   for (int i = 0; i < voiceNoteSize; i++)
//   {
//     size_t bytes_written;
//     i2s_write((i2s_port_t)0, (const char *)&voiceNoteData[i], 2, &bytes_written, portMAX_DELAY);
//   }
// }
// Interrupt Callback -> PIR Sensor Detection Flag
void detectMovement()
{
  unsigned long current_mills = millis();
  start_timer = true;
  previous_milisec = current_mills;
}
// Camera Configuration
void config_init()
{
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_VGA;
  config.pixel_format = PIXFORMAT_JPEG; // for streaming
  // config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_DRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;
}
