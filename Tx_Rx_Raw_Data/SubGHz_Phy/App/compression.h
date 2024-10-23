// Header file for compression functions

/*
An implementation of Delta/Huffman Compressor (DHC) for embedded systems.

Copyright (c) 2022
Marcelo Barros de Almeida <marcelobarrosalmeida@gmail.com>
Marden Fagundes <maf_cadastro-github@yahoo.com>
*/

#pragma once

#define DHC_MAX_BITS    16
#define DHC_TABLE_SIZE (DHC_MAX_BITS+1)

#include <stdbool.h>

typedef enum dhc_compress_mode_e
{
    DHC_COMPRESS_MODE_NONE = 0,
    DHC_COMPRESS_MODE_HUFFMAN_MAPPED,
} dhc_compress_mode_t;

//for RLE
typedef struct {
    uint8_t *data;  // To store the encoded values
    size_t size;   // Current size of the array
    size_t capacity; // Current capacity of the array
} DynamicArray_8;

typedef struct {
    uint16_t *data;  // To store the encoded values
    size_t size;   // Current size of the array
    size_t capacity; // Current capacity of the array
} DynamicArray_16;

bool dhc_file_write(char *file_name, uint8_t *data, uint32_t size_bits, dhc_compress_mode_t mode);

/*
@brief Calculate compress ratio of a vector of 16 bits signed integers (int16_t)
@param[in] samples sample data (must be a vector of int16_t)
@param[in] sample_size sample size (it is not size in bytes, it is size in int16_t samples)
@param[out] data_size_bits compressed output size, in bits (must be allocated before calling)
@param[in] map huffman mapping (if using any special huffman map), otherwise pass 0
@return reduction ratio (if <= 0.0 do not use compress, if > 0.0 you can use compress)
*/
float dhc_compress_evaluate(int16_t *samples, uint32_t sample_size, uint32_t *data_size_bits, uint8_t *map);

/*
@brief compress a vector of 16 bits signed integers (int16_t)
@param[out] data vector where compressed data will be storaged (must be allocated before calling)
@param[out] data_size_bits compressed output size, in bits (must be allocated before calling)
@param[in] samples sample data (must be a vector of int16_t)
@param[in] sample_size sample size (it is not size in bytes, it is size in int16_t samples)
@param[in] map huffman mapping (if using any special huffman map), otherwise pass 0
@return true in case of success or false if the operation fails
*/
bool dhc_compress(uint8_t *data, uint32_t *data_size_bits, int16_t *samples, uint32_t sample_size, uint8_t *map);

/*
@brief decompress a vector of bytes (uint8_t)
@param[out] samples output vector where decompressed samples will be storaged (must be allocated before calling, must be a vector of int16_t)
@param[out] sample_size output size, in bytes (must be allocated before calling, it is not size in bytes, it is size in int16_t samples)
@param[in] data compressed data (must be a vector of unt8_t)
@param[in] data_size_bits compressed data size, in bits
@param[in] map huffman mapping (if using any special huffman map), otherwise pass 0
@return true in case of success or false if the operation fails
*/
bool dhc_decompress(int16_t *samples, uint32_t *sample_size, uint8_t *data, uint32_t data_size_bits, uint8_t *map);

//Addition for application:

/*
 * Function for compressing the timestamp arrays
@param[out] data: data vector where compressed data will be stored (must be allocated before calling)
@param[out] data_size_bits: compressed output size, in bits (must be allocated before calling)
@param[in] samples: sample data (must be a vector of uint8_t)
@param[in] sample_size: sample size (it is not size in bytes, it is size in int8_t samples)
@return true in case of success or false if the operation fails
 */
bool compress_timestamp(int8_t *samples, int32_t sample_size, DynamicArray_8 *encoded_vals, DynamicArray_16 *encoded_count);

/*
 * For the dynamic array, clearing memory. Both for 16bit and 8bit
 */
void freeArray_16(DynamicArray_16 *array);

void freeArray_8(DynamicArray_8 *array);
