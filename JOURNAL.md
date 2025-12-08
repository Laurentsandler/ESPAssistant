# Development Journal

**Total Time Spent: 16.9 hours**

---

## December 7, 2025

Designed a case with space for a small battery, about a couple of millimeters thick. I added flexible pieces on the case to push the buttons, which should make it more functional while keeping the design compact. Added the STL files to the repository so anyone can print their own case.

**Files added:**
- `Case with space for battery.stl`
- `Case lid.stl`
- Case mockup images in the Images folder

![Case mockup bottom view](Images/Case%20mockup%20bottom%20view.jpg)
![Case mockup side view](Images/Case%20mockup%20side%20veiw.jpg)
![Case mockup top alternate view](Images/Case%20mockup%20top%20alternate%20view.jpg)
![Case mockup top view](Images/Case%20mockup%20top%20view.jpg)

---

## ~5 days ago - Finalizing and getting PCB manufactured (2.0 hours)

Finally finalized the PCB design! I decided to add a fourth button to make the interface more flexible. Made all the buttons low-profile to keep the device as thin as possible.

I removed the expandable I/O that I had considered earlier. I thought about using a small I/O expansion chip, but decided against it to keep things simple - this is my first PCB project after all, and I want to make sure I can actually get it working before adding complexity.

Ordered the PCB today! It should arrive in 5-6 days. Once it arrives, I'll need to solder all the components and finish up the code and custom UI. Excited to see how it turns out!

---

## ~18 days ago - Tested circuit with new board (1.3 hours)

The Seeed Studio XIAO ESP32S3 board arrived today in a really tiny package - I was amazed at how small it is! Connected it to the components on my breadboard and uploaded the code.

Ran into some issues right away because this board has a different configuration compared to the other ESP32 dev kit I was using before. Had to spend some time fixing the configuration in the code, but once I got that sorted out, everything worked perfectly!

One nice surprise: the upload speed is much faster on this board. That's going to save a lot of time during development.

---

## ~19 days ago - Redesigned the PCB (2.0 hours)

Redesigned the PCB to use holes with pins instead of direct soldering for the microcontroller. This should make it easier to replace the microcontroller if needed, and also makes the assembly process more forgiving.

Got some inspiration from another mini computer I saw on Instagram (it had a Kickstarter campaign). It made me think about what features would make this more versatile.

I wanted to add expandable GPIO at the top for servos or sensors - that would be really cool for future projects. I also wanted to use a right-angled female socket connector, but JLCPCBA didn't have it in stock. Had to settle for a low-profile flat design that was available. It's not exactly what I wanted, but it should work fine.

Still considering adding a pogo pin connector at the back for additional expandability. Not sure if it's worth the added complexity yet.

---

## ~20 days ago - Designing the PCB (4.0 hours)

Spent the day designing the PCB in EasyEDA while waiting for the components to ship. This is my first time designing a PCB from scratch, so there's definitely a learning curve. I'm watching tutorials and reading documentation as I go.

The plan is to have JLCPCB manufacture the PCB after I test the circuit on a breadboard. Don't want to order a PCB that doesn't work! Better to validate the design first.

---

## ~20 days ago - Researching for smaller components (0.5 hours)

Realized that my original design was way too big to be practical. If I want this to be something I can actually carry around and use throughout the day, it needs to be much smaller.

Did some research on smaller ESP32 microcontrollers and found the Seeed Studio XIAO ESP32 S3 board. It's incredibly small and has a built-in charging circuit, which is perfect for what I need!

Here's what I'm planning for the final parts list:
- Seeed Studio XIAO ESP32 S3 (very small with built-in charging)
- Small INMP441 microphone
- Small 3.9" diagonal OLED screen
- Three small buttons

I already had all the components except the microcontroller, so I ordered that on Amazon. Should arrive in a couple of days.

---

## ~20 days ago - Designed and 3D printed prototype (2.0 hours)

Designed a case in Onshape to test the prototype in the real world. Wanted to see how it actually feels to use this thing before committing to the final design.

3D printed the case and assembled all the components. Took it with me throughout the day to test it in real-world conditions. This was the moment of truth - would the battery last? Would it be practical to actually use?

Results: Battery lasted about 11.5 hours without stopping! That's way better than I expected. The concept is proven - this can actually work as a daily carry device!

---

## ~20 days ago - Built Large prototype (5.0 hours)

Built my first working prototype using whatever parts I had available on a proto board. It's not pretty, but it works!

Components I used:
- Small battery
- PMS board
- Microphone
- Two buttons
- OLED display
- ESP32 dev board

Successfully got it to:
1. Record speech
2. Transcribe it (using Groq Whisper API)
3. Send to AI (Groq Llama 3.1)
4. Display the response on screen

The code is rough and the hardware is bulky, but seeing it work for the first time was incredibly satisfying. This proves the concept can work!

---

## ~20 days ago - Making the idea (0.1 hours)

Had the initial idea for this project. I wanted a small computer that could help me throughout the day with AI assistance. 

The input would be through a microphone, buttons, and a small display - keeping it simple and focused.

My main use case is for school: I want to be able to track assignments from teachers. The workflow would be:
1. Press a button
2. Record my voice describing the assignment
3. Have it transcribed automatically
4. Send it to a custom website that organizes my assignments and notes

This could be a game-changer for staying organized!
