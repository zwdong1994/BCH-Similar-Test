#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include "bch.h"

#define CONFIG_M 8
#define CONFIG_T 13
#define CHUNK_SIZE 4096
#define CODE_LENGTH ((CONFIG_M * CONFIG_T)/8 + ((CONFIG_M * CONFIG_T)%8>0?1:0))

struct bch_control *bch;
char source_string[CHUNK_SIZE + 1];
char src_bch_result[2 * CODE_LENGTH + 1];

void usage(){
    std::cout << "-a: The selected modify bytes number."<< std::endl;
    std::cout << "-b: The distance between different modified bytes."<< std::endl;
}

void init(){
    bch = init_bch(CONFIG_M, CONFIG_T, 0);
}

void get_randomstr(char *dst, int length){
    int i;
    for(i = 0; i < length; i++){
        dst[i] = 'a' + (rand() % 26);
    }
    dst[length] = '\0';
}

void ByteToHexStr(const unsigned char *source, char *dest, int sourceLen) {
    short i;
    unsigned char highByte, lowByte;

    for (i = 0; i < sourceLen; i++)
    {
        highByte = source[i] >> 4;
        lowByte = source[i] & 0x0f ;

        highByte += 0x30;

        if (highByte > 0x39)
            dest[i * 2] = highByte + 0x07;
        else
            dest[i * 2] = highByte;

        lowByte += 0x30;
        if (lowByte > 0x39)
            dest[i * 2 + 1] = lowByte + 0x07;
        else
            dest[i * 2 + 1] = lowByte;
    }
    return ;
}

int same_count(char *str1, char *str2, int length){
    int i;
    int count = 0;
    for(i=0; i < length; i++){
        count += (str1[i] == str2[i])?1:0;
    }
    return count;
}

double dst_compare(char *src, int length, int change_num, int change_distance ){
    uint8_t hv[CODE_LENGTH + 1];
    uint8_t dst[length + 1];
    char bch_result[2 * CODE_LENGTH + 1];
    int i, j;
    int count = 0;
    double aver_same = 0.0;
    for(i = 0; i < length - ((change_num - 1) * change_distance); i++) {
        memcpy(dst, src, length);
        for (j = 0; j < change_num; j++) {
            dst[i + j * change_distance] = src[i + j] + 1;
        }

        memset(hv, 0, CODE_LENGTH + 1);
        memset(bch_result, 0, 2 * CODE_LENGTH + 1);
        encode_bch(bch, dst, length, hv);
        ByteToHexStr(hv, bch_result, CODE_LENGTH);
        count += same_count(bch_result, src_bch_result, 2 * CODE_LENGTH);
    }

    aver_same = 1.0 * count / (length - ((change_num - 1) * change_distance));
    return aver_same;
}

int main(int argc, char *argv[]){
    int change_num = 1;
    int change_distance = 1;
    int num_flag = 0, distance_flag = 0;
    int j;
    double aver;
    int ch;
    uint8_t hv[CODE_LENGTH + 1];
    while((ch = getopt(argc, argv, "a:b:") ) != -1){
        //std::cout << ch << std::endl;
        switch (ch) {
            case 'a':
                num_flag = 1;
                change_num = atoi(optarg);
                std::cout << "The selected modify bytes number is: " << change_num << std::endl;
                break;
            case 'b':
                distance_flag = 1;
                change_distance = atoi(optarg);
                std::cout << "The distance between different modified bytes is: " << change_distance << std::endl;
                break;
            default:
                usage();
                exit(-1);
        }
    }
    if(num_flag == 0){
        std::cout << "The selected modify bytes number is: " << change_num << std::endl;
    }
    if(distance_flag == 0){
        std::cout << "The distance between different modified bytes is: " << change_distance << std::endl;
    }
    std::cout << "Please input the enter to start test! " << std::endl;
    getchar();
    init();

    get_randomstr(source_string, CHUNK_SIZE);

    memset(hv, 0, CODE_LENGTH + 1);
    memset(src_bch_result, 0, 2 * CODE_LENGTH + 1);
    encode_bch(bch, source_string, CHUNK_SIZE, hv);
    ByteToHexStr(hv, src_bch_result, CODE_LENGTH);

    for(;change_num > 0; change_num--){

        for(j = change_distance; j >= 1; j--){

            std::cout << "The change number is: "
                      << change_num
                      << std::endl
                      << "The change distance is: "
                      << j
                      << std::endl;

            aver = dst_compare(source_string, CHUNK_SIZE, change_num, j);
            std::cout << "The average same string length is: "
                      << aver
                      << std::endl;
        }
    }
    return 0;
}
