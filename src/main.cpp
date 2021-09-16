#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
using namespace std;

#define RAM_LENGTH 4096
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define STACK_LENGTH 16
// Memory
unsigned char ram[RAM_LENGTH];
unsigned short stack[STACK_LENGTH];
unsigned char video_frame[SCREEN_WIDTH][SCREEN_HEIGHT];
// Registers
unsigned char V0, V1, V2, V3, V4, V5, V6, V7, V8, V9, VA, VB, VC, VD, VE, VF;
unsigned char DT, ST, SP;
unsigned short I;
unsigned short PC;

unsigned char *decode_register(unsigned char register_hex);
unsigned int read_rom(unsigned char **rom);
void print_array_hex(unsigned char *buffer, unsigned int length);
void bootstrap_fontset(unsigned char *ram);
void print_registers();
void execute_opcode();

union opCode_t
{
    struct
    {
        unsigned char lo;
        unsigned char hi;
    };
    unsigned short hl;
} opCode;

int main()
{
    unsigned char *rom;
    unsigned int rom_size = read_rom(&rom);
    bootstrap_fontset(ram);
    print_array_hex(rom, rom_size);
    print_array_hex(ram, RAM_LENGTH);

    cout << "ram length: " << sizeof(ram) << endl;
    cout << "stack length: " << sizeof(stack) << endl;

    while (true)
    {
        opCode.hi = rom[PC];
        opCode.lo = rom[PC + 1];
        // cout << setfill('0') << setw(4) << hex << opCode.hl << dec << endl;
        // cout << setfill('0') << setw(2) << hex << (int)opCode.hi << dec << endl;
        // cout << setfill('0') << setw(2) << hex << (int)opCode.lo << dec << endl;
        cout << endl;
        execute_opcode();
        print_registers();
    }

    return 0;
}

void execute_opcode()
{
    switch (opCode.hl & 0xF000)
    {
    // 6xkk - LD Vx, byte
    case 0x6000:
    {
        unsigned char *reg = decode_register((opCode.hl & 0x0F00) >> 8);
        *reg = opCode.hl & 0x00FF;
        PC += 2;
        break;
    }
    // Annn - LD I, addr
    case 0xA000:
    {
        I = opCode.hl & 0x0FFF;
        PC += 2;
        break;
    }
    default:
        cerr << "unknown opcode: 0x" << setfill('0') << setw(4) << hex << (int)(opCode.hl & 0xFFFF) << dec << " " << endl;
        throw std::exception();
        break;
    }
}

unsigned char *decode_register(unsigned char register_hex)
{
    switch (register_hex)
    {
    case 0x01:
        return &V1;
        break;
    case 0x02:
        return &V2;
        break;
    case 0x03:
        return &V3;
        break;
    case 0x04:
        return &V4;
        break;
    case 0x05:
        return &V5;
        break;
    case 0x06:
        return &V6;
        break;
    case 0x07:
        return &V7;
        break;
    case 0x08:
        return &V8;
        break;
    case 0x09:
        return &V9;
        break;
    case 0x0A:
        return &VA;
        break;
    case 0x0B:
        return &VB;
        break;
    case 0x0C:
        return &VC;
        break;
    case 0x0D:
        return &VD;
        break;
    case 0x0E:
        return &VE;
        break;
    case 0x0F:
        return &VF;
        break;
    default:
        cerr << "unknown register_hex: 0x" << setfill('0') << setw(2) << hex << (int)register_hex << dec << " " << endl;
        throw std::exception();
        break;
    }
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
    for (int i = 0; i < length; i++)
    {
        if (i % 32 == 0)
        {
            if (i != 0)
                cout << "|" << endl;
            cout << i << "\t-\t| ";
        }
        cout << setfill('0') << setw(2) << hex << (0xFF & (buffer[i])) << dec << " ";
    }
    cout << "|" << endl;
}

void print_registers()
{
    cout << "V0:" << setfill('0') << setw(2) << hex << (int)V0 << dec << " ";
    cout << "V1:" << setfill('0') << setw(2) << hex << (int)V1 << dec << " ";
    cout << "V2:" << setfill('0') << setw(2) << hex << (int)V2 << dec << " ";
    cout << "V3:" << setfill('0') << setw(2) << hex << (int)V3 << dec << " ";
    cout << "V4:" << setfill('0') << setw(2) << hex << (int)V4 << dec << " ";
    cout << "V5:" << setfill('0') << setw(2) << hex << (int)V5 << dec << " ";
    cout << "V6:" << setfill('0') << setw(2) << hex << (int)V6 << dec << " ";
    cout << "V7:" << setfill('0') << setw(2) << hex << (int)V7 << dec << " " << endl;
    cout << "V8:" << setfill('0') << setw(2) << hex << (int)V8 << dec << " ";
    cout << "V9:" << setfill('0') << setw(2) << hex << (int)V9 << dec << " ";
    cout << "VA:" << setfill('0') << setw(2) << hex << (int)VA << dec << " ";
    cout << "VB:" << setfill('0') << setw(2) << hex << (int)VB << dec << " ";
    cout << "VC:" << setfill('0') << setw(2) << hex << (int)VC << dec << " ";
    cout << "VD:" << setfill('0') << setw(2) << hex << (int)VD << dec << " ";
    cout << "VE:" << setfill('0') << setw(2) << hex << (int)VE << dec << " ";
    cout << "VF:" << setfill('0') << setw(2) << hex << (int)VF << dec << " " << endl;
    cout << "PC:" << setfill('0') << setw(2) << hex << (int)PC << dec << " " << endl;
    cout << "I:" << setfill('0') << setw(2) << hex << (int)I << dec << " " << endl;
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