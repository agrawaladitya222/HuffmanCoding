#include "code.h"
#include "defines.h"
#include "header.h"
#include "huffman.h"
#include "io.h"
#include "node.h"

#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define OPTIONS "hi:o:v"

void print_statistics(void) {
    fprintf(stderr, "Uncompressed file size: %" PRIu64 "\n", bytes_read / 2);
    fprintf(stderr, "Compressed file size: %" PRIu64 "\n", bytes_written);
    double space = (double) bytes_written / (bytes_read / 2.0);
    space = 100.0 * (1.0 - space);
    fprintf(stderr, "Space saving: %5.2f%%\n", space);
}

int main(int argc, char **argv) {
    int opt = 0;
    int infile = STDIN_FILENO;
    int outfile = STDOUT_FILENO;
    bool verbose = false;
    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'h':
            // print help message
            fprintf(stderr,
                "Synopsis: This is a huffman decoder. Decompresses a file using Huffman "
                "coding algorithm.\nUsage: ./decode -i infile -o outfile (if decode is in "
                "your current working directory\nOptions:\n\t-h: prints a help "
                "message\n\t-i infile: specifies path to input file\n\t-o outfile: "
                "specifies path to outfile\n\t-v: prints out compression statistics\n");
            return 0;
        case 'i':
            infile = open(optarg, O_RDONLY);
            if (infile == -1) {
                fprintf(stderr, "Error, invalid file.\n");
                return 1;
            }
            break;
        case 'o': outfile = open(optarg, O_WRONLY | O_CREAT); break;
        case 'v':
            //print stats
            verbose = true;
            break;
        default: fprintf(stderr, "Error, invalid command-line option.\n"); return 1;
        }
    }

    // read header and verify magic number
    Header header;
    read_bytes(infile, (uint8_t *) &header, sizeof(Header));
    if (header.magic != MAGIC) {
        fprintf(stderr, "Error, invalid magic number: %" PRIu32 "\n", header.magic);
        return 1;
    }

    // set permissions
    struct stat statbuf;
    fstat(infile, &statbuf);
    header.permissions = statbuf.st_mode;
    fchmod(outfile, statbuf.st_mode);

    // read in tree dump and rebuild tree
    uint8_t dump[header.tree_size];
    read_bytes(infile, dump, header.tree_size);
    Node *root = rebuild_tree(header.tree_size, dump);

    // read infile by bit
    uint8_t bit;
    Node *curr = root;
    uint8_t outputbuf[header.file_size];
    uint64_t bufindex = 0;
    while (read_bit(infile, &bit) && bufindex < header.file_size) {
        if (bit == 0) {
            curr = curr->left;
        } else {
            curr = curr->right;
        }

        if (curr->left == NULL && curr->right == NULL) {
            outputbuf[bufindex] = curr->symbol;
            bufindex += 1;
            curr = root;
        }
    }

    write_bytes(outfile, outputbuf, bufindex);

    close(infile);
    close(outfile);
    delete_tree(&root);
    if (verbose == true) {
        print_statistics();
    }
    return 0;
}
