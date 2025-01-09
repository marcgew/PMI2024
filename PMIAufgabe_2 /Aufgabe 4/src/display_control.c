#include <uart.h>
#include <clocks.h>
#include <systick.h>
#include <pmi_string.h>
#include <ow.h>
#include <DS18B20.h>
#include <i2c_sw.h>
#include <QMC5883L.h>
#include <spi.h>
#include <ADXL345.h>
#include <ili9341.h>

/**
 * @brief Draws static text and labels on the display for sensor data visualization
 *        This function initializes the display and sets up fixed elements for the ADXL345, QMC5883L, and DS18B20
 */
void draw_solids(void)
{
    ili9341_init(ILI9341_ORIENTATION_270); // Initialize the display with a 270-degree orientation

    // Labels for ADXL345 accelerometer data.
    ili9341_text_pos_set(1, 0);
    ili9341_str_print("ADXL345", ILI9341_COLOR_LIGHTGREY, ILI9341_COLOR_BLACK);

    ili9341_text_pos_set(1, 1);
    ili9341_str_print("X:", ILI9341_COLOR_RED, ILI9341_COLOR_BLACK);
    ili9341_text_pos_set(1, 2);
    ili9341_str_print("Y:", ILI9341_COLOR_YELLOW, ILI9341_COLOR_BLACK);
    ili9341_text_pos_set(1, 3);
    ili9341_str_print("Z:", ILI9341_COLOR_BLUE, ILI9341_COLOR_BLACK);
    
    // Labels for QMC5883L magnetometer data
    ili9341_text_pos_set(1, 4);
    ili9341_str_print("QMC5883L", ILI9341_COLOR_LIGHTGREY, ILI9341_COLOR_BLACK);
    ili9341_text_pos_set(1, 5);
    ili9341_str_print("X:", ILI9341_COLOR_RED, ILI9341_COLOR_BLACK);
    ili9341_text_pos_set(1, 6);
    ili9341_str_print("Y:", ILI9341_COLOR_YELLOW, ILI9341_COLOR_BLACK);
    ili9341_text_pos_set(1, 7);
    ili9341_str_print("Z:", ILI9341_COLOR_BLUE, ILI9341_COLOR_BLACK);

    // Label for DS18B20 temperature sensor data
    ili9341_text_pos_set(1, 8);
    ili9341_str_print("DS18B20", ILI9341_COLOR_LIGHTGREY, ILI9341_COLOR_BLACK);
    ili9341_text_pos_set(1, 9);
    ili9341_str_print("T:", ILI9341_COLOR_RED, ILI9341_COLOR_BLACK);


    // Units for each sensor's data
    ili9341_text_pos_set(10, 1);
    ili9341_str_print("m/s^2", ILI9341_COLOR_LIGHTGREY, ILI9341_COLOR_BLACK);
    ili9341_text_pos_set(10, 2);
    ili9341_str_print("m/s^2", ILI9341_COLOR_LIGHTGREY, ILI9341_COLOR_BLACK);
    ili9341_text_pos_set(10, 3);
    ili9341_str_print("m/s^2", ILI9341_COLOR_LIGHTGREY, ILI9341_COLOR_BLACK);

    ili9341_text_pos_set(13, 5);
    ili9341_str_print("T", ILI9341_COLOR_LIGHTGREY, ILI9341_COLOR_BLACK);
    ili9341_text_pos_set(13, 6);
    ili9341_str_print("T", ILI9341_COLOR_LIGHTGREY, ILI9341_COLOR_BLACK);
    ili9341_text_pos_set(13, 7);
    ili9341_str_print("T", ILI9341_COLOR_LIGHTGREY, ILI9341_COLOR_BLACK);

    ili9341_text_pos_set(7, 9);
    ili9341_str_print("K", ILI9341_COLOR_LIGHTGREY, ILI9341_COLOR_BLACK);
}

/**
 * @brief Reads and displays data from the ADXL345 accelerometer on the screen
 */
void draw_ADXL345_Values(void)
{

    float ADXL345_x_float = 0;
    float ADXL345_y_float = 0;
    float ADXL345_z_float = 0;
    char ADXL345_x_str[7];
    char ADXL345_y_str[7];
    char ADXL345_z_str[7];

    ADXL345_collect_data(&ADXL345_x_float, &ADXL345_y_float, &ADXL345_z_float);

    pmi_string_float2str(ADXL345_x_str, 7, ADXL345_x_float, 7);
    pmi_string_float2str(ADXL345_y_str, 7, ADXL345_y_float, 7);
    pmi_string_float2str(ADXL345_z_str, 7, ADXL345_z_float, 7);

    ili9341_text_pos_set(3, 1);
    ili9341_str_clear(6, ILI9341_COLOR_BLACK);
    ili9341_text_pos_set(3, 1);
    ili9341_str_print(ADXL345_x_str, ILI9341_COLOR_WHITE, ILI9341_COLOR_BLACK);

    ili9341_text_pos_set(3, 2);
    ili9341_str_clear(6, ILI9341_COLOR_BLACK);
    ili9341_text_pos_set(3, 2);
    ili9341_str_print(ADXL345_y_str, ILI9341_COLOR_WHITE, ILI9341_COLOR_BLACK);

    ili9341_text_pos_set(3, 3);
    ili9341_str_clear(6, ILI9341_COLOR_BLACK);
    ili9341_text_pos_set(3, 3);
    ili9341_str_print(ADXL345_z_str, ILI9341_COLOR_WHITE, ILI9341_COLOR_BLACK);
}

/**
 * @brief Reads and displays data from the QMC5883L magnetometer on the screen
 */
void draw_QMC5883L_Values(void)
{
    float qmc5883l_x_float = 0;
    float qmc5883l_y_float = 0;
    float qmc5883l_z_float = 0;
    char qmc5883l_x_str[10];
    char qmc5883l_y_str[10];
    char qmc5883l_z_str[10];

    qmc5883l_mag_get(&qmc5883l_x_float, &qmc5883l_y_float, &qmc5883l_z_float);

    pmi_string_float2str(qmc5883l_x_str, 10, qmc5883l_x_float, 7);
    pmi_string_float2str(qmc5883l_y_str, 10, qmc5883l_y_float, 7);
    pmi_string_float2str(qmc5883l_z_str, 10, qmc5883l_z_float, 7);

    ili9341_text_pos_set(3, 5);
    ili9341_str_clear(10, ILI9341_COLOR_BLACK);
    ili9341_text_pos_set(3, 5);
    ili9341_str_print(qmc5883l_x_str, ILI9341_COLOR_WHITE, ILI9341_COLOR_BLACK);

    ili9341_text_pos_set(3, 6);
    ili9341_str_clear(10, ILI9341_COLOR_BLACK);
    ili9341_text_pos_set(3, 6);
    ili9341_str_print(qmc5883l_y_str, ILI9341_COLOR_WHITE, ILI9341_COLOR_BLACK);

    ili9341_text_pos_set(3, 7);
    ili9341_str_clear(10, ILI9341_COLOR_BLACK);
    ili9341_text_pos_set(3, 7);
    ili9341_str_print(qmc5883l_z_str, ILI9341_COLOR_WHITE, ILI9341_COLOR_BLACK);
    ili9341_text_pos_set(13, 7);
}

/**
 * @brief Reads and displays data from the DS18B20 temperature sensor on the screen
 */
void draw_DS18B20_Values(void)
{
    float DS18B20_float = 0;
    char DS18B20_str[4];

    DS18B20_float = DS18B20_get_temp();

    pmi_string_float2str(DS18B20_str, 4, DS18B20_float, 4);

    ili9341_text_pos_set(3, 9);
    ili9341_str_clear(3, ILI9341_COLOR_BLACK);
    ili9341_text_pos_set(3, 9);
    ili9341_str_print(DS18B20_str, ILI9341_COLOR_WHITE, ILI9341_COLOR_BLACK);
}

/**
 * @brief Updates all sensor values on the display
 */
void draw_values(void)
{
    draw_ADXL345_Values();

    draw_QMC5883L_Values();

    draw_DS18B20_Values();
}