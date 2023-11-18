# tick-circuit

This is a work in progress, largely copied from the
[Mix and Tick](http://aminet.net/package/docs/hard/Mix_and_Tick)
by Michael Schulz.

This will take a +5V and GND input from a power supply and output a 50 or 60 Hz
square wave from J2, depending on the quartz crystal placed at X1. This circuit
has been prototyped with the 3.93216 MHz crystal, providing 60 Hz. The PCB in
place comes from someone who has never done a PCB before, and this version is
currently on order.

## BOM

| Part             | Quantity | Location |
| ---------------- | -------- | -------- |
| 74HCT4060        | 1        | U1       |
| 74HCT73          | 1        | U2       |
| 10M Resistor     | 1        | R1       |
| 100K Resistor    | 1        | R2       |
| 6.8pF Capacitor  | 2        | C1, C2   |
| 100nF Capacitor  | 1        | C3       |
| 3.93216 MHz Xtal | 1        | X1       |

3.93216 MHz Xtal can be replaced with a 3.2768 MHz for 50 Hz output.

## Resources

This uses the logic-4000 libraries from https://github.com/alexisvl/kicad-schlib
