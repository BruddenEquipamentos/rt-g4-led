#include "d_digits.h"

const digit_t dm_space = {
    .cols = 0,
    .data = {
        0b0,
        0b0,
        0b0,
        0b0,
        0b0
    }
};

const digit_t dm_percentage = {
    .cols=5,
    .data={
        0b11001,
        0b10010,
        0b00100,
        0b01001,
        0b10011,
    }
};

const digit_t dm_hyphen = {
    .cols=3,
    .data={
        0b000,
        0b000,
        0b111,
        0b000,
        0b000,
    }
};

const digit_t dm_slash = {
    .cols=3,
    .data={
        0b000,
        0b001,
        0b010,
        0b100,
        0b000,
    }
};

const digit_t dm_heart = {
    .cols=7,
    .data = {
        0b0110110,
        0b1111111,
        0b0111110,
        0b0011100,
        0b0001000,
    }
};

const digit_t dm_0 = {
    .cols = 3,
    .data = {
        0b111,
        0b101,
        0b101,
        0b101,
        0b111,
    }};

const digit_t dm_1 = {
    .cols = 2,
    .data = {
        0b01,
        0b11,
        0b01,
        0b01,
        0b01,
    }};

const digit_t dm_2 = {
    .cols = 3,
    .data = {
        0b111,
        0b001,
        0b111,
        0b100,
        0b111,
    }};

const digit_t dm_3 = {
    .cols = 3,
    .data = {
        0b111,
        0b001,
        0b111,
        0b001,
        0b111,
    }};

const digit_t dm_4 = {
    .cols = 3,
    .data = {
        0b101,
        0b101,
        0b111,
        0b001,
        0b001,
    }};

const digit_t dm_5 = {
    .cols = 3,
    .data = {
        0b111,
        0b100,
        0b111,
        0b001,
        0b111,
    }};

const digit_t dm_6 = {
    .cols = 3,
    .data = {
        0b111,
        0b100,
        0b111,
        0b101,
        0b111,
    }};

const digit_t dm_7 = {
    .cols = 3,
    .data = {
        0b111,
        0b001,
        0b010,
        0b100,
        0b100,
    }};

const digit_t dm_8 = {
    .cols = 3,
    .data = {
        0b111,
        0b101,
        0b111,
        0b101,
        0b111,
    }};

const digit_t dm_9 = {
    .cols = 3,
    .data = {
        0b111,
        0b101,
        0b111,
        0b001,
        0b111,
    }};

const digit_t dm_A = {
    .cols = 3,
    .data = {
        0b111,
        0b101,
        0b111,
        0b101,
        0b101,
    }
};

const digit_t dm_C = {
    .cols = 3,
    .data = {
        0b111,
        0b100,
        0b100,
        0b100,
        0b111,
    }
};

const digit_t dm_D = {
    .cols = 3,
    .data = {
        0b110,
        0b101,
        0b101,
        0b101,
        0b110,
    }
};

const digit_t dm_E = {
    .cols = 3,
    .data = {
        0b111,
        0b100,
        0b111,
        0b100,
        0b111,
    }
};

const digit_t dm_F = {
    .cols = 3,
    .data = {
        0b111,
        0b100,
        0b111,
        0b100,
        0b100,
    }
};


const digit_t dm_G = {
    .cols = 3,
    .data = {
        0b111,
        0b100,
        0b111,
        0b101,
        0b111
    }
};

const digit_t dm_H = {
    .cols = 3,
    .data = {
        0b101,
        0b101,
        0b111,
        0b101,
        0b101,
    }
};

const digit_t dm_I = {
    .cols = 1,
    .data = {
        0b1,
        0b1,
        0b1,
        0b1,
        0b1,
    }
};

const digit_t dm_L = {
    .cols = 2,
    .data = {
        0b10,
        0b10,
        0b10,
        0b10,
        0b11,
    }
};

const digit_t dm_M = {
    .cols = 5,
    .data = {
        0b10001,
        0b11011,
        0b10101,
        0b10001,
        0b10001,
    }
};

const digit_t dm_N = {
    .cols = 3,
    .data = {
        0b110,
        0b101,
        0b101,
        0b101,
        0b101,
    }
};

const digit_t dm_O = {
    .cols = 3,
    .data = {
        0b111,
        0b101,
        0b101,
        0b101,
        0b111,
    }
};

const digit_t dm_P = {
    .cols = 3,
    .data = {
        0b111,
        0b101,
        0b111,
        0b100,
        0b100,
    }
};

const digit_t dm_R = {
    .cols = 3,
    .data = {
        0b111,
        0b101,
        0b110,
        0b101,
        0b101,
    }
};

const digit_t dm_S = {
    .cols = 3,
    .data = {
        0b111,
        0b100,
        0b111,
        0b001,
        0b111,
    }
};

const digit_t dm_T = {
    .cols = 3,
    .data = {
        0b111,
        0b010,
        0b010,
        0b010,
        0b010,
    }
};

const digit_t dm_U = {
    .cols = 3,
    .data = {
        0b101,
        0b101,
        0b101,
        0b101,
        0b111,
    }
};

const digit_t dm_W = {
    .cols = 5,
    .data = {
        0b10001,
        0b10001,
        0b10101,
        0b10101,
        0b11111,
    }
};

const digit_t dm_X = {
    .cols = 5,
    .data = {
        0b10001,
        0b01010,
        0b00100,
        0b01010,
        0b10001
    }
};

const digit_t dm_x = {
    .cols = 3,
    .data = {
        0b000,
        0b000,
        0b101,
        0b010,
        0b101,
    }
};

digit_map_t dm_map[] = {
    {.ch=' ', .dm=&dm_space},
    {.ch='%', .dm=&dm_percentage},
    {.ch='-', .dm=&dm_hyphen},
    {.ch='/', .dm=&dm_slash},
    {.ch='@', .dm=&dm_heart},
    {.ch='0', .dm=&dm_0},
    {.ch='1', .dm=&dm_1},
    {.ch='2', .dm=&dm_2},
    {.ch='3', .dm=&dm_3},
    {.ch='4', .dm=&dm_4},
    {.ch='5', .dm=&dm_5},
    {.ch='6', .dm=&dm_6},
    {.ch='7', .dm=&dm_7},
    {.ch='8', .dm=&dm_8},
    {.ch='9', .dm=&dm_9},
    {.ch='A', .dm=&dm_A},
    {.ch='C', .dm=&dm_C},
    {.ch='D', .dm=&dm_D},
    {.ch='E', .dm=&dm_E},
    {.ch='F', .dm=&dm_F},
    {.ch='G', .dm=&dm_G},
    {.ch='H', .dm=&dm_H},
    {.ch='I', .dm=&dm_I},
    {.ch='L', .dm=&dm_L},
    {.ch='M', .dm=&dm_M},
    {.ch='N', .dm=&dm_N},
    {.ch='O', .dm=&dm_O},
    {.ch='P', .dm=&dm_P},
    {.ch='R', .dm=&dm_R},
    {.ch='S', .dm=&dm_S},
    {.ch='T', .dm=&dm_T},
    {.ch='U', .dm=&dm_U},
    {.ch='W', .dm=&dm_W},
    {.ch='X', .dm=&dm_X},
    {.ch='x', .dm=&dm_x},
};

const digit_t *dm_number(const int number) {
    switch (number) {
    case 0: return &dm_0;
    case 1: return &dm_1;
    case 2: return &dm_2;
    case 3: return &dm_3;
    case 4: return &dm_4;
    case 5: return &dm_5;
    case 6: return &dm_6;
    case 7: return &dm_7;
    case 8: return &dm_8;
    case 9: return &dm_9;
    }
    return 0;
}

const digit_t *dm_digit(char ch) {
    for (uint32_t i = 0; i < sizeof(dm_map)/sizeof(digit_map_t); ++i) {
        if (dm_map[i].ch == ch) return dm_map[i].dm;
    }
    return 0;
}

int dm_compute_length(char *text) {
    int length = 0;
    while (*text) {
        const digit_t *dm = dm_digit(*text);
        if (dm == 0) return 0;
        length += dm->cols + 1;
        text++;
    }
    return (length > 0) ? --length: length;
}
