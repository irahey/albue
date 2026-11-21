# One-shot prompt: pan-tilt workshop tutorial, diagrams, sketches, and checklist

Paste everything below the line into an AI assistant that can create files. Tell it to produce all four deliverables in one go.

---

You are writing the handout for a 120-minute, beginner-friendly workshop called "Vibe coding a robot: from prompt to physical world". Participants build a phone-controlled pan-tilt platform with an AI coding assistant, then learn the physical-world skills that the AI does not do for them: paper checks, power budgets, measured power-up, mechanical limits, and failsafes.

## Deliverables (produce all four)

1. **Tutorial**: one self-contained HTML file with no external dependencies. It contains every section listed under "Tutorial structure", three inline diagrams, copy buttons for the prompts and sketches, and both reference sketches embedded.
2. **ESP32 sketch** (`PanTilt_Workshop.ino`), also embedded in the tutorial.
3. **ESP8266 sketch** (`PanTilt_Workshop_ESP8266.ino`), also embedded in the tutorial.
4. **Checklist**: one self-contained HTML file for this project only. No tabs, and no checklists for other projects.

## The project and the hard facts

- Two micro servos (SG90 or MG90S type) in a pre-printed pan-tilt bracket, driven from a web page on a phone.
- **ESP32 DevKit is the main choice** (Arduino-ESP32 core 3.x). **ESP8266 (NodeMCU or Wemos D1 mini) is the second choice** (ESP8266 core). In every place where the guide shows two choices, show the ESP32 first.
- The board creates its own Wi-Fi access point (unique SSID such as PT-01, WPA2 password, channel 1, 6, or 11). The page is at `http://192.168.4.1`.
- Pins. ESP32: pan signal GPIO18, tilt signal GPIO19. ESP8266: pan signal D1 (GPIO5), tilt signal D2 (GPIO4). The board's GND goes to the − rail. The 3V3 pin is not used.
- **Two supplies, one shared ground.** The board is powered from the laptop's USB cable (later from a second power bank or a charger). The servos are powered from a USB-C power bank through a 5 V rail. The grounds are joined. The servo rail never comes from the 3V3 pin.
- Breadboard with a + rail (5 V) and a − rail (GND), a 470 µF electrolytic capacitor across the rails (long leg to +, stripe to −), and two 3-pin male header strips for the servo plugs (pins in the order SIG, 5 V, GND). Servo wire colours: orange or yellow is signal, red is 5 V, brown or black is GND.
- Power numbers: the power bank through the breakout gives about 5 V. Micro servos often stall near 0.5 to 0.7 A each, so two can pull over 1 A. Do not use four fresh alkaline AA cells for servos rated 6 V (they start near 6.4 V).
- The bracket is pre-printed. Several free models exist for SG90 and MG90S servos on Thingiverse, Printables, Cults3D, MakerWorld, and CGTrader. Tell the reader to check each model's license, print a fit-test copy first, and keep the firmware limits at least 10 degrees inside the hard stops.
- **Tools and supplies in every kit**: a small Phillips screwdriver (one per two people), screws, and double-sided tape. The screwdriver and screws fit the servo horns and the bracket. The tape sticks the breadboard and the USB-C breakout to the base. Screw sizes depend on the bracket and servos (many designs use small M2 or M3 screws), so tell the reader to check the model's page. Servos usually include a few small screws. Add all three to the kit table.
- Soldering is optional (a 10-minute rotating station: solder a 3-pin header strip onto a small perfboard, three joints). Nobody should be blocked if they skip it.

## The USB-C power source (important details)

A USB-C power bank stays off until it sees a 5.1 kΩ resistor from each CC pin to ground. A bare cut cable has none, so it usually gives 0 V. The kit uses a USB-C breakout board with pins VBUS (the 5 V pin, sometimes labelled VCC, V+, or 5V), GND, CC1, CC2, D+, D−, SBU1, and SBU2.

- Check for resistors with the bank unplugged: CC1 to GND and CC2 to GND should each read about 5.1 kΩ. If not, fit one 5.1 kΩ resistor from CC1 to GND and another from CC2 to GND. Keep them separate. Never join CC1 and CC2.
- Find VBUS by plugging the bank in and measuring with black on GND: the pad that reads about 5 V.
- Attach a red lead to VBUS and a black lead to GND. Leave D+, D−, SBU1, and SBU2 unconnected.
- Confirm about 5 V with the red lead positive. Leave it on for a minute to check the bank does not switch itself off. Test every bank model.
- Instructors prepare this before the workshop. Attendees only measure it.
- If it does not work: a USB-C to USB-A adapter plus an old USB-A cable cut open (red is 5 V, black is GND, insulate the data wires); or four rechargeable AA cells (about 4.8 to 5.6 V) in a holder. Avoid a cut USB-C cable with no resistors. A power-delivery trigger board is acceptable only if fixed at 5 V.

## Firmware behaviour (both sketches)

- Serves the control page from the board itself (two sliders, a centre button). The page sends the target angles every 200 ms. It reads angle limits from `/info`.
- `/aim?p=..&t=..` sets targets. Limits (pan 30 to 150, tilt 50 to 130, centre 90) are enforced in firmware, not only in the page.
- Slew-limited to 90 degrees per second. No instant jumps.
- Failsafe: if no command arrives for 1.2 s, move back to the centre.
- Both servos go to the centre at boot.
- Named constants: pins, SSID, password, channel, limits, invert flags, pulse range (start with 500 to 2400 microseconds), slew rate, failsafe time. No `delay()` in the main loop.
- ESP32: use `ledcAttach` and `ledcWrite` (core 3.x), 50 Hz, 14-bit resolution, `WebServer.h`.
- ESP8266: use the built-in Servo library with `writeMicroseconds`, `ESP8266WebServer.h`, and serve the page with `send_P` because it lives in flash.
- Comment every constant the reader may need to change.

## Tutorial structure (keep this order and these step numbers)

Start with a short subtitle and a prominent warning box saying the material is not tested on hardware, with the specific things to check. Then:

1. **Choose your board**: a comparison table (pins, servo output, steadiness, IDE board name, core, camera add-on possible or not, sketch file name). ESP8266 note: servos can twitch when Wi-Fi is busy, so test early.
2. **Time plan** (table): 0-10 intro, safety, kit tour; 10-25 paper and prompt; 25-45 wire the breadboard with servos unplugged (optional soldering station); 45-55 gated power-up; 55-60 break; 60-80 review the AI's code, flash, load the page; 80-100 plug in servos, centre, fit horns and bracket, set limits, pull-the-Wi-Fi test; 100-112 run without the laptop, free play, and time to fix problems; 112-120 take-home checklist and questions.
3. **Kit** (table, including the Phillips screwdriver, screws, and double-sided tape) and a note explaining why there are two supplies.
4. **Safety in five lines**: nothing moves until the gates pass and horns are fitted only after centring; the board is 3.3 V and the 5 V rail never goes to a pin; capacitor polarity; do not force a powered servo or drive it into a stop; unplug the bank when changing wiring and keep fingers clear at first power-up.
5. **The 5 V source**: the explanation, the USB-C breakout diagram, instructor preparation steps, and the fallback options table.
6. **Step 1, paper first**: the ESP32 diagram, the ESP8266 diagram, a pin table covering both boards, and a power budget as tick boxes. Gate: pin table and power budget filled in.
7. **Step 2, prompt the AI**: a copyable prompt for each board, containing the exact board, the pins, the supply facts, the limits, the requirements, and a request to list assumptions before the code. Add a note about expected AI surprises (older ESP32 PWM functions that do not exist on core 3.x, a servo library not installed, `send` instead of `send_P` on ESP8266, servo pulse ranges differing).
8. **Step 3, wire the breadboard**: tick boxes that start with sticking the breadboard and the USB-C breakout to the base with double-sided tape (power off), and end with a continuity test (no beep between + and − rails, none between a signal pin and +). Gate: every wire matches the pin table and there is no short. Include the optional soldering station note.
9. **Step 4, gated power-up with servos unplugged**: board on USB and LED on; 3V3 about 3.3 V; plug in the bank and measure the rails at about 5 V; wait 30 seconds and touch the parts; unplug the bank. Gate: readings as expected, nothing warm.
10. **Step 5, review, flash, load the page**: a seven-item review list, board settings for both boards, Serial Monitor at 115200 (ESP8266 may print garbled boot text first), join the unit's Wi-Fi, open the address. Gate: the page loads and shows the firmware limits.
11. **Step 6, centre, fit, calibrate**: plug servos in, power up, servos jump to centre (fingers clear), fit horns and bracket with the small screws and the Phillips screwdriver, straight and level, move sliders slowly and listen for buzzing, record where the bracket touches, set limits at least 10 degrees inside, flip invert flags instead of rewiring. Gate: both sliders run end to end with no buzzing, strain, or warm servo.
12. **Step 7, break it on purpose**: cut the Wi-Fi and check it returns to centre; press reset; jump a slider end to end; ask the AI what happens at 400 degrees and test it; write down one thing the AI got wrong. Gate: cut the link and it returns to centre, reboot and it stays at centre.
13. **Running without the laptop**: the sketch lives in flash and starts on power-up; use a second bank or charger for the board; keep the ground joined; power the servos first; some banks switch off at low draw; no Serial Monitor; repeat the failsafe test; never power the board from the servo rail.
14. **Troubleshooting table** covering upload failures, ESP32 compile errors, no voltage on the servo rail, power bank shutting off, servo jitter, ESP8266 servo twitching, ESP8266 crash on page load, board resets when servos move, buzzing at an end, nothing moves, wrong direction, returns to centre on its own (that is the failsafe), and the page not loading.
15. **Print notes for the bracket** (including checking which screws and nuts the model needs), **take-home upgrades**, **both reference sketches** (with copy buttons), and an **instructor prep checklist** (printing, packing the screwdrivers, screws, and double-sided tape, preparing the breakouts, header strips, labelled SSIDs and channels, a 10-unit room test, venue rules for soldering, spares, setup email a week ahead).

## Diagrams (three, drawn as inline SVG and embedded as images)

All on a white background, with clear labels and no overlapping text.

- **Wiring diagram (ESP32)**: laptop USB to the ESP32; GPIO18 (orange wire) to the pan strip signal pin; GPIO19 (blue wire) to the tilt strip signal pin; GND (black) to the − rail; the USB-C power bank through the breakout board to the rails (red lead to the + rail, black lead to the − rail); the 470 µF capacitor across the rails; the two servo header strips with SIG, 5 V, and GND pins, each wired to the signal pin, the + rail, and the − rail. Include a legend for the wire colours and a "key points" box (board powered by the laptop, servos powered by the bank, only the grounds shared, never feed servos from 3V3).
- **Wiring diagram (ESP8266)**: the same layout with D1 (GPIO5) for pan and D2 (GPIO4) for tilt, and the board labelled NodeMCU or D1 mini.
- **USB-C breakout diagram**: the breakout board at the centre with a power bank connected by a USB-C cable; the pins VBUS, GND, CC1, CC2, D+, D−, SBU1, and SBU2; VBUS to a red lead going to the + rail; GND to a black lead going to the − rail; a 5.1 kΩ resistor from CC1 to GND and another from CC2 to GND, shown as separate branches; D+, D−, SBU1, and SBU2 marked as not connected; a "why the resistors" box; and a "check before connecting servos" box (CC resistance with the bank unplugged, no beep between VBUS and GND, about 5 V once plugged in).

## Design requirements for the tutorial

- Plain, readable layout: serif headings, sans-serif body, monospace for code. Works in light and dark mode. Prints cleanly.
- Amber callout boxes for each "Gate". Tick boxes for checklist items. A warning box for the "not tested" notice.
- Copy buttons on the prompts and sketches (with a fallback if the clipboard is blocked).
- Embed the diagrams as images (for example data URIs) so page styles cannot change their colours.
- Sentence case headings. Short sentences. No filler.

## Checklist requirements (the fourth deliverable)

One HTML page for this project only, with a progress bar and a reset button. A small table of the pins for both boards at the top. Six phases, each ending in a gate: **Paper first**; **The 5 V source (USB-C breakout)**; **Wire and check**; **Read the AI's code**; **Centre, fit, calibrate**; **Break it on purpose** (including running without the laptop). Use the facts above. Do not include checklists for a camera robot, an agentic robot, a generic template, or any other project.

## Honesty rules

- State clearly that nothing has been tested on hardware. Do not claim otherwise.
- Do not invent product names, part numbers, prices, or links. If a detail depends on the reader's parts (pin labels, wire colours, servo pulse range, breakout layout), say to check it and measure.
- Before writing, list any assumption you are making. After writing, check that every pin, voltage, and limit is the same in the tutorial, both sketches, all three diagrams, and the checklist.
