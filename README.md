__67 Key keyboard with trackpoint__


__TODO__
[ ] tp reading and writing from ram stalls sometimes. dataAvailable not being set?
[ ] uf2: turn down brightness on rgbs
[ ] sends 3 reports whenever any key changes. problem?
[ ] Implement debounce?
[ ] Set serial number in descriptor? check uf2 source
[ ] Merge spi master and matrix?
[ ] Make force reset universal
[ ] Wakeup from TP and spacebar working, with idle(1). Standby not working, must set up low level interrupt and clocks. Linux not setting wakeup enabled by default, works when changed. Must be hid only? Added udev rule from archwiki.
[ ] Turn off unused clocks? APB ADC. Whatever was started by uf2
[ ] Figure out clean way to have keys do things outside of keymap
[x] set up modifier reporting
[x] set up system for leds
[x] Delay a bit after middle mouse to make sure it's not a scroll. Do like tp wired keyboard, only send middle button on release? Done, hardcoded dirtily. 
