import re

# Input and output file paths
txt_file = 'W1.TXT'  # raw wave data
c_file = 'output.c'    # output .c arrays
h_file = 'data_arrays.h'  # corresponding .h for arrays

# Max data size limit
max_size_bytes = 20 * 1024  # 50KB

# Data type size in bytes
int8_size = 1   # 1 byte for int8_t (for timestamps)
int16_size = 2  # 2 bytes for int16_t (for x, y, z, r, q, p)

# size index
total_size = 0

# lists for each type of data
hours = []
minutes = []
seconds = []
x_vals = []
y_vals = []
z_vals = []
r_vals = []
q_vals = []
p_vals = []

# Process each line and extract the data
with open(txt_file, 'r') as file:
    for line in file:
        # Estimate the size of the current line in terms of data storage
        line_size = (3 * int8_size) + (6 * int16_size)  # 3 timestamp bytes + 6 measurement shorts

        # Stop if the total size exceeds 50kB
        if total_size + line_size > max_size_bytes:
            print("Reached the 50KB limit, stopping processing.")
            break
        
        # Split the line into timestamp and values
        timestamp, values = line.split(',', 1)
        
        # Extract the timestamp parts (HH:MM:SS)
        h, m, s = map(int, timestamp.split(':'))
        hours.append(h)
        minutes.append(m)
        seconds.append(s)
        
        # Extract the numeric values after the timestamp
        match = re.findall(r'-?\d+', values)
        if match:
            # Convert to integers and store in respective lists
            x_vals.append(int(match[0]))
            y_vals.append(int(match[1]))
            z_vals.append(int(match[2]))
            r_vals.append(int(match[3]))
            q_vals.append(int(match[4]))
            p_vals.append(int(match[5]))

        # Increment total size by the size of the current line
        total_size += line_size

# Get the number of entries (size of one of the arrays, e.g., x_vals)
num_entries = len(x_vals)

# Write the data to a .c file as separate arrays
with open(c_file, 'w') as file:
    # Write timestamp arrays
    file.write(f"// Hour array\n")
    file.write(f"int8_t hours[{num_entries}] = {{" + ", ".join(map(str, hours)) + "};\n\n")
    
    file.write(f"// Minute array\n")
    file.write(f"int8_t minutes[{num_entries}] = {{" + ", ".join(map(str, minutes)) + "};\n\n")
    
    file.write(f"// Second array\n")
    file.write(f"int8_t seconds[{num_entries}] = {{" + ", ".join(map(str, seconds)) + "};\n\n")

    # Write separate arrays for each measurement using int16_t
    file.write(f"// X values array\nint16_t x_vals[{num_entries}] = {{" + ", ".join(map(str, x_vals)) + "};\n\n")
    file.write(f"// Y values array\nint16_t y_vals[{num_entries}] = {{" + ", ".join(map(str, y_vals)) + "};\n\n")
    file.write(f"// Z values array\nint16_t z_vals[{num_entries}] = {{" + ", ".join(map(str, z_vals)) + "};\n\n")
    file.write(f"// R values array\nint16_t r_vals[{num_entries}] = {{" + ", ".join(map(str, r_vals)) + "};\n\n")
    file.write(f"// Q values array\nint16_t q_vals[{num_entries}] = {{" + ", ".join(map(str, q_vals)) + "};\n\n")
    file.write(f"// P values array\nint16_t p_vals[{num_entries}] = {{" + ", ".join(map(str, p_vals)) + "};\n")

# Print the total size processed and number of entries
print(f"Processed {total_size} bytes of data with {num_entries} entries.")

# Write the corresponding .h file
with open(h_file, 'w') as file:
    file.write(f"#ifndef DATA_ARRAYS_H\n")
    file.write(f"#define DATA_ARRAYS_H\n\n")
    
    file.write(f"// Define the number of measurements processed\n")
    file.write(f"#define NUM_ENTRIES    {num_entries}\n\n")
    
    file.write(f"// Declare the arrays\n")
    file.write(f"extern int8_t hours[NUM_ENTRIES];         // Array for hours\n")
    file.write(f"extern int8_t minutes[NUM_ENTRIES];       // Array for minutes\n")
    file.write(f"extern int8_t seconds[NUM_ENTRIES];       // Array for seconds\n")
    file.write(f"extern int16_t x_vals[NUM_ENTRIES];       // Array for X-axis values\n")
    file.write(f"extern int16_t y_vals[NUM_ENTRIES];       // Array for Y-axis values\n")
    file.write(f"extern int16_t z_vals[NUM_ENTRIES];       // Array for Z-axis values\n")
    file.write(f"extern int16_t r_vals[NUM_ENTRIES];       // Array for R-axis values\n")
    file.write(f"extern int16_t q_vals[NUM_ENTRIES];       // Array for Q-axis values\n")
    file.write(f"extern int16_t p_vals[NUM_ENTRIES];       // Array for P-axis values\n\n")
    
    file.write(f"#endif // DATA_ARRAYS_H\n")

print(f"Header file 'data_arrays.h' generated with {num_entries} entries.")
