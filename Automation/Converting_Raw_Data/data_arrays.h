#ifndef DATA_ARRAYS_H
#define DATA_ARRAYS_H

// Define the number of measurements processed
#define NUM_ENTRIES    1365

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
