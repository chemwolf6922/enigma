#ifndef __ENIGMA_H
#define __ENIGMA_H

#include <stdint.h>
#include <stdbool.h>

typedef struct{
    uint8_t key_map[26];
}plug_board_t;

typedef struct{
    uint8_t key_map[26];
}reflector_t;

typedef struct{
    uint8_t key_map[26];
    uint8_t key_map_reverse[26];
    uint8_t turn_over_key;
    uint8_t offset;
}rotor_t;

typedef struct{
    uint8_t key_A;
    uint8_t key_B;
}wire_t;

typedef struct{
    rotor_t rotors[5];
    plug_board_t plug_board;
    reflector_t reflector;
    int selected_rotor_num[3];
}enigma_t;

int init_enigma(enigma_t* enigma);
int set_enigma_key(char* rotor_positions,
                    char* plug_board_config);
int set_enigma_key_direct(enigma_t* enigma,
                            uint8_t* selected_rotors,
                            uint8_t* rotor_positions,
                            wire_t* plug_board_config,
                            int plug_board_config_len);
int enigma_encrypt_decrypt(enigma_t* enigma,char* content);
int enigma_encrypt_decrypt_direct(enigma_t* enigma,uint8_t* content_code,int len);

#endif