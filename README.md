# nonmasc brainfuck compiler

just a simple [brainfuck](https://esolangs.org/wiki/Brainfuck) compiler.

usage:

`jabf <input file>? [options]`

options:

- \-o: output file
- \-h: shows help menu and exits
- \-v: shows version menu and exits

compilation:

to compile locally just run `make`

to compile and install set export `BINDIR` to the install location
(set to `/usr/local/bin` by default). And then run `sudo make
install`
