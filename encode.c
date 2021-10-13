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

/*void post_trav(Node *root, uint8_t *dump, uint32_t *byteindex) {
    //node_print(root);
    if (root->left == NULL && root->right == NULL) {
        dump[*byteindex] = 'L';
        *byteindex += 1;
        dump[*byteindex] = root->symbol;
        *byteindex += 1;
    } else {
        post_trav(root->left, dump, byteindex);
        post_trav(root->right, dump, byteindex);
        dump[*byteindex] = 'I';
        *byteindex += 1;
    }
    return;
}*/
void post_trav(Node *root, uint8_t *dump, uint32_t *byteindex) {
    if (root == NULL) {
        return;
    }
    if (root->left != NULL) {
        post_trav(root->left, dump, byteindex);
    }
    if (root->right != NULL) {
        post_trav(root->right, dump, byteindex);
    }
    if (root->left == NULL && root->right == NULL) {
        dump[*byteindex] = 'L';
        *byteindex += 1;
        dump[*byteindex] = root->symbol;
        *byteindex += 1;
    } else {
        dump[*byteindex] = 'I';
        *byteindex += 1;
    }
}

void print_stats(void) {
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
            fprintf(stderr, "Synopsis: This is a huffman encoder. Compresses a file using Huffman "
                            "coding algorithm.\nUsage: ./encode -i infile -o outfile (if encode is "
                            "in your current working directory\nOptions:\n\t-h: prints a help "
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

    // create histogram
    uint8_t buffer[BLOCK]; // buffer to hold infile
    uint32_t bytes; // keep track of bytes read in loop
    uint64_t hist[ALPHABET]; // histgram
    for (uint32_t i = 0; i < ALPHABET; i += 1) { // initialize hist to 0
        hist[i] = 0;
    }
    while ((bytes = read_bytes(infile, buffer, BLOCK)) > 0) {
        for (uint32_t i = 0; i < bytes; i += 1) {
            hist[buffer[i]] += 1;
            //printf("%c\n", buffer[i]);
        }
    }

    // increment count of element 0 and 255 as outlined in asgn6.pdf
    hist[0] += 1;
    hist[255] += 1;

    // construct huffman tree
    Node *root = build_tree(hist); // root stores root of huffman tree

    // construct code table
    Code table[ALPHABET]; // table of codes
    build_codes(root, table);

    // construct header
    Header header;
    header.magic = MAGIC;
    struct stat statbuf;
    fstat(infile, &statbuf);
    header.permissions = statbuf.st_mode;
    fchmod(outfile, statbuf.st_mode);
    uint32_t unique = 0; // holds number of unique symbols in file
    for (uint32_t i = 0; i < ALPHABET; i += 1) {
        if (hist[i] != 0) {
            unique += 1;
        }
    }
    header.tree_size = (3 * unique) - 1;
    if (infile == STDIN_FILENO) {
        header.file_size = bytes_read;
    } else {
        header.file_size = statbuf.st_size;
    }

    // write header to outfile
    write_bytes(outfile, (uint8_t *) &header, sizeof(Header));

    // post-order traversal to write tree to outfile
    uint8_t dump[MAX_TREE_SIZE]; // buffer to hold tree dump
    uint32_t byteindex = 0; // index of buffer to hold tree dump
    for (uint32_t j = 0; j < MAX_TREE_SIZE; j += 1) {
        dump[j] = 0;
    }
    post_trav(root, dump, &byteindex);
    write_bytes(outfile, dump, byteindex);

    // write infile to outfile using codes
    lseek(infile, 0, SEEK_SET);
    while ((bytes = read_bytes(infile, buffer, BLOCK)) > 0) {
        for (uint32_t i = 0; i < bytes; i += 1) {
            write_code(outfile, &table[buffer[i]]);
        }
    }

    flush_codes(outfile);
    close(infile);
    close(outfile);
    delete_tree(&root);
    if (verbose == true) {
        print_stats();
    }
    return 0;
}
