#include "enigma.h"
#include <string.h>
#include <stdio.h>

int init_rotor(rotor_t* rotor,char* wiring,char turn_over_key)
{
    if(strlen(wiring)!=26)
    {
        return -1;
    }
    rotor->offset = 0;
    rotor->turn_over_key = turn_over_key - 'A';
    for(int i=0;i!=26;i++)
    {
        rotor->key_map[i] = wiring[i] - 'A';
    }
    return 0;
}

int init_plug_board(plug_board_t* plug_board)
{
    for(int i=0;i!=26;i++)
    {
        plug_board->key_map[i] = i;
    }
    return 0;
}

int init_reflector(reflector_t* reflector,char* mapping)
{
    if(strlen(mapping)!=26)
    {
        return -1;
    }
    for(int i=0;i<26;i++)
    {
        reflector->key_map[i] = (uint8_t)(mapping[i] - 'A');
    }
    return 0;
}

int init_enigma(enigma_t* enigma)
{
    char* rotor_wirings[5] = {
        "EKMFLGDQVZNTOWYHXUSPAIBRCJ",
        "AJDKSIRUXBLHWTMCQGZNPYFVOE",
        "BDFHJLCPRTXVZNYEIWGAKMUSQO",
        "ESOVPZJAYQUIRHXLNFTGKDCMWB",
        "VZBRGITYUPSDNHLXAWMJQOFECK",
    };
    char rotor_turn_overs[5] = {'Q','E','V','J','Z'};
    char reflector_wiring[] = "YRUHQSLDPXNGOKMIEBFZCWVJAT";
    for(int i=0;i!=5;i++)
    {
        init_rotor(&enigma->rotors[i],rotor_wirings[i],rotor_turn_overs[i]);   
    }
    init_plug_board(&enigma->plug_board);
    init_reflector(&enigma->reflector,reflector_wiring);
}

int set_plug_board(plug_board_t* plug_board,wire_t** wires,int num_wire)
{
    // check wires
    uint8_t wire_counts[26];
    memset(wire_counts,0,sizeof(wire_counts));
    for(int i=0;i!=num_wire;i++)
    {   
        if(wires[i]->key_A > 'Z' - 'A')
        {
            return -1;
        }
        if(wires[i]->key_B > 'Z' - 'A')
        {
            return -1;
        }
        wire_counts[wires[i]->key_A]++;
        wire_counts[wires[i]->key_B]++;
    }
    for(int i=0;i!=26;i++)
    {
        if(wire_counts[i] > 1)
        {
            return -1;
        }
    }
    // set key map
    for(int i=0;i!=26;i++)
    {
        plug_board->key_map[i] = i;
    }
    for(int i=0;i!=num_wire;i++)
    {
        plug_board->key_map[wires[i]->key_A] = wires[i]->key_B;
        plug_board->key_map[wires[i]->key_B] = wires[i]->key_A;
    }
    return 0;
}