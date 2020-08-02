# 67 Key keyboard with trackpoint

## TODO

- [ ] Finish keymap/layers
- [ ] Make tp reset non blocking?
- [ ] uf2: turn down brightness on rgbs
- [ ] sends 3 reports whenever any key changes. problem?
- [ ] Implement debounce? Does happen occasionally.
- [ ] Set serial number in descriptor? check uf2 source. add to
tud\_descriptor\_string\_cb
- [ ] Merge spi master and matrix?
- [ ] Make force reset universal
- [ ] Turn off unused clocks? APB ADC. Whatever was started by uf2
- [ ] Figure out clean way to have keys do things outside of keymap
- [ ] Set up Fn lock
- [x] tp reading and writing from ram stalls sometimes. dataAvailable not being
set? Done, set up timeout in tp\_getDataBit().
- [x] Wakeup from TP and spacebar working, with idle(1). Standby not working, must
set up low level interrupt and clocks. Linux not setting wakeup enabled by
default, works when changed. Must be hid only? Added udev rule from archwiki.
Changed HID portocol to keyboard in hid descriptor. Wakeup enabled without udev
rule
- [x] set up modifier reporting
- [x] set up system for leds
- [x] Delay a bit after middle mouse to make sure it's not a scroll. Do like tp
wired keyboard, only send middle button on release? Done, hardcoded dirtily.

## Power and Sleep

The trackpoint uses an external interrupt on the falling edge to clock in data.
There is also a line going straight to the space bar, set up to interrupt on a
low level. Level interrupts are asynchronout and can wake up from deep sleep.
These are both configured in trackpoint.c. So, to go to sleep, I set tp clk pin
to trigger on a low level and enable the spacebar interrupt. Immediately on wake
up (or first call) I turn off the spacebar interrupt and reset the tp clk pin to
a falling edge interrupt.

Waking up from sleep has been a little tricky. Entering deep sleep works. Waking
up from a low level external interrupt also works. But, waking up from a usb
resume does not start up correctly. The current draw spikes, then drops back
down. So, I'm using idle(2). Sleeps and wakes up fine.

- Deep sleep with Trackpoint:	13.5mA
- Deep sleep w/o Trackpoint:	1.1mA
- Idle(2) with Trackpoint:		17.5mA
- Normal with LEDs:				26mA
