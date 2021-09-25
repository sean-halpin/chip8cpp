#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
using namespace std;

#define RAM_LENGTH 4096
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define STACK_LENGTH 0x400
#define ROM_OFFSET 0x200
// Memory
unsigned char video_frame[SCREEN_WIDTH][SCREEN_HEIGHT];
unsigned char ram[RAM_LENGTH];
unsigned short stack[STACK_LENGTH];
// Registers
unsigned char V[0x0F];
unsigned char DT, ST, SP;
unsigned short I;
unsigned short PC;
// Hex Keyboard
unsigned char keyboard[0x0F];

unsigned int read_rom(unsigned char **rom);
void print_array_hex(unsigned char *buffer, unsigned int length);
void bootstrap_fontset(unsigned char *ram);
void print_video_frame();
void clear_video_frame();
void print_registers();
void execute_opcode();
void diagnostics();
void clr();
void error(const char *);

unsigned int max_cycles = 10000;
unsigned int cycles = 0;

union opCode_t
{
    struct
    {
        unsigned char lo;
        unsigned char hi;
    };
    unsigned short code;
} op;

int main()
{
    unsigned char *rom;
    unsigned int rom_size = read_rom(&rom);
    bootstrap_fontset(ram);
    std::copy(rom, rom + rom_size, ram + ROM_OFFSET);
    PC = ROM_OFFSET;

    while (cycles++ < max_cycles)
    {
        op.hi = ram[PC];
        op.lo = ram[PC + 1];
        diagnostics();
        execute_opcode();
    }
    diagnostics();
    return 0;
}

void execute_opcode()
{
    switch (op.code & 0xF000)
    {
    case 0x0000:
    {
        switch (op.code & 0x00FF)
        {
        // 00E0 - CLS
        case 0xE0:
            clear_video_frame();
            PC += 2;
            break;
        // 00EE - RET
        case 0xEE:
            if (SP == 0)
            {
                PC += 2;
            }
            else
            {
                PC = stack[--SP] + 2;
            }
            break;
        default:
            error("Unknown 0x0000 OpCode");
            break;
        }
        break;
    }
    // 1nnn - JP addr
    case 0x1000:
    {
        unsigned short nnn = op.code & 0x0FFF;
        PC = nnn;
    }
    // 2nnn - CALL addr
    case 0x2000:
    {
        if (SP >= STACK_LENGTH)
            error("StackOverflow");
        unsigned short nnn = op.code & 0x0FFF;
        stack[SP++] = PC;
        PC = nnn;
        break;
    }
    // 3xkk - SE Vx, byte
    case 0x3000:
    {
        unsigned char x = (op.code & 0x0F00) >> 8;
        unsigned char kk = op.code & 0x00FF;
        PC += (V[x] == kk) ? 4 : 2;
        break;
    }
    // 4xkk - SNE Vx, byte
    case 0x4000:
    {
        unsigned char x = (op.code & 0x0F00) >> 8;
        unsigned char kk = op.code & 0x00FF;
        PC += (V[x] != kk) ? 4 : 2;
        break;
    }
    // 6xkk - LD Vx, byte
    case 0x6000:
    {
        unsigned char x = (op.code & 0x0F00) >> 8;
        unsigned char kk = op.code & 0x00FF;
        V[x] = kk;
        PC += 2;
        break;
    }
    // 7xkk - ADD Vx, byte
    case 0x7000:
    {
        unsigned char x = (op.code & 0x0F00) >> 8;
        unsigned char kk = op.code & 0x00FF;
        V[x] += kk;
        PC += 2;
        break;
    }
    case 0x8000:
    {
        unsigned char x = (op.code & 0x0F00) >> 8;
        unsigned char y = (op.code & 0x00F0) >> 4;
        switch (op.code & 0x000F)
        {
        // 8xy0 - LD Vx, Vy
        case 0x0:
            V[x] = V[y];
            PC += 2;
            break;
        default:
            error("Unknown 0x8000 OpCode");
            break;
        }
        break;
    }
    // Annn - LD I, addr
    case 0xA000:
    {
        I = op.code & 0x0FFF;
        PC += 2;
        break;
    }
    // Dxyn - DRW Vx, Vy, nibble
    case 0xD000:
    {
        unsigned char x = (op.code & 0x0F00) >> 8;
        unsigned char y = (op.code & 0x00F0) >> 4;
        unsigned char n = (op.code & 0x000F);

        unsigned char vf = 0x00;

        unsigned char coord_x = V[x];
        unsigned char coord_y = V[y];

        unsigned char m = 0b10000000;
        for (unsigned char h = 0; h < n; h++)
        {
            m = 0b10000000;
            for (unsigned char w = 0; w < 8; w++)
            {
                if ((ram[I + h] & m) > 0) // Draw a pixel
                {
                    cout << "w:" << w << "\th:" << h << endl;
                    if (video_frame[coord_x + w][coord_y + h]) // Pixel already set, set VF to 1
                    {
                        vf = 0x01;
                    }
                    video_frame[coord_x + w][coord_y + h] ^= 1;
                }
                m >>= 1;
            }
        }
        V[0xF] = vf;
        PC += 2;
        break;
    }
    case 0xE000:
    {
        unsigned char x = (op.code & 0x0F00) >> 8;
        switch (op.code & 0x00FF)
        {
        // Ex9E - SKP Vx
        case 0x9E:
            PC += keyboard[V[x]] == 0x01 ? 4 : 2;
            break;
        // ExA1 - SKNP Vx
        case 0xA1:
            PC += keyboard[V[x]] != 0x01 ? 4 : 2;
            break;
        default:
            error("Unknown 0xE000 OpCode");
            break;
        }
        break;
    }
    case 0xF000:
    {
        unsigned const char x = (op.code & 0x0F00) >> 8;
        switch (op.code & 0x00FF)
        {
        case 0x0007:
        {
            // Fx07 - LD Vx, DT
            V[x] = DT;
            PC += 2;
            break;
        }
        case 0x0015:
        {
            // Fx15 - LD DT, Vx
            DT = V[x];
            PC += 2;
            break;
        }
        case 0x001E:
        {
            // Fx1E - ADD I, Vx
            I += V[x];
            // Overflow
            V[0xF] = (I & 0xF000) != 0 ? 1 : 0;
            I = I & 0xFFF;
            PC += 2;
            break;
        }
        case 0x0029:
        {
            // Fx29 - LD F, Vx
            I = V[x] * 5; // Since each sprite is 5 bytes long
            PC += 2;
            break;
        }
        case 0x0033:
        {
            // Fx33 - LD B, Vx
            unsigned char vx = V[x];
            ram[I] = vx / 100;
            ram[I + 1] = (vx % 100) / 10;
            ram[I + 2] = vx % 10;
            PC += 2;
            break;
        }
        case 0x0065:
        {
            // Fx65 - LD Vx, [I]
            for (int i = 0; i < x; i++)
            {
                V[i] = ram[I + x];
            }
            PC += 2;
            break;
        }
        default:
            error("Unknown 0xF000 OpCode");
            break;
        }
        break;
    }
    default:
        error("Unknown OpCode");
        break;
    }
}

void clr()
{
    {
#if defined _WIN32
        system("cls");
        //clrscr(); // including header file : conio.h
#elif defined(__LINUX__) || defined(__gnu_linux__) || defined(__linux__)
        system("clear");
        //std::cout<< u8"\033[2J\033[1;1H"; //Using ANSI Escape Sequences
#elif defined(__APPLE__)
        system("clear");
#endif
    }
}

void diagnostics()
{
    // clr();
    // print_array_hex(ram, RAM_LENGTH);
    cout << "ram length: " << sizeof(ram) << endl;
    print_video_frame();
    print_registers();
    cout << "cycles : " << cycles << endl;
    cout << "opCode: " << setfill('0') << setw(4) << hex << op.code << dec << endl;
}

void error(const char *msg)
{
    diagnostics();
    throw std::runtime_error(msg);
}

unsigned int read_rom(unsigned char **rom)
{
    long size = 0;
    ifstream file("./roms/PONG", ios::in | ios::binary | ios::ate);
    size = file.tellg();
    file.seekg(0, ios::beg);

    *rom = new unsigned char[size];
    file.read((char *)*rom, size);
    file.close();

    if (rom == nullptr)
        cout << "Error: memory could not be allocated" << endl;

    return (unsigned int)size;
}

void print_array_hex(unsigned char *buffer, unsigned int length)
{
    for (unsigned int i = 0; i < length; i++)
    {
        if (i % 32 == 0)
        {
            if (i != 0)
                cout << "|" << endl;
            cout << "0x" << setfill('0') << setw(4) << hex << (int)(i) << dec << "\t-\t| ";
        }
        cout << setfill('0') << setw(2) << hex << (0xFF & (buffer[i])) << dec << " ";
    }
    cout << "|" << endl
         << endl;
}

void clear_video_frame()
{
    for (int h = 0; h < SCREEN_HEIGHT; h++)
    {
        for (int w = 0; w < SCREEN_WIDTH; w++)
        {
            video_frame[w][h] = 0;
        }
    }
}

void print_video_frame()
{
    for (int h = 0; h < SCREEN_HEIGHT; h++)
    {
        for (int w = 0; w < SCREEN_WIDTH; w++)
        {
            if (video_frame[w][h])
            {
                cout << "x";
            }
            else
            {
                cout << "-";
            }
        }
        cout << "|" << endl;
    }
    cout << endl;
}

void print_registers()
{
    cout << "V0:" << setfill('0') << setw(2) << hex << (int)(V[0x00]) << dec << " ";
    cout << "V1:" << setfill('0') << setw(2) << hex << (int)(V[0x01]) << dec << " ";
    cout << "V2:" << setfill('0') << setw(2) << hex << (int)(V[0x02]) << dec << " ";
    cout << "V3:" << setfill('0') << setw(2) << hex << (int)(V[0x03]) << dec << " ";
    cout << "V4:" << setfill('0') << setw(2) << hex << (int)(V[0x04]) << dec << " ";
    cout << "V5:" << setfill('0') << setw(2) << hex << (int)(V[0x05]) << dec << " ";
    cout << "V6:" << setfill('0') << setw(2) << hex << (int)(V[0x06]) << dec << " ";
    cout << "V7:" << setfill('0') << setw(2) << hex << (int)(V[0x07]) << dec << " " << endl;
    cout << "V8:" << setfill('0') << setw(2) << hex << (int)(V[0x08]) << dec << " ";
    cout << "V9:" << setfill('0') << setw(2) << hex << (int)(V[0x09]) << dec << " ";
    cout << "VA:" << setfill('0') << setw(2) << hex << (int)(V[0x0A]) << dec << " ";
    cout << "VB:" << setfill('0') << setw(2) << hex << (int)(V[0x0B]) << dec << " ";
    cout << "VC:" << setfill('0') << setw(2) << hex << (int)(V[0x0C]) << dec << " ";
    cout << "VD:" << setfill('0') << setw(2) << hex << (int)(V[0x0D]) << dec << " ";
    cout << "VE:" << setfill('0') << setw(2) << hex << (int)(V[0x0E]) << dec << " ";
    cout << "VF:" << setfill('0') << setw(2) << hex << (int)(V[0x0F]) << dec << " " << endl;
    cout << "PC:" << setfill('0') << setw(4) << hex << (int)PC << dec << " " << endl;
    cout << "SP:" << setfill('0') << setw(4) << hex << (int)SP << dec << " " << endl;
    cout << "I:" << setfill('0') << setw(4) << hex << (int)I << dec << " " << endl;
}

void bootstrap_fontset(unsigned char *ram)
{
    // "0"	Binary
    ram[0x00] = 0b11110000;
    ram[0x01] = 0b10010000;
    ram[0x02] = 0b10010000;
    ram[0x03] = 0b10010000;
    ram[0x04] = 0b11110000;
    // "1"	Binary
    ram[0x05] = 0b00100000;
    ram[0x06] = 0b01100000;
    ram[0x07] = 0b00100000;
    ram[0x08] = 0b00100000;
    ram[0x09] = 0b01110000;
    // "2"	Binary
    ram[0x0A] = 0b11110000;
    ram[0x0B] = 0b00010000;
    ram[0x0C] = 0b11110000;
    ram[0x0D] = 0b10000000;
    ram[0x0E] = 0b11110000;
    // "3"	Binary
    ram[0x0F] = 0b11110000;
    ram[0x10] = 0b00010000;
    ram[0x11] = 0b11110000;
    ram[0x12] = 0b00010000;
    ram[0x13] = 0b11110000;
    // "4"	Binary
    ram[0x14] = 0b10010000;
    ram[0x15] = 0b10010000;
    ram[0x16] = 0b11110000;
    ram[0x17] = 0b00010000;
    ram[0x18] = 0b00010000;
    // "5"	Binary
    ram[0x19] = 0b11110000;
    ram[0x1A] = 0b10000000;
    ram[0x1B] = 0b11110000;
    ram[0x1C] = 0b00010000;
    ram[0x1D] = 0b11110000;
    // "6"	Binary
    ram[0x1E] = 0b11110000;
    ram[0x1F] = 0b10000000;
    ram[0x20] = 0b11110000;
    ram[0x21] = 0b10010000;
    ram[0x22] = 0b11110000;
    // "7"	Binary
    ram[0x23] = 0b11110000;
    ram[0x24] = 0b00010000;
    ram[0x25] = 0b00100000;
    ram[0x26] = 0b01000000;
    ram[0x27] = 0b01000000;
    // "8"	Binary
    ram[0x28] = 0b11110000;
    ram[0x29] = 0b10010000;
    ram[0x2A] = 0b11110000;
    ram[0x2B] = 0b10010000;
    ram[0x2C] = 0b11110000;
    // "9"	Binary
    ram[0x2D] = 0b11110000;
    ram[0x2E] = 0b10010000;
    ram[0x2F] = 0b11110000;
    ram[0x30] = 0b00010000;
    ram[0x31] = 0b11110000;
    // "A"	Binary
    ram[0x32] = 0b11110000;
    ram[0x33] = 0b10010000;
    ram[0x34] = 0b11110000;
    ram[0x35] = 0b10010000;
    ram[0x36] = 0b10010000;
    // "B"	Binary
    ram[0x37] = 0b11100000;
    ram[0x38] = 0b10010000;
    ram[0x39] = 0b11100000;
    ram[0x3A] = 0b10010000;
    ram[0x3B] = 0b11100000;
    // "C"	Binary
    ram[0x3C] = 0b11110000;
    ram[0x3D] = 0b10000000;
    ram[0x3E] = 0b10000000;
    ram[0x3F] = 0b10000000;
    ram[0x40] = 0b11110000;
    // "D"	Binary
    ram[0x41] = 0b11100000;
    ram[0x42] = 0b10010000;
    ram[0x43] = 0b10010000;
    ram[0x44] = 0b10010000;
    ram[0x45] = 0b11100000;
    // "E"	Binary
    ram[0x46] = 0b11110000;
    ram[0x47] = 0b10000000;
    ram[0x48] = 0b11110000;
    ram[0x49] = 0b10000000;
    ram[0x4A] = 0b11110000;
    // "F"	Binary
    ram[0x4B] = 0b11110000;
    ram[0x4C] = 0b10000000;
    ram[0x4D] = 0b11110000;
    ram[0x4E] = 0b10000000;
    ram[0x4F] = 0b10000000;
}