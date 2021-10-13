#include "io.h"

#include "code.h"
#include "defines.h"

#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

static uint8_t bitbuf[BLOCK]; // global buffer of bits
static uint32_t bitindex = 0; // global index of buffer of bits
uint64_t bytes_read = 0;
uint64_t bytes_written = 0;
// helper function for getting bits
uint8_t get_bit(Code *c, uint32_t i) {
    /*uint8_t result = (1 << (i % 8));
    result &= c->bits[i / 8];
    result = (result >> (i % 8));*/
    uint8_t result;
    result = (c->bits[i] >> (i % 8)) & 1;
    return result;
}

int read_bytes(int infile, uint8_t *buf, int nbytes) {
    int bytes = 0;
    int total = 0;
    do {
        bytes = read(infile, &buf[total], nbytes - total);
        if (bytes == 0 || bytes == -1) {
            break;
        }
        total += bytes;
    } while (total != nbytes && bytes > 0);

    bytes_read += total;
    return total;
}

int write_bytes(int outfile, uint8_t *buf, int nbytes) {
    int bytes = 0; // holds number of bytes wriiten
    int total = 0;

    while (total != nbytes) {
        bytes = write(outfile, &buf[total], nbytes - total);
        if (bytes == 0 || bytes == -1) {
            break;
        }
        total += bytes;
    }

    bytes_written += total;
    return total;
}

bool read_bit(int infile, uint8_t *bit) { // decode use
    // pseudo code from Eugene's section
    // if buffer is empty, fill it
    uint32_t bread; // holds number of bits read
    static uint32_t lastbit = 0;
    if (bitindex == 0) {
        bread = read_bytes(infile, bitbuf, BLOCK);
        if (bread < BLOCK) {
            lastbit = bread * 8 + 1; // index of last bit read + 2
        }
    }
    // pass back the bit
    *bit = (((1 << (bitindex % 8)) & bitbuf[bitindex / 8]) >> (bitindex % 8));
    bitindex += 1;
    if (bitindex == 8 * BLOCK) {
        bitindex = 0;
    }
    // return if a valid bit was returned
    if (bitindex == lastbit
        && lastbit != 0) { // "lastbit!=0" bc lastbit always 0 unless less than BLOCK
        return false; // bytes are read, which is only case when false could be
    } else { // returned
        return true;
    }
}

void write_code(int outfile, Code *c) { // encode use
    // pseudo code from Eugene's section
    // Buffer each bit of the Code
    for (uint32_t i = 0; i < code_size(c); i += 1) {
        if (get_bit(c, i) == 1) {
            bitbuf[bitindex / 8] |= (1 << (bitindex % 8)); // set bit at bitindex in bitbuf
        } else {
            bitbuf[bitindex / 8]
                &= ~(1 << (bitindex % 8)); // clr (set to 0) bit at bitindex in bitbuf
        }
        bitindex += 1;
        // if buffer is full, write buffer out
        if (bitindex == 8 * BLOCK) {
            write_bytes(outfile, bitbuf, BLOCK);
            bitindex = 0;
        }
    }
}

void flush_codes(int outfile) { // encode use
    if (bitindex > 0) {
        uint32_t bytes;
        if (bitindex % 8 == 0) {
            bytes = bitindex / 8;
        } else {
            bytes = bitindex / 8 + 1;
            // 0-out the remaining bits of the last byte
            for (uint32_t i = bitindex; i < 8 * bytes; i += 1) {
                bitbuf[i / 8] &= ~(1 << (i % 8));
            }
        }

        write_bytes(outfile, bitbuf, bytes);
    }
    // clear buffer
    bitindex = 0;
    return;
}
