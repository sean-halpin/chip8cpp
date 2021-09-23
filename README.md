## A CHIP8 interpreter in C++

#### Build
```
mkdir -p ./bin && g++ -pedantic -Wall -g -std=c++14 ./src/main.cpp -o ./bin/main.bin
```

#### RUN
```
./bin/main.bin
```

#### OUTPUT

The chip-8 video frame is visualized in ascii, followed by a printout of the registers.

![image](./media/chip8_screen.png)

## Resources

- [http://www.cs.columbia.edu/~sedwards/classes/2016/4840-spring/designs/Chip8.pdf](http://www.cs.columbia.edu/~sedwards/classes/2016/4840-spring/designs/Chip8.pdf)
- [wikipedia.org/wiki/CHIP-8](https://en.wikipedia.org/wiki/CHIP-8#Opcode_table)
- [github.com/ismaelrh/Java-chip8-emulator](https://github.com/ismaelrh/Java-chip8-emulator/tree/6bbf5496e4f10bac47c4895dbe673a42a3548b9e)