// C program for compression functions
/*
An implementation of Delta/Huffman Compressor (DHC) for embedded systems.

Copyright (c) 2022
Marcelo Barros de Almeida <marcelobarrosalmeida@gmail.com>
Marden Fagundes <maf_cadastro-github@yahoo.com>
*/

// Addition of RLE compression algorithm for timestamps by Megan Sorour

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

#include "sys_app.h"

#include "compression.h"

#include "subghz_phy_app.h"

#define DHC_DEBUG 0
#define DHC_ABS(x)     (((x) < 0) ? -(x) : (x))

typedef struct dhc_stream_ctrl_s
{
    uint8_t *stream;
    uint16_t byte_pos;
    uint8_t bit_pos;
    uint32_t size_bits;
} dhc_stream_ctrl_t;

struct dhc_table_s
{
    uint16_t num_bits_prefix;
    uint16_t num_bits_index;
    uint32_t value_prefix;
    uint8_t *str_prefix;
};

static void dhc_io_put8_tb(uint8_t value, uint8_t *buf)
{
    buf[0] = value;
}

static void dhc_io_put8_tb_apr(uint8_t value, uint8_t **buf)
{
    dhc_io_put8_tb(value,*buf);
    *buf += 1;
}

static void dhc_io_put32_tb(uint32_t value, uint8_t *buf)
{
    buf[3] = (uint8_t)(value      );
    buf[2] = (uint8_t)(value >> 8 );
    buf[1] = (uint8_t)(value >> 16);
    buf[0] = (uint8_t)(value >> 24);
}

static void dhc_io_put32_tb_apr(uint32_t value, uint8_t **buf)
{
    dhc_io_put32_tb(value,*buf);
    *buf += 4;
}

#define dhc_io_put8_tb_ap(v,x)  dhc_io_put8_tb_apr(v,&x)
#define dhc_io_put32_tb_ap(v,x) dhc_io_put32_tb_apr(v,&x)

// prefix bits, index bits, prefix value, prefix string
struct dhc_table_s dhc_table[] = {
    /*  0 */ { 2,  0, 0x00000000, (uint8_t *) "00"},
    /*  1 */ { 3,  1, 0x00000002, (uint8_t *) "010"},
    /*  2 */ { 3,  2, 0x00000006, (uint8_t *) "011"},
    /*  3 */ { 3,  3, 0x00000001, (uint8_t *) "100"},
    /*  4 */ { 3,  4, 0x00000005, (uint8_t *) "101"},
    /*  5 */ { 3,  5, 0x00000003, (uint8_t *) "110"},
    /*  6 */ { 4,  6, 0x00000007, (uint8_t *) "1110"},
    /*  7 */ { 5,  7, 0x0000000F, (uint8_t *) "11110"},
    /*  8 */ { 6,  8, 0x0000001F, (uint8_t *) "111110"},
    /*  9 */ { 7,  9, 0x0000003F, (uint8_t *) "1111110"},
    /* 10 */ { 8, 10, 0x0000007F, (uint8_t *) "11111110"},
    /* 11 */ { 9, 11, 0x000000FF, (uint8_t *) "111111110"},
    /* 12 */ {10, 12, 0x000001FF, (uint8_t *) "1111111110"},
    /* 13 */ {11, 13, 0x000003FF, (uint8_t *) "11111111110"},
    /* 14 */ {12, 14, 0x000007FF, (uint8_t *) "111111111110"},
    /* 15 */ {13, 15, 0x00000FFF, (uint8_t *) "1111111111110"},
    /* 16 */ {14, 16, 0x00001FFF, (uint8_t *) "11111111111110"},
    };

static uint8_t dhc_log2_down_abs(int16_t _value)
{
    uint8_t res = 0;
    uint16_t value = (uint16_t) DHC_ABS(_value);

    if(value)
    {
        res = -1;
        while(value)
        {
            value = value >> 1;
            res++;
        }
    }

    return res;
}

static uint8_t dhc_log2_up_abs(int16_t _value)
{
    uint8_t res = 0;
    uint16_t value = (uint16_t) DHC_ABS(_value);

    if(value)
    {
        while(value)
        {
            value = value >> 1;
            res++;
        }
    }

    return res;
}

static uint32_t dhc_pow2(uint8_t value)
{
    uint32_t pow = 1;

    if(value)
    {
        while(value)
        {
            value--;
            pow = pow*2;
        }
    }

    return pow;
}

static void dhc_hist_reset(uint32_t *hist, uint8_t *map)
{
    for(size_t pos = 0 ; pos < DHC_TABLE_SIZE ; pos++)
    {
        hist[pos] = 0;
        map[pos] = pos;
    }
}
static void dhc_hist_update(uint32_t *hist, uint8_t bin)
{
    hist[bin]++;
}

static void dhc_hist_print(uint32_t *hist, uint8_t *map)
{
#if DHC_DEBUG == 1
    APP_PRINTF("HISTOGRAM:\r\n");

    bool use_map = map == 0 ? false : true;

    for(size_t bin = 0 ; bin < DHC_TABLE_SIZE ; bin++)
    {
        if(use_map){
            APP_PRINTF("[%02u][%05u] %u (±%u±%u)\r\n",map[bin],dhc_pow2(map[bin]),hist[bin],map[bin]>0?dhc_pow2(map[bin]-1):0,dhc_pow2(map[bin])-1);
        }
        else {
            APP_PRINTF("[%02lu][%05u] %u (±%u±%u)\r\n",bin,dhc_pow2(bin),hist[bin],bin>0?dhc_pow2(bin-1):0,dhc_pow2(bin)-1);
        }
    }
#endif
}

static void dhc_hist_bubble_sort(uint32_t *hist, uint8_t *idx, size_t size)
{
    size_t i, j;

    for (i = 0; i < size-1; i++)
    {
        for (j = 0; j < size-i-1; j++)
        {
            if (hist[j] < hist[j+1])
            {
                uint32_t t = hist[j];
                hist[j] = hist[j+1];
                hist[j+1] = t;

                uint8_t v = idx[j];
                idx[j] = idx[j+1];
                idx[j+1] = v;
            }
        }
    }
}

#if DHC_DEBUG == 1
static void dhc_prefix_print(uint16_t bin)
{
    APP_PRINTF("%s",dhc_table[bin].str_prefix);
}

static void dhc_index_print(uint16_t value, uint16_t num_bits)
{
    for(size_t pos = num_bits ; pos > 0 ; pos--)
    {
        APP_PRINTF("%c",(value >> (pos-1)) & 0x01 ? '1' : '0');
    }
}
#endif

static void dhc_stream_init(dhc_stream_ctrl_t *ctrl, uint8_t *stream, uint16_t size, bool compress)
{
    ctrl->stream = stream;
    ctrl->bit_pos = 0;
    ctrl->byte_pos = 0;

    if(compress)
        ctrl->stream[0] = 0;
}

static uint32_t dhc_stream_size_bits_get(dhc_stream_ctrl_t *ctrl)
{
    return ctrl->byte_pos*8 + ctrl->bit_pos;
}

static void dhc_stream_add(dhc_stream_ctrl_t *ctrl, uint32_t bit_stream, uint8_t size)
{
    uint8_t remain = size;

    if(size == 0)
        return;

    ctrl->stream[ctrl->byte_pos] |= (uint8_t)(bit_stream << ctrl->bit_pos);

    if(ctrl->bit_pos + size >= 8)
    {
        remain -= (8-ctrl->bit_pos);
        bit_stream >>= (8-ctrl->bit_pos);
        ctrl->bit_pos = 0;
        ctrl->byte_pos++;
        ctrl->stream[ctrl->byte_pos] = 0;
    }
    else
    {
        ctrl->bit_pos += size;
        remain -= size;
    }

    while(remain)
    {
        ctrl->stream[ctrl->byte_pos] = (uint8_t)(bit_stream);

        if(remain < 8)
        {
            ctrl->bit_pos = remain;
            remain = 0;
        }
        else
        {
            bit_stream >>= 8;
            remain -= 8;
            ctrl->byte_pos++;
            ctrl->stream[ctrl->byte_pos] = 0;
        }
    }
}

static uint32_t dhc_stream_next_bits_get(dhc_stream_ctrl_t *ctrl, uint8_t size)
{
    uint8_t remain = size;
    uint32_t ofs = 0;

    if(size == 0)
        return 0;

    uint32_t bit_stream = (uint8_t)(ctrl->stream[ctrl->byte_pos] >> ctrl->bit_pos);

    if(ctrl->bit_pos + size >= 8)
    {
        remain -= (8-ctrl->bit_pos);
        ofs = (8-ctrl->bit_pos);
        ctrl->bit_pos = 0;
        ctrl->byte_pos++;
    }
    else
    {
        ctrl->bit_pos += size;
        remain -= size;
        bit_stream &= ~(0xFFFFFFFFL << size);
    }

    while(remain)
    {
        bit_stream |= (ctrl->stream[ctrl->byte_pos] << ofs);

        if(remain < 8)
        {
            ctrl->bit_pos = remain;
            remain = 0;
            bit_stream &= ~(0xFFFFFFFFL << (ctrl->bit_pos + ofs));
        }
        else
        {
            ofs += 8;
            remain -= 8;
            ctrl->byte_pos++;
        }
    }

    return bit_stream;
}

static uint32_t dhc_stream_prefix_end_get(dhc_stream_ctrl_t *ctrl)
{
    uint32_t token = 0;
    uint32_t ofs = 0;
    uint8_t bit;

    do
    {
        bit = (ctrl->stream[ctrl->byte_pos] >> ctrl->bit_pos) & 0x01;
        token |= (bit << ofs);

        if(++ctrl->bit_pos >= 8)
        {
            ctrl->bit_pos = 0;
            ctrl->byte_pos++;
        }

        ofs++;

    } while(bit);

    return token;
}

static int16_t dhc_index2value(int16_t index, uint8_t bin)
{
    int16_t value;
    uint32_t pwr_bin = dhc_pow2(bin);

    if(index < (pwr_bin/2))
        value = 1 + index - pwr_bin;
    else
        value = index;

    return value;
}

static int16_t dhc_stream_next_value_get(dhc_stream_ctrl_t *ctrl, uint8_t *map)
{
    int16_t index = 0;
    int16_t data = 0;
    uint32_t bit_stream;
    uint8_t bin = 0;
    uint8_t map_bin;

    bit_stream = dhc_stream_next_bits_get(ctrl,dhc_table[0].num_bits_prefix);

    if(bit_stream == dhc_table[0].value_prefix) // 00
    {
        bin = 0;
    }
    else
    {
        bit_stream |= (dhc_stream_next_bits_get(ctrl,1) << 2);

        for(bin = 1 ; bin < 6 ; bin++)
        {
            if(bit_stream == dhc_table[bin].value_prefix)
                break;
        }

        if(bin >= 6)
        {
            // 110, 1110, 11110 ...
            // 011, 0111, 01111 ...
            // 3, 7, F, 1F, 3F ...
            // +1: 4, 8, 16, 32, 64 ...
            // log2(): 2, 3, 4, 5, ...
            // +3: 5, 6, 7, 8 ...
            bit_stream |= (dhc_stream_prefix_end_get(ctrl) << 3);
            bin = dhc_log2_down_abs(bit_stream + 1) + 3;
        }
    }

    map_bin = map ? map[bin] : bin;

#if DHC_DEBUG == 1
    dhc_prefix_print(bin);
    APP_PRINTF(" ");
    fflush(stdout);
#endif

    index = dhc_stream_next_bits_get(ctrl,dhc_table[map_bin].num_bits_index);
    data = dhc_index2value(index,map_bin);

#if DHC_DEBUG == 1
    dhc_index_print(index,dhc_table[map_bin].num_bits_index);
    APP_PRINTF(" (%d -> %d) (%d -> %d)\r\n",data,index,bin,map_bin);
    fflush(stdout);
#endif

    return data;
}

bool dhc_compress(uint8_t *output, uint32_t *output_size_bits, int16_t *data, uint32_t size, uint8_t *map)
{
    uint8_t bin;
    int16_t index;
    int16_t delta;
    int16_t prev_value;
    uint8_t map_bin;
    dhc_stream_ctrl_t ctrl =  { 0 };

    if((size < 1)||(data == 0)||(output == 0)||(output_size_bits == 0))
        return false;

    dhc_stream_init(&ctrl,output,size,true);

    // data[-1] => should be central value among all possible discrete values
    // Worth the calc ?
    prev_value = 0;

    for(size_t pos = 0 ; pos < size ; pos++)
    {
        delta = data[pos] - prev_value;
        prev_value = data[pos];
        bin = dhc_log2_up_abs(delta);

        if(delta >= 0)
            index = delta;
        else
            index = dhc_pow2(bin) - 1 - DHC_ABS(delta);

        map_bin = map ? map[bin] : bin;

#if DHC_DEBUG == 1
        dhc_prefix_print(map_bin);
        APP_PRINTF(" ");
        dhc_index_print(index,dhc_table[bin].num_bits_index);
        APP_PRINTF(" (%d -> %d) (%d -> %d)\r\n",delta,index,bin,map_bin);
        fflush(stdout);
#endif

        dhc_stream_add(&ctrl,dhc_table[map_bin].value_prefix,dhc_table[map_bin].num_bits_prefix);
        dhc_stream_add(&ctrl,index,dhc_table[bin].num_bits_index);
    }

    *output_size_bits = dhc_stream_size_bits_get(&ctrl);

    return true;
}

float dhc_compress_evaluate(int16_t *data, uint32_t size, uint32_t *compressed_size_in_bits, uint8_t *map)
{
    uint8_t bin;
    int16_t delta;
    int16_t prev_value;
    uint32_t hist[DHC_TABLE_SIZE];
    uint8_t map_bin[DHC_TABLE_SIZE];
    // int16_t index;

    if((size < 1)||(data == 0))
        return 0.0;

    dhc_hist_reset(hist,map_bin);

    uint32_t total_bits = 0;
    uint32_t compressed_bits = 0;

    // data[-1] => should be central value among all possible discrete values
    // Worth the calc ?
    prev_value = 0;

    for(size_t pos = 0 ; pos < size ; pos++)
    {
        delta = data[pos] - prev_value;
        prev_value = data[pos];
        bin = dhc_log2_up_abs(delta);
        dhc_hist_update(hist,bin);

        total_bits += 16;
        compressed_bits += dhc_table[bin].num_bits_prefix + dhc_table[bin].num_bits_index;
    }

    dhc_hist_print(hist,0);

    float compress_rate = 100.0*((float)total_bits-compressed_bits)/total_bits;
#if DHC_DEBUG == 1
    APP_PRINTF("Default mapping: %d bits -> %d bits (compressed by %2.2f%%)\r\n",total_bits,compressed_bits,compress_rate);
#endif

//    if(map)
//    {
//        // using better mapping based on frequency ---------------------------
//        // weighting by number of bits ?
//        // for(size_t pos = 0 ; pos < DHC_TABLE_SIZE ; pos++)
//        //    hist[pos] *= dhc_table[pos].num_bits_index > 0 ? dhc_table[pos].num_bits_index : 1;
//
//        dhc_hist_bubble_sort(hist,map_bin,DHC_TABLE_SIZE);
//
//        for(size_t pos = 0 ; pos < DHC_TABLE_SIZE ; pos++)
//            map[map_bin[pos]] = pos;
//
//        dhc_hist_print(hist,map);
//
//        total_bits = 0;
//        compressed_bits = 0;
//        prev_value = 0;
//
//        for(size_t pos = 0 ; pos < size ; pos++)
//        {
//            delta = data[pos] - prev_value;
//            prev_value = data[pos];
//            bin = dhc_log2_up_abs(delta);
//
//            total_bits += 16;
//            compressed_bits += dhc_table[map[bin]].num_bits_prefix + dhc_table[bin].num_bits_index;
//        }
//
//        compress_rate = 100.0*((float)total_bits-compressed_bits)/total_bits;
//#if DHC_DEBUG == 1
//        APP_PRINTF("Sorted mapping: %d bits -> %d bits (compressed by %2.2f%%)\r\n",total_bits,compressed_bits,compress_rate);
//#endif
//    }

    *compressed_size_in_bits = compressed_bits;

    return compress_rate;
}

bool dhc_decompress(int16_t *output, uint32_t *output_size, uint8_t *data, uint32_t data_size_bits, uint8_t *map)
{
    dhc_stream_ctrl_t ctrl =  { 0 };
    uint32_t pos = 0;
    int16_t prev_value = 0;
    int16_t delta;
    uint8_t map_bin[DHC_TABLE_SIZE];
    uint8_t *pmap = map_bin;

    if((data_size_bits < 2)||(output == 0)||(data == 0))
        return false;

    uint32_t size = data_size_bits % 8 == 0 ? data_size_bits / 8 : data_size_bits/8 + 1;
    dhc_stream_init(&ctrl,data,size,false);

    // build reverse map
    if(map)
    {
        for(size_t pos = 0 ; pos < DHC_TABLE_SIZE ; pos++)
            map_bin[map[pos]] = pos;
    }
    else
        pmap = 0;

    while(dhc_stream_size_bits_get(&ctrl) < data_size_bits)
    {
        delta = dhc_stream_next_value_get(&ctrl,pmap);
        output[pos] = delta + prev_value;
        prev_value = output[pos];
        pos++;
    }

    *output_size = pos;

    return true;
}

/*
header DHC_COMPRESS_MODE_HUFFMAN_MAPPED

BB: mode (1 byte)
LLLLLLLL: size in bits (4 bytes)
*/
bool dhc_file_write(char *file_name, uint8_t *data, uint32_t size_bits, dhc_compress_mode_t mode)
{
    bool ret = true;
    uint8_t header[5];
    uint8_t *pbuf;
    uint32_t size;

    if(!size_bits || !data || !file_name)
        return false;

    pbuf = header;
    dhc_io_put8_tb_ap(mode,pbuf);
    dhc_io_put32_tb_ap(size_bits,pbuf);

    FILE *fp = fopen(file_name,"w+b");

    if(fp == NULL)
        return false;

    size = 5;

    if(fwrite(header,1,size,fp) == size)
    {
        size = size_bits % 8 == 0 ? size_bits / 8 : size_bits/8 + 1;

        if(fwrite(data,1,size,fp) != size)
            ret = false;
    }
    else
        ret = false;

    fclose(fp);

    return ret;
}

//for implementation of a dynamic array for RLE
void initArray_16(DynamicArray_16 *array, size_t initialCapacity) {
    array->data = malloc(initialCapacity * sizeof(int16_t));
    array->size = 0;
    array->capacity = initialCapacity;
}

void initArray_8(DynamicArray_8 *array, size_t initialCapacity) {
    array->data = malloc(initialCapacity * sizeof(int8_t));
    array->size = 0;
    array->capacity = initialCapacity;
}

void append_16(DynamicArray_16 *array, int16_t value) {
    if (array->size == array->capacity) {
        array->capacity *= 2; // Double the capacity
        array->data = realloc(array->data, array->capacity * sizeof(int16_t));
    }
    array->data[array->size++] = value;
}


void append_8(DynamicArray_8 *array, int8_t value) {
    if (array->size == array->capacity) {
        array->capacity *= 2; // Double the capacity
        array->data = realloc(array->data, array->capacity * sizeof(int8_t));
    }
    array->data[array->size++] = value;
}

void freeArray_16(DynamicArray_16 *array) {
    free(array->data);
}

void freeArray_8(DynamicArray_8 *array) {
    free(array->data);
}

//Function for encoding timestamp (3 uint8_t arrays)
bool compress_timestamp(int8_t *samples, int32_t sample_size, DynamicArray_8 *encoded_vals, DynamicArray_16 *encoded_count) {
	int8_t currentDig; //keeping track of the digit in the run
	//int8_t prevDig = -1; //make negative at first, time can't be negative
	uint32_t digCount = 0; //counter to keep track of how many occurrences there are of a specific number in a row
	uint32_t i = 0;
	initArray_8(encoded_vals, 1);
	initArray_16(encoded_count, 1);
#if (PRINT_ALL == 1)
	APP_PRINTF("Sample size: %d\r\n", sample_size);
#endif
	for (i = 0; i < sample_size; i++) {
		currentDig = samples[i];
		digCount = 1;
#if (PRINT_ALL == 1)
		APP_PRINTF("Current digit: %d, ", currentDig);
		HAL_Delay(10);
#endif
		while((samples[i+1]==currentDig)&&(i+1 < sample_size)) { //counts the digits in a run, and increments for loop counter i
			digCount++;
			i++;
		}
#if (PRINT_ALL == 1)
		APP_PRINTF("%d occurrences \n", digCount);
		HAL_Delay(10);
#endif
		append_8(encoded_vals, currentDig);
		append_16(encoded_count, (int16_t)digCount);

	}
	return true;
}
