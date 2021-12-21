
/****************************************************************/
// Display section
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
/****************************************************************/

/****************************************************************/
// DHT Section
#define DHTTYPE DHT11
#define GLOBAL_SOFTWARE_VERSION "0.1.0"
#define DHT_PIN 3
/****************************************************************/

/****************************************************************/
#define RELAY_PIN D0
/****************************************************************/
