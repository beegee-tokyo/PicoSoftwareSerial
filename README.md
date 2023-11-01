
# Origin: [Flamabalistic PicoSoftwareSerialExample](https://github.com/Flamabalistic/PicoSoftwareSerialExample)

Converted without any code changes into an installable Arduino and PIO library.

## _**All credits go to [Flamabalistic](https://github.com/Flamabalistic)**_

-----------------------------------------------

PicoSoftwareSerialExample

Quick 'n' dirty PIO based implementation of SoftwareSerial for an RP2040 based board

    What definetly works:
        Baud rate of 115200
        2 streams running on the one PIO block (taking up all 4 state machines)

Not too sure about the reliability of it all, but it should work in a pinch!

A dedicated implementation of SoftwareSerial for the Pico which uses the PIO.
I have tested it with a speed up to 115200 and it works like a charm.

----------------------------------------------