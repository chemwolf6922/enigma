#include "enigma.h"
#include <stdio.h>

int main(int argc,void* argv)
{
    enigma_t enigma;
    init_enigma(&enigma);
    uint8_t selected_rotors[3] = {0,1,2};
    uint8_t rotor_positions[3] = {0,0,0};
    wire_t wires[2];
    wires[0].key_A = 0;
    wires[0].key_B = 1;
    wires[1].key_A = 2;
    wires[1].key_B = 3;
    set_enigma_key_direct(&enigma,selected_rotors,
                        rotor_positions,wires,2);
    char test_phrase[] = "AAAAAAAAAA";
    printf("%s\n",test_phrase);
    enigma_encrypt_decrypt(&enigma,test_phrase);
    printf("%s\n",test_phrase);
    set_enigma_key_direct(&enigma,selected_rotors,
                        rotor_positions,wires,2);
    enigma_encrypt_decrypt(&enigma,test_phrase);
    printf("%s\n",test_phrase);
    return 0;
}