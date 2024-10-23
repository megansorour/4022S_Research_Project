// Header for File containing raw wave data arrays to be transmitted

#ifndef DATA_ARRAYS_H
#define DATA_ARRAYS_H

#include <stdint.h>

// Type def for a certain data entry, compressed data
typedef struct {
    uint8_t node_ID;       // ID of the node the data is from
    uint8_t data_name;        // Name of the data ie W1.TXT
    uint8_t* x_vals;
    uint8_t* y_vals;
    uint8_t* z_vals;
    uint8_t* r_vals;
    uint8_t* q_vals;
    uint8_t* p_vals;
} DataEntry;

// Define the number of measurements processed
#define NUM_ENTRIES    1365

//Codes for each sensor data type
#define TYPE_X 1
#define TYPE_Y 2
#define TYPE_Z 3
#define TYPE_R 4
#define TYPE_Q 5
#define TYPE_P 6


// summary packet
extern const uint8_t payload0[];

// Declare the arrays
extern int8_t hours[NUM_ENTRIES];         // Array for hours
extern int8_t minutes[NUM_ENTRIES];       // Array for minutes
extern int8_t seconds[NUM_ENTRIES];       // Array for seconds
extern int16_t x_vals[NUM_ENTRIES];       // Array for X-axis values
extern int16_t y_vals[NUM_ENTRIES];       // Array for Y-axis values
extern int16_t z_vals[NUM_ENTRIES];       // Array for Z-axis values
extern int16_t r_vals[NUM_ENTRIES];       // Array for R-axis values
extern int16_t q_vals[NUM_ENTRIES];       // Array for Q-axis values
extern int16_t p_vals[NUM_ENTRIES];       // Array for P-axis values

#endif // DATA_ARRAYS_H
