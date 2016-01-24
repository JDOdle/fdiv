//
//  main.c
//  SplitFiles
//
//  Created by Justin Odle on 1/15/16.
//  Copyright © 2016 Justin Odle. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

//actions
#define SPLIT 0
#define JOIN  1
#define CLEAR 2

//option values
#define SIZE "size"
#define PARTS "parts"
#define DEST "dest"
#define REM "rem"
#define VERBOSE "verbose"
#define HELP "help"

//option shifts
#define S 13 //Size
#define P 10 //Parts
#define D  7 //Destination

//option flags
#define R 0x40 //Remove
#define V 0x20 //Verbose
#define H 0x10 //Help

//parsing flags
#define S_VAL(bitmap) (bitmap >> S)
#define P_VAL(bitmap) (bitmap >> P) & 7
#define D_VAL(bitmap) (bitmap >> D) & 7
#define R_VAL(bitmap) (bitmap & R)
#define V_VAL(bitmap) (bitmap & V)
#define H_VAL(bitmap) (bitmap & H)
#define F_VAL(bitmap) (bitmap & 15)

uint16_t parse_options(uint8_t action, const char *optn[], uint8_t size);
uint8_t split_size(char *srcpath, char *filename, char *destpath, uint64_t size, bool rem, bool verbose);
uint8_t split_parts(char *srcpath, char *filename, char *destpath, uint8_t parts, bool rem, bool verbose);
uint8_t split(char *src, char *dest, uint8_t parts, uint64_t size, uint64_t total, bool verbose);
uint8_t join(char *srcpath, char *filename, char *destpath, bool rem, bool verbose);
uint8_t clear(char *filepath, uint8_t parts, bool verbose);

void help();
void help_split();
void help_join();
void help_clear();

int main(int argc, const char * argv[]) {
    
    uint16_t optns = 0;
    uint8_t action = SPLIT;
    char *filepath = NULL;
    char *srcpath = NULL;
    char *filename = NULL;
    char *destpath = NULL;
    uint8_t srclen = 0;
    uint8_t filelen = 0;
    uint8_t destlen = 0;
    
    uint64_t size = 0;
    uint8_t parts = 1;
    
    
    //default operation
    if (argc < 2) {
        help();
        return 0;
    }
    
    //get action
    if (strcmp(argv[1], "split") == 0)
        action = SPLIT;
    else if (strcmp(argv[1], "join") == 0)
        action = JOIN;
    else if (strcmp(argv[1], "clear") == 0)
        action = CLEAR;
    else {
        help();
        return 0;
    }
    
    //get options
    optns = parse_options(action, &argv[2], argc-2);
    
    filepath = malloc(strlen(argv[F_VAL(optns)])+1);
    strcpy(filepath, argv[F_VAL(optns)]);
    
    //get string lengths
    if (D_VAL(optns)) {
        if (argv[D_VAL(optns)][0] == '-')
            destlen = strlen(strchr(argv[D_VAL(optns)], 'd')+1);
        else
            destlen = strlen(argv[D_VAL(optns)]);
    }
    if (strchr(filepath, '/') == NULL) {
        srclen = 0;
        filelen = strlen(filepath);
    }
    else {
        srclen = strrchr(filepath, '/')-filepath+1;
        filelen = strlen(strrchr(filepath, '/')+1);
    }
    if (filelen == 0) {
        fprintf(stderr, "fdiv error: '%s' is a directory\n", filepath);
        return 1;
    }
    
    //get string values
    srcpath = malloc(srclen+1);
    strncpy(srcpath, filepath, srclen);
    filename = malloc(filelen+1);
    strcpy(filename, filepath+srclen);
    if (destlen == 0) {
        destpath = malloc(srclen+1);
        strcpy(destpath, srcpath);
    }
    else {
        destpath = malloc(destlen+1);
        if (argv[D_VAL(optns)][0] == '-')
            strncpy(destpath, strchr(argv[D_VAL(optns)], 'd')+1, destlen);
        else
            strncpy(destpath, argv[D_VAL(optns)], destlen);
    }
    
    if (action == SPLIT) {
        if (argc < 3 || H_VAL(optns)) {
            help_split();
            return 0;
        }
        if ((S_VAL(optns)) && (P_VAL(optns))) {
            fprintf(stderr, "fdiv error: only size or parts value can be specified\n");
            return 1;
        }
        else if (S_VAL(optns)) {
            if (argv[S_VAL(optns)][0] == '-') {
                size = atoi(strchr(argv[S_VAL(optns)], 's')+1);
            }
            else {
                size = atoi(argv[S_VAL(optns)]);
            }
            return split_size(srcpath, filename, destpath, size, R_VAL(optns), V_VAL(optns));
        }
        else if (P_VAL(optns)) {
            if (argv[P_VAL(optns)][0] == '-') {
                parts = atoi(strchr(argv[P_VAL(optns)], 'p')+1);
            }
            else {
                parts = atoi(argv[P_VAL(optns)]);
            }
            split_parts(srcpath, filename, destpath, parts, R_VAL(optns), V_VAL(optns));
        }
        else {
            return split_parts(srcpath, filename, destpath, parts, R_VAL(optns), V_VAL(optns));
        }
    }
    else if (action == JOIN) {
        if (argc < 3 || H_VAL(optns)) {
            help_join();
            return 0;
        }
        if ((S_VAL(optns)) || (P_VAL(optns))) {
            fprintf(stderr, "fdiv error: invalid option(s) -%s%s\n",
                    (S_VAL(optns))?"s":"", (P_VAL(optns))?"p":"");
            return 1;
        }
        return join(srcpath, filename, destpath, R_VAL(optns), V_VAL(optns));
    }
    else if (action == CLEAR) {
        if (argc < 3 || H_VAL(optns)) {
            help_clear();
            return 0;
        }
        if ((S_VAL(optns)) || (P_VAL(optns)) || (D_VAL(optns)) || (R_VAL(optns))) {
            fprintf(stderr, "fdiv error: invalid option(s) -%s%s%s%s\n",
                    (S_VAL(optns))?"s":"", (P_VAL(optns))?"p":"",
                    (D_VAL(optns))?"d":"", (R_VAL(optns))?"r":"");
            return 1;
        }
        return clear(filepath, 255, V_VAL(optns));
    }
    return 0;
}

uint16_t parse_options(uint8_t action, const char *optn[], uint8_t size) {
    /*Bitmap structure:
     *
     *bits 15-13:   size    flag/position
     *              0:      not specified
     *              1-6:    value is in next position
     *              7:      value is in the same position
     *bits 12-10:   parts   flag/position
     *              0:      not specified
     *              1-7:    position
     *bits   9-7:   dest    flag/position
     *              0:      not specified
     *              1-7:    position
     *bit      6:   rem     flag
     *bit      5:   verbose flag
     *bit      4:   help    flag
     *bits   3-0:   file    position
     */
    uint16_t optn_flags = 0;
    
    for (uint8_t i = 0; i < size; i++) {
        
        //Parse verbose option
        if (strnstr(optn[i], "--", 2) != NULL) {
            if (strcmp((optn[i]+2), SIZE) == 0) {
                optn_flags |= (i+1) << S;
            }
            else if (strncmp((optn[i]+2), SIZE, 4) == 0) {
                optn_flags |= 7 << S;
            }
            if (strcmp((optn[i]+2), PARTS) == 0) {
                optn_flags |= (i+1) << P;
            }
            else if (strncmp((optn[i]+2), PARTS, 5) == 0) {
                optn_flags |= 7 << P;
            }
            if (strcmp((optn[i]+2), DEST) == 0) {
                optn_flags |= (i+1) << D;
            }
            else if (strncmp((optn[i]+2), DEST, 4) == 0) {
                optn_flags |= 7 << D;
            }
            if (strcmp((optn[i]+2), REM) == 0)
                optn_flags |= R;
            if (strcmp((optn[i]+2), VERBOSE) == 0)
                optn_flags |= V;
            if (strcmp((optn[i]+2), HELP) == 0)
                optn_flags |= H;
        }
        //Parse succinct options
        else if (optn[i][0] == '-') {
            uint8_t noptn = strlen(optn[i]);
            uint8_t o = 1;
            for (; o < noptn-1; o++) {
                if (optn[i][o] == 's') {
                    optn_flags |= (i+2) << S;
                    break;
                }
                if (optn[i][o] == 'p') {
                    optn_flags |= (i+2) << P;
                    break;
                }
                if (optn[i][o] == 'd') {
                    optn_flags |= (i+2) << D;
                    break;
                }
                if (optn[i][o] == 'r') optn_flags |= R;
                if (optn[i][o] == 'v') optn_flags |= V;
                if (optn[i][o] == 'h') optn_flags |= H;
            }
            if (o == noptn-1) {
                if (optn[i][o] == 's') {
                    optn_flags |= (i+3) << S;
                    i++;
                }
                if (optn[i][o] == 'p') {
                    optn_flags |= (i+3) << P;
                    i++;
                }
                if (optn[i][o] == 'd') {
                    optn_flags |= (i+3) << D;
                    i++;
                }
                if (optn[i][o] == 'r') optn_flags |= R;
                if (optn[i][o] == 'v') optn_flags |= V;
                if (optn[i][o] == 'h') optn_flags |= H;
            }
        }
        //Parse filepath
        else {
            optn_flags |= i+2;
        }
    }
    return optn_flags;
}

uint8_t split_size(char *srcpath, char *filename, char *destpath, uint64_t size, bool rem, bool verbose) {
    uint8_t ret = 0;
    char *src = malloc(strlen(srcpath)+strlen(filename)+1);
    strcpy(src, srcpath);
    strcat(src, filename);
    
    FILE *src_file = fopen(src, "rb");
    fseek(src_file, 0, SEEK_END);
    uint64_t src_size = ftell(src_file);
    fclose(src_file);
    
    char *dest = malloc(strlen(destpath)+strlen(filename)+9);
    strcpy(dest, destpath);
    strcat(dest, filename);
    strcat(dest, ".part");
    
    uint8_t parts = ceil(((double)src_size)/((double)size));

    ret |= split(src, dest, parts, size, src_size, verbose);
    if (rem) ret |= clear(src, 0, verbose);
    
    free(src);
    free(dest);
    
    return ret;
}
uint8_t split_parts(char *srcpath, char *filename, char *destpath, uint8_t parts, bool rem, bool verbose) {
    uint8_t ret = 0;
    char *src = malloc(strlen(srcpath)+strlen(filename)+1);
    strcpy(src, srcpath);
    strcat(src, filename);
    
    FILE *src_file = fopen(src, "rb");
    if (src_file == NULL) {
        fprintf(stderr, "fdiv error: no such file '%s'\n", src);
        return 1;
    }
    fseek(src_file, 0, SEEK_END);
    uint64_t src_size = ftell(src_file);
    fclose(src_file);
    
    char *dest = malloc(strlen(destpath)+strlen(filename)+9);
    strcpy(dest, destpath);
    strcat(dest, filename);
    strcat(dest, ".part");
    
    uint64_t size = ceil(((double)src_size)/((double)parts));
    
    ret |= split(src, dest, parts, size, src_size, verbose);
    if (rem) ret |= clear(src, 0, verbose);
    
    free(src);
    free(dest);
    
    return ret;
}

uint8_t split(char *src, char *dest, uint8_t parts, uint64_t size, uint64_t total, bool verbose) {
    
    FILE *src_file = fopen(src, "rb");
    uint64_t src_size = 0;
    
    //allocate array of <num_parts> files
    FILE *file_part;
    
    //allocate string of <size> bytes
    uint8_t *part = malloc(size);
    
    uint16_t dest_len = strlen(dest);

    //write each file part
    for (uint16_t i = 0; i < parts; i++) {
        dest[dest_len+0] = '\0';
        dest[dest_len+1] = '\0';
        dest[dest_len+2] = '\0';
        sprintf(dest, "%s%u", dest, i);
        if (verbose) printf("making '%s'\n", dest);
        file_part = fopen(dest, "w");
        
        if (total < size)
            fread(part, 1, total, src_file);
        else
            total -= fread(part, 1, size, src_file);
        
        fwrite(part, 1, size, file_part);
        fclose(file_part);
    }
    
    fclose(src_file);
    
    return 0;
}

uint8_t join(char *srcpath, char* filename, char *destpath, bool rem, bool verbose) {
    
    
    char *src = malloc(strlen(srcpath)+strlen(filename)+9);
    strcpy(src, srcpath);
    strcat(src, filename);
    
    //check and prompt user if file already exists
    if( access( filename, F_OK ) != -1 ) {
        char force = 0;
        printf("The file '%s' already exists, would you like to join the files anyways? (y/N): ",
                filename);
        scanf("%c", &force);
        if (!(force == 'Y' || force == 'y'))
            return 0;
    }
    
    strcat(src, ".part");
    
    char *dest = malloc(strlen(destpath)+strlen(filename)+1);
    strcpy(dest, destpath);
    strcat(dest, filename);
    //open file with <filename> for writing
    FILE *whole = fopen(dest, "w");
    
    //for each file, <filename>.part<i> read into <part>
    //for each file write <part> into <whole>
    uint8_t i = 0;
    while (true) {
        //get filename for current part
        char *part_filename = malloc(strlen(filename)+8);
        sprintf(part_filename, "%s%u", src, i);
        
        //open part file for reading
        FILE *part_file = fopen(part_filename, "rb");
        if (part_file == NULL) {
            i--;
            break;
        }
        if (verbose) printf("joining '%s'\n", part_filename);
        
        //get size of the part file
        uint64_t part_size = 0;
        fseek(part_file, 0, SEEK_END);
        part_size = ftell(part_file);
        fseek(part_file, 0, SEEK_SET);
        
        //allocate array for part data
        uint8_t *part_data = malloc(part_size);
        fread(part_data, 1, part_size, part_file);
        fwrite(part_data, 1, part_size, whole);
        
        fclose(part_file);
        free(part_data);
        free(part_filename);
        i++;
    }
    fclose(whole);
    if (rem) return clear(src, i+1, verbose);
    return 0;
}

uint8_t clear(char *filepath, uint8_t parts, bool verbose) {
    if (parts == 0) remove(filepath);
    else if (parts < 0xFF) {
        for (uint8_t i = 0; i < parts; i++) {
            char *part = malloc(strlen(filepath)+9);
            sprintf(part, "%s%u", filepath, i);
            if (verbose) printf("removing '%s'\n", part);
            remove(part);
        }
    }
    else {
        for (uint8_t i = 0; i < parts; i++) {
            char *part = malloc(strlen(filepath)+9);
            sprintf(part, "%s.part%u", filepath, i);
            remove(part);
        }
    }
    
    return 0;
}

void help() {
    printf("fdiv: A command-line tool to distribute large files into\n");
    printf("      smaller parts and rejoin parts of a file.\n");
    printf("version: 1.0    author: Justin Odle\n\n");
    printf("usage: fdiv [action] [options] [file]\n\n");
    printf("Actions:\n");
    printf("   split                    Split a file into several parts\n");
    printf("   join                     Join part files into one file\n");
    printf("   clear                    Delete all part files for a file\n\n");
    printf("Options:\n");
    printf("   -s  or  --size           Specify Size (value in bytes)\n");
    printf("   -p  or  --parts          Specify number of Parts (max: 255)\n");
    printf("   -d  or  --dest           Specify path to the Destination folder\n");
    printf("   -r  or  --rem            Remove source files after split or join\n");
    printf("   -v  or  --verbose        Be Verbose when joining/splitting/clearing files, \n");
    printf("                            showing them as they are joined/split/cleared\n");
    printf("   -h  or  --help           Print Help (this message) and exit\n");
}

void help_split() {
    printf("usage: fdiv split [options] [file]\n\n");
    printf("Options:\n");
    printf("   -s  or  --size           Specify Size (value in bytes)\n");
    printf("   -p  or  --parts          Specify number of Parts (max: 255)\n");
    printf("   -d  or  --dest           Specify path to the Destination folder\n");
    printf("   -r  or  --rem            Remove source files after split\n");
    printf("   -v  or  --verbose        Be Verbose when splitting files, showing them as \n");
    printf("                            they are split\n");
    printf("   -h  or  --help           Print Help (this message) and exit\n");
}

void help_join() {
    printf("usage: fdiv join [options] [folder]\n\n");
    printf("Options:\n");
    printf("   -d  or  --dest           Specify path to the Destination folder\n");
    printf("   -r  or  --rem            Remove source files after join\n");
    printf("   -v  or  --verbose        Be Verbose when joining files, showing them as \n");
    printf("                            they are joined\n");
    printf("   -h  or  --help           Print Help (this message) and exit\n");
}

void help_clear() {
    printf("usage: fdiv clear [options] [file]\n\n");
    printf("Options:\n");
    printf("   -v  or  --verbose        Be Verbose when clearing files, showing them as \n");
    printf("                            they are cleared\n");
    printf("   -h  or  --help           Print Help (this message) and exit\n");
}















