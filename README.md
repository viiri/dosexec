# dosexec

dosexec is a shitty hacked up tool that allows transparent execution of 16-bit
DOS executables on (hopefully) any machine. The use case that led to this was
wanting to run some 16-bit tools on my 64-bit Windows machine, and DOSbox's
lack of "transparent" behavior.

The source code should make it very evident that I hacked this together out of
my WIP WonderSwan emulator.

It's a serious hunk of garbage though, so it'll probably only work with very
simple tools that do very basic I/O. Feel free to expand on it though, I think
I engineered it enough to be easy to add more BIOS routines.

If you find this useful, I'd really like to hear about it. I've felt like this
would be a useful tool for a couple years now, wondering if I'm the only one!
Email is <trap15@raidenii.net> as usual.

# Supported BIOS routines

- INT 21h
    - AH=02h
    - AH=09h
    - AH=25h (not really)
    - AH=2Ah 
    - AH=2Ch
    - AH=30h
    - AH=35h (not really)
    - AH=3Bh
    - AH=3Ch
    - AH=3Dh
    - AH=3Eh
    - AH=3Fh
    - AH=40h
    - AH=42h
    - AX=4300h (not really)
    - AX=4400h (not really)
    - AH=48h
    - AH=49h
    - AH=4Ah
    - AH=4Ch

Yes that's really it. They're not even well implemented.

There is stuff in place to make stuff sorta work even if the routine's not
actually implemented, but who knows.

# Using

It takes no command line parameters (though it really probably should). Some
stuff might get logged out to a `log.txt`, sorry about that. You can turn it
off if you poke the two values at the top of src/core/util.c.

Simply: `./dosexec DOSPRO~1.COM /switch donger whoosit`

# Cool Links

* <http://daifukkat.su/> - My website
* <https://bitbucket.org/trap15/dosexec> - This repository
* <http://www.ctyme.com/rbrown.htm> - Ralf Brown's Interrupt List

# Greetz

* Ralf Brown for the fuckin impeccable interrupt list
* \#raidenii - Forever impossible
* Data East arcade game music

# Licensing

All contents within this repository (unless otherwise specified) are licensed
under the following terms:

The MIT License (MIT)

Copyright (c) 2015 Alex 'trap15' Marshall

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
