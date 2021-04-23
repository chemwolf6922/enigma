#include "enigma.h"
#include <string.h>
#include <stdio.h>

// #define SHOW_STEPS

void dump_num(void *src, int len)
{
    for (int i = 0; i != len; i++)
    {
        printf("%02d ", *(uint8_t *)(src + i));
    }
    printf("\n");
}

int init_rotor(rotor_t *rotor, char *wiring, char turn_over_key)
{
    if (strlen(wiring) != 26)
    {
        return -1;
    }
    rotor->offset = 0;
    rotor->turn_over_key = turn_over_key - 'A';
    for (int i = 0; i != 26; i++)
    {
        rotor->key_map[i] = wiring[i] - 'A';
        rotor->key_map_reverse[rotor->key_map[i]] = i;
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
        enigma->rotors[enigma->selected_rotor_num[i]].offset = rotor_positions[i];
    }
    int ret = set_plug_board(&enigma->plug_board, plug_board_config, plug_board_config_len);
    return ret;
}

int enigma_encrypt_decrypt_direct(enigma_t *enigma, uint8_t *content_code, int len)
{
    rotor_t *rotor[3];
    for (int i = 0; i != 3; i++)
    {
        rotor[i] = &enigma->rotors[enigma->selected_rotor_num[i]];
    }
    for (int i = 0; i != len; i++)
    {
        // spin rotors, start from the right most one (the third one)
        for (int j = 2; j >= 0; j--)
        {
            rotor[j]->offset = (rotor[j]->offset + 1) % 26;
            if (rotor[j]->offset != rotor[j]->turn_over_key)
            {
                break;
            }
        }
        // calculate cipher text
        uint8_t src = content_code[i];
#ifdef SHOW_STEPS
        int scramble_step = 0;
        printf("%d: %02d %c\n", scramble_step++, src, (char)(src + 'A'));
#endif
        if (src > 26)
        {
            return -1;
        }
        src = enigma->plug_board.key_map[src];
#ifdef SHOW_STEPS
        printf("%d: %02d %c\n", scramble_step++, src, (char)(src + 'A'));
#endif
        // rotors, start from the right most one
        for (int j = 2; j >= 0; j--)
        {
            src = (src + rotor[j]->offset) % 26;
            src = rotor[j]->key_map[src];
            src = (src + 26 - rotor[j]->offset) % 26;
#ifdef SHOW_STEPS
            printf("%d: %02d %c\n", scramble_step++, src, (char)(src + 'A'));
#endif
        }
        src = enigma->reflector.key_map[src];
        for (int j = 0; j < 3; j++)
        {
            src = (src + rotor[j]->offset) % 26;
            src = rotor[j]->key_map_reverse[src];
            src = (src + 26 - rotor[j]->offset) % 26;
#ifdef SHOW_STEPS
            printf("%d: %02d %c\n", scramble_step++, src, (char)(src + 'A'));
#endif
        }
        src = enigma->plug_board.key_map[src];
#ifdef SHOW_STEPS
        printf("%d: %02d %c\n", scramble_step++, src, (char)(src + 'A'));
#endif
        content_code[i] = src;
    }
    return 0;
}

int enigma_encrypt_decrypt(enigma_t *enigma, char *content)
{
    int len = strlen(content);
    for (int i = 0; i != len; i++)
    {
        content[i] -= 'A';
    }
    int ret = enigma_encrypt_decrypt_direct(enigma, (uint8_t *)content, len);
    for (int i = 0; i != len; i++)
    {
        content[i] += 'A';
    }
    return ret;
}