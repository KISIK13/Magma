
#include <iostream>;
#include <inttypes.h>;
using namespace std;
#define max_open_text_size 1024




void magma(char, uint8_t*, uint8_t*, uint8_t*, int);
uint64_t merge_8_blocks8bit_to_1_block64bit(uint8_t*, int);
void split_1_block64bit_to_2_blocks32bit(uint64_t, uint32_t*, uint32_t*);
void split_key256bit_to_8_keys32bit(uint32_t*, uint8_t*);
uint8_t split_1_block32bit_to_4_blocks8bit(uint32_t);
void enc(uint32_t*, uint32_t*, uint32_t*);
uint32_t merge_4_blocks8bit_to_1_block32bit(uint8_t*);






static unsigned char Pi[8][16] =
{
    {1, 7, 14, 13, 0, 5, 8, 3, 4, 15, 10, 6, 9, 12, 11, 2},
    {8, 14, 2, 5, 6, 9, 1, 12, 15, 4, 11, 0, 13, 10, 3, 7},
    {5, 13, 15, 6, 9, 2, 12, 10, 11, 7, 8, 1, 4, 3, 14, 0},
    {7, 15, 5, 10, 8, 1, 6, 13, 0, 9, 3, 14, 11, 4, 2, 12},
    {12, 8, 2, 1, 13, 4, 15, 6, 7, 0, 10, 5, 3, 14, 9, 11},
     {11, 3, 5, 8, 2, 15, 10, 13, 14, 1, 7, 4, 12, 9, 6, 0},
     {6, 8, 2, 3, 9, 10, 5, 12, 1, 14, 4, 7, 11, 13, 0, 15},
    {12, 4, 6, 2, 10, 5, 11, 9, 14, 8, 13, 7, 0, 3, 15, 1}  
};


uint32_t merge_4_blocks8bit_to_1_block32bit(uint8_t* blocks8bit)
{
    uint32_t block32bit_tmp = blocks8bit[3] | (blocks8bit[2] << 8) | (blocks8bit[1] << 16) | (blocks8bit[0] << 24);
    return block32bit_tmp;
}

uint8_t split_1_block32bit_to_4_blocks8bit(uint32_t block32bit2)
{
    
    uint8_t block8bit;
    block8bit = (uint8_t)block32bit2;
    return block8bit;
}

void enc(uint32_t * block32bit1, uint32_t * block32bit2,uint32_t * keys32bit)
{
    //k0,k1,k2,k3,k4,k5,k6,k7
    //k0,k1,k2,k3,k4,k5,k6,k7
    //k0,k1,k2,k3,k4,k5,k6,k7
    //k7,k6,k5,k4,k3,k2,k1,k0
    uint8_t block32bit_in_4_blocks8bit[4];
    uint8_t first_4bit;
    uint8_t second_4bit;

    
  

    for (int i = 0; i < 32; i++)
    {
        int j = 0;
        if (i < 24)
            j = i % 8;
        else
            j = 31 - i;
        uint32_t tmp = *block32bit2;
        *block32bit2 += keys32bit[j] % UINT32_MAX;
        for (int i = 0; i < 4; i++)
        {
            block32bit_in_4_blocks8bit[3-i] =
                split_1_block32bit_to_4_blocks8bit(*block32bit2);
            *block32bit2 >>= 8;
        }
        
        for (int i = 0; i < 4; i++)
        {
            first_4bit = (block32bit_in_4_blocks8bit[i] & 0xf0) >> 4;
            second_4bit = (block32bit_in_4_blocks8bit[i] & 0x0f);
            first_4bit = Pi[i * 2][first_4bit];
            second_4bit = Pi[i * 2 + 1][second_4bit];
            block32bit_in_4_blocks8bit[i] = (first_4bit << 4) | second_4bit;
        }


        *block32bit2 = merge_4_blocks8bit_to_1_block32bit(block32bit_in_4_blocks8bit);
        *block32bit2 = (*block32bit2 << 11) | (*block32bit2 >> 21);
        
        *block32bit2 = *block32bit2 ^ *block32bit1;

        *block32bit1 = tmp;
        if (i == 31)
        {
            uint32_t tmp1 = *block32bit2;
            *block32bit2 = *block32bit1;
            *block32bit1 = tmp1;
        }

        //printf("open text in round %d  -  %" PRIx32 "   ",i, *block32bit1);
        //printf("%" PRIx32 "\n", *block32bit2);
        
        
    }
   
   
    
}


void split_key256bit_to_8_keys32bit(uint32_t* keys32bit, uint8_t* key256bit)
{
    int j = 0;
    for (int i = 0; i < 8; i++)
    {
       
        keys32bit[i] = key256bit[j+3] | (key256bit[j + 2] << 8) | (key256bit[j + 1] << 16) | (key256bit[j] << 24);
        j += 4;
    }
    for (int i = 0 ; i < 8; i ++)
        printf("key %d - %x\n", i, keys32bit[i]);
    cout << endl;
}

void split_1_block64bit_to_2_blocks32bit(uint64_t block64bit, uint32_t* block32bit1, uint32_t* block32bit2)
{
    *block32bit1 = (uint32_t)block64bit;
    *block32bit2 = (uint64_t)(block64bit >> 32);
}

uint64_t merge_8_blocks8bit_to_1_block64bit(uint8_t * open_text, int index)
{
    uint64_t open_text_64bit = 0;
  
    for (int i = index; i < index + 8; i++)
    {
       open_text_64bit = (open_text_64bit << 8) | open_text[i];
    }
    //printf("%" PRIx64 "\n", open_text_64bit);
    return open_text_64bit;

}

void magma(char mode, uint8_t* result, uint8_t* open_text, uint8_t* key256bit, int len)
{
    if ((len % 8) !=0)
        len = len + 8 - (len % 8);
    uint32_t keys32bit[8];
    split_key256bit_to_8_keys32bit(keys32bit, key256bit);
    uint64_t open_text_64bit;
    uint32_t first_part_of_open_text_32bit, second_part_of_open_text_32bit;

    for (int i = 0; i < len; i+=8)
    {
        open_text_64bit = merge_8_blocks8bit_to_1_block64bit(open_text, i);
        split_1_block64bit_to_2_blocks32bit(
            open_text_64bit, 
            &second_part_of_open_text_32bit,
            &first_part_of_open_text_32bit
        );
        
        enc(&first_part_of_open_text_32bit,
            &second_part_of_open_text_32bit,
            keys32bit
        );
        
        
       
        printf("%" PRIx32 "%" PRIx32, first_part_of_open_text_32bit, second_part_of_open_text_32bit);
    }
   
    
}


int main()
{

    uint8_t open_text[max_open_text_size], result[max_open_text_size], key256bit[32];
    int open_text_size = 0;
    //while (true)
    //{
     //   uint8_t symb = getchar();
      //  if (symb == '\n')// || open_text_size - 1 > max_open_text_size)
      //      break;
       // open_text[open_text_size] = symb;
        //open_text_size++;
    //}
    open_text_size = 8;
    //открытый текст как в госте
    open_text[0] = 254;
    open_text[1] = 220;
    open_text[2] = 186;
    open_text[3] = 152;
    open_text[4] = 118;
    open_text[5] = 84;
    open_text[6] = 50;
    open_text[7] = 16;
    cout << "open text - ";
    for (int i = 0; i < open_text_size; i++)
        printf("%x", open_text[i]);
    cout << endl;
    open_text[open_text_size] = '\0';
    

    //ключ как в госте
    key256bit[0] = 255;
    key256bit[1] = 238;
    key256bit[2] = 221;
    key256bit[3] = 204;
    key256bit[4] = 187;
    key256bit[5] = 170;
    key256bit[6] = 153;
    key256bit[7] = 136;
    key256bit[8] = 119;
    key256bit[9] = 102;
    key256bit[10] = 85;
    key256bit[11] = 68;
    key256bit[12] = 51;
    key256bit[13] = 34;
    key256bit[14] = 17;
    key256bit[15] = 0;
    key256bit[16] = 240;
    key256bit[17] = 241;
    key256bit[18] = 242;
    key256bit[19] = 243;
    key256bit[20] = 244;
    key256bit[21] = 245;
    key256bit[22] = 246;
    key256bit[23] = 247;
    key256bit[24] = 248;
    key256bit[25] = 249;
    key256bit[26] = 250;
    key256bit[27] = 251;
    key256bit[28] = 252;
    key256bit[29] = 253;
    key256bit[30] = 254;
    key256bit[31] = 255;
    
    cout << "key - ";
    for (int i = 0; i < 32; i++)
        printf("%x", key256bit[i]);
    cout << endl;
    char mode = 'E';
    magma(mode, result, open_text, key256bit, open_text_size);
}
