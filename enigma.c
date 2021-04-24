#include "enigma.h"
#include <string.h>
#include <stdio.h>
#include <malloc.h>

uint32_t offset_lu[26 * 26];

int init_rotor(rotor_t *rotor, char *wiring, char turn_over_key)
{
    if (strlen(wiring) != 26)
    {
        return -1;
    }
    rotor->buf_offset = 0;
    rotor->turn_over_key_buf = (turn_over_key - 'A') * 26;
    for (int i = 0; i != 26; i++)
    {
        rotor->key_map[i] = wiring[i] - 'A';
        rotor->key_map_reverse[rotor->key_map[i]] = i;
    }
    // offset
    for (int i = 0; i != 26; i++)
    {
        // input
        for (int j = 0; j != 26; j++)
        {
            rotor->key_map_buf[i * 26 + j] = (rotor->key_map[(j + i) % 26] + 26 - i) % 26;
            rotor->key_map_reverse_buf[i * 26 + j] = (rotor->key_map_reverse[(j + i) % 26] + 26 - i) % 26;
        }
    }
    return 0;
}

int init_plug_board(plug_board_t *plug_board)
{
    for (int i = 0; i != 26; i++)
    {
        plug_board->key_map[i] = i;
    }
    return 0;
}

int init_reflector(reflector_t *reflector, char *mapping)
{
    if (strlen(mapping) != 26)
    {
        return -1;
    }
    for (int i = 0; i < 26; i++)
    {
        reflector->key_map[i] = (uint8_t)(mapping[i] - 'A');
    }
    return 0;
}

int init_offset_lu(void)
{
    memset(offset_lu, 0, sizeof(offset_lu));
    for (int i = 0; i != 26; i++)
    {
        offset_lu[i * 26] = (i * 26 + 26) % 676;
    }
}

int init_enigma(enigma_t *enigma)
{
    char *rotor_wirings[5] = {
        "EKMFLGDQVZNTOWYHXUSPAIBRCJ",
        "AJDKSIRUXBLHWTMCQGZNPYFVOE",
        "BDFHJLCPRTXVZNYEIWGAKMUSQO",
        "ESOVPZJAYQUIRHXLNFTGKDCMWB",
        "VZBRGITYUPSDNHLXAWMJQOFECK",
    };
    char rotor_turn_overs[5] = {'R', 'F', 'W', 'K', 'A'};
    char reflector_wiring[] = "YRUHQSLDPXNGOKMIEBFZCWVJAT";
    for (int i = 0; i != 5; i++)
    {
        init_rotor(&enigma->rotors[i], rotor_wirings[i], rotor_turn_overs[i]);
    }
    init_plug_board(&enigma->plug_board);
    init_reflector(&enigma->reflector, reflector_wiring);
    init_offset_lu();
}

int set_plug_board(plug_board_t *plug_board, wire_t *wires, int num_wire)
{
    // check wires
    uint8_t wire_counts[26];
    memset(wire_counts, 0, sizeof(wire_counts));
    for (int i = 0; i != num_wire; i++)
    {
        if (wires[i].key_A > 'Z' - 'A')
        {
            return -1;
        }
        if (wires[i].key_B > 'Z' - 'A')
        {
            return -1;
        }
        wire_counts[wires[i].key_A]++;
        wire_counts[wires[i].key_B]++;
    }
    for (int i = 0; i != 26; i++)
    {
        if (wire_counts[i] > 1)
        {
            return -1;
        }
    }
    // set key map
    for (int i = 0; i != 26; i++)
    {
        plug_board->key_map[i] = i;
    }
    for (int i = 0; i != num_wire; i++)
    {
        plug_board->key_map[wires[i].key_A] = wires[i].key_B;
        plug_board->key_map[wires[i].key_B] = wires[i].key_A;
    }
    return 0;
}

int set_enigma_key_direct(enigma_t *enigma,
                          uint8_t *selected_rotors,
                          uint8_t *rotor_positions,
                          wire_t *plug_board_config,
                          int plug_board_config_len)
{
    for (int i = 0; i != 3; i++)
    {
        if (selected_rotors[i] >= sizeof((*enigma).rotors) / sizeof(rotor_t))
        {
            return -1;
        }
        enigma->selected_rotor_num[i] = selected_rotors[i];
        if (rotor_positions[i] >= 26)
        {
            return -1;
        }
        enigma->rotors[enigma->selected_rotor_num[i]].buf_offset = rotor_positions[i] * 26;
    }
    int ret = set_plug_board(&enigma->plug_board, plug_board_config, plug_board_config_len);
    return ret;
}

int set_enigma_key_from_save(enigma_t* enigma,enigma_key_t* key)
{
    for (int i = 0; i != 3; i++)
    {
        if (key->rotor_num[i] >= sizeof((*enigma).rotors) / sizeof(rotor_t))
        {
            return -1;
        }
        enigma->selected_rotor_num[i] = key->rotor_num[i];
        if (key->rotor_offset[i] >= 26)
        {
            return -1;
        }
        enigma->rotors[enigma->selected_rotor_num[i]].buf_offset = key->rotor_offset[i] * 26;
    }
    memcpy(enigma->plug_board.key_map,key->plug_board.key_map,26);
    return 0;
}

/*
    get the current setting (offset will change if encryption happend)
*/
int get_enigma_key(enigma_t* enigma,enigma_key_t* key)
{
    memcpy(key->plug_board.key_map,enigma->plug_board.key_map,26);
    for(int i=0;i!=3;i++)
    {
        key->rotor_num[i] = enigma->selected_rotor_num[i];
        key->rotor_offset[i] = enigma->rotors[enigma->selected_rotor_num[i]].buf_offset / 26;
    }
    return 0;
}

int set_enigma_key(enigma_t *enigma,
                   char *selected_rotors,
                   char *rotor_positions,
                   char *plug_board_config)
{
    if (selected_rotors == NULL || rotor_positions == NULL || plug_board_config == NULL)
    {
        return -1;
    }
    if (strlen(selected_rotors) != 3 || strlen(rotor_positions) != 3)
    {
        return -1;
    }
    if (((strlen(plug_board_config) + 1) % 3) != 0 && strlen(plug_board_config) != 0)
    {
        return -1;
    }
    uint8_t rotor_nums[3];
    for (int i = 0; i != 3; i++)
    {
        rotor_nums[i] = selected_rotors[i] - '1'; // '1' -> 0
    }
    uint8_t rotor_pos[3];
    for (int i = 0; i != 3; i++)
    {
        rotor_pos[i] = rotor_positions[i] - 'A'; // 'A' -> 0
    }
    int num_of_wires = (strlen(plug_board_config) + 1) / 3;
    int ret = 0;
    if (num_of_wires > 0)
    {
        wire_t *wires = (wire_t *)malloc(sizeof(wire_t) * num_of_wires);
        if (wires == NULL)
        {
            return -1;
        }
        for (int i = 0; i != num_of_wires; i++)
        {
            wires[i].key_A = plug_board_config[i * 3] - 'A';
            wires[i].key_B = plug_board_config[i * 3 + 1] - 'A';
        }
        ret = set_enigma_key_direct(enigma, rotor_nums, rotor_pos, wires, num_of_wires);
        free(wires);
    }
    else
    {
        ret = set_enigma_key_direct(enigma, rotor_nums, rotor_pos, NULL, 0);
    }
    return ret;
}

void enigma_encrypt_decrypt_direct(enigma_t *enigma, uint8_t *content_code, int len)
{
    rotor_t *rotor[3];
    // select rotors
    for (int i = 0; i != 3; i++)
    {
        rotor[i] = &enigma->rotors[enigma->selected_rotor_num[i]];
    }
    for (int i = 0; i != len; i++)
    {
        // spin rotors, start from the right most one
        rotor[2]->buf_offset = offset_lu[rotor[2]->buf_offset];
        if (rotor[2]->buf_offset == rotor[2]->turn_over_key_buf)
        {
            rotor[1]->buf_offset = offset_lu[rotor[1]->buf_offset];
            if (rotor[1]->buf_offset == rotor[1]->turn_over_key_buf)
            {
                rotor[0]->buf_offset = offset_lu[rotor[0]->buf_offset];
            }
        }
        // load value
        uint8_t src = content_code[i];
        // plug board
        src = enigma->plug_board.key_map[src];
        // rotors, start from the right most one
        src = rotor[2]->key_map_buf[src + rotor[2]->buf_offset];
        src = rotor[1]->key_map_buf[src + rotor[1]->buf_offset];
        src = rotor[0]->key_map_buf[src + rotor[0]->buf_offset];
        // reflector
        src = enigma->reflector.key_map[src];
        // reverse rotors, start form the left most one
        src = rotor[0]->key_map_reverse_buf[src + rotor[0]->buf_offset];
        src = rotor[1]->key_map_reverse_buf[src + rotor[1]->buf_offset];
        src = rotor[2]->key_map_reverse_buf[src + rotor[2]->buf_offset];
        // plug board
        src = enigma->plug_board.key_map[src];
        // store value
        content_code[i] = src;
    }
}

int enigma_encrypt_decrypt(enigma_t *enigma, char *content)
{
    int len = strlen(content);
    for (int i = 0; i != len; i++)
    {
        content[i] -= 'A';
        if (content[i] >= 26)
        {
            return -1;
        }
    }
    enigma_encrypt_decrypt_direct(enigma, (uint8_t *)content, len);
    for (int i = 0; i != len; i++)
    {
        content[i] += 'A';
    }
    return 0;
}