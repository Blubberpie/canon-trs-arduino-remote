# Arduino Remote Control for DSLRs
An Arduino-powered timer-based remote control for DSLRs that support a TRS 2.5mm jack input.

## Disclaimers
This is a personal project so there won't be much documentation or help. I made the entire setup based on what I had lying around.

## Features
- Triggers the camera every X seconds/minutes.
- Increment/decrement timer by 15 seconds with two buttons.
- I had a Focus button + Release button (separated from the Arduino) for manual control
- On-board LED blinks for the number of minutes of the timer, followed by a one second delay. i.e. 3 mins = blink thrice (wait 1 second, repeat)

## Parts
- Arduino UNO R3
  - Buttons
  - Jumper cables
- 2.5mm to 3.5mm headphone cable (TRS)
  - TRRS is also fine, but I had a TRRS splitter cable
- Relay module (mine was SRD-05VDC-SL-C)
- Camera

## Timerless
If you just want a remote, all you need is the 2.5mm cable, some wires, and some buttons. This entire project is based on this image:

![TRS Camera Controller](https://www.doc-diy.net/photo/eos_wired_remote/pinout.png)

See https://www.doc-diy.net/photo/eos_wired_remote/ for the details.

## The Arduino
- Arduino I/O
  - 9: Coil trigger
  - 10: Increment button
  - 11: Decrement button
- The power supply in the diagram below represents the camera.
  - Tip: Release (red wire in diagram)
  - Ring: Focus (orange)
  - Sleeve: Ground (black)
- Two buttons on the right:
  - Top: Increment timer by 15 seconds
  - Bottom: Decrement timer by 15 seconds
- Two buttons on the left (should be separate from the Arduino - except maybe for ground wires):
  - Left: Focus
  - Right: Release
- Tinkercad doesn't have the SRD-05VDC-SL-C relay, so this the diagram shows a rough wiring
(I don't know even if it's functional). You can wire yours appropriately based on what you have.

![Tinkercad Diagram](https://user-images.githubusercontent.com/37857275/201477317-8e5d74d6-6465-48fe-91b1-13a9396180c9.png)
