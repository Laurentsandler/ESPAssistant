# Development Journal

**Total Time Spent: 23.0 hours**

---

## Portable Aesthetic Phone Project

*Started January 29, 2026*

### Making the Idea (0.4 hours)

I wanted to build a small phone with numerous sensors and outputs. I wanted to do this because if I build my own, I have complete control over the UI, apps, and how it functions.

After doing some research, I was inspired by **The Light Phone** - a minimalist phone with an e-ink display.

The only problem was that it had an e-ink display, while I wanted a full-color high-refresh-rate display.

**Design Requirements:**
- About 3.5-inch display
- Super modern and cool look
- Relatively long battery life
- Full-color high-refresh-rate display (not e-ink)

---

### Research - Choosing the Development Board (2.0 hours)

When picking out the parts for this project, I wanted to make sure that anyone who came across this could easily make it even if they had very little experience. So I decided to go with a pre-built module combining everything I needed instead of building a custom PCB. So I set out on Amazon researching for the perfect module. It took me a very long time to pick out a module. I wanted one with the most functionality, and there are a lot.

I finally decided on this one:

**Waveshare ESP32-S3 3.5 inch Capacitive Touch Display Development Board**

It was perfect. It had:
- Loads of sensors
- Built-in speakers
- Built-in camera
- Battery headers
- ESP32-S3 architecture (which I am very familiar with)
- Expandable I/O
- From a reputable company (Waveshare)

---

### Finding a Suitable Battery (0.2 hours)

It was going to be very easy to add a battery into this project. Since the dev board already had battery headers. I wanted the battery life on it to be very long. I settled on this battery which was easily accessible on Amazon:

**EEMB Battery 3.7V 2000mAh 103454 Lipo Rechargeable Battery Lithium-Ion Polymer Battery with JST Connector**

It had pretty good mAh (2000mAh) and was 3.7V exactly as needed for the dev board.

---

### Designing the Case (3.5 hours)

I wanted to make sure the case had enough space for a battery and a little button to press on the buttons on the dev board. I was going to print it in a nice dark blue color. That suits the black of the display. I found a starting point of a much smaller case for this same dev board, but it didn't have space for a battery and it had some MagSafe rings, which I didn't need. I spent a couple of hours adding some peripheral holes for the microphone and speaker, extending it to have space for the battery, and just tweaking it.

Then I printed it on my Ender 3 S1 to make sure it was actually printable without supports, and that people wouldn't have trouble printing it.

**Hardware needed:**
- 4x M2x8 Countersunk Screws (to secure the case)

---

## Previous ESPAssistant Development

## December 7, 2025
I designed a case and included space for a small battery, a couple of millimeters thick, and some flexible pieces on the case to push the buttons. Some photographs are attached below. And I added the STL files to the repository.



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

I finalized the PCB I added a fourth button and made all of the buttons low-profile ones. And remove the expandable I/O. I considered using a small I/O expansion chip, but since this was my first time making a PCB, I wanted it to be as simple as possible. So I decided against that.

I should get the PCB in 5-6 days. Once I get it, I'll solder on the components. In the meantime, I'll need to finish writing the code for it and designing a custom UI.

---

## ~18 days ago - Tested circuit with new board (1.3 hours)

Today my Seeed Studio XIAO ESP32S3 board finally arrived. It came in this small little tiny package. So I connected it to all my components on a breadboard and uploaded the code. But some things didn't work. Turns out the seeed studio board has a different configuration than my other ESP32 dev kit. So I changed that, and once I changed it, everything worked without problems. Also, I noticed that the upload speed on this board is way faster. It can upload stuff a lot faster than my other board.

---

## ~19 days ago - Redesigned the PCB (2.0 hours)

Today I decided to redesign the PCB a little bit because first I realized that it was gonna be pretty hard to directly solder on the microcontroller onto the PCB instead of just using pins. So I redesigned it to use holes where I can put pins in, and then the microcontroller on top to connect it to the PCB. Also, I got some inspiration from another mini computer I saw on Instagram, they also had a Kick starter:

I saw that it has really easy expandable gpio at the top, which I thought would be really nice to have incase i wanted to connect some servos or maybe a temp/humidity sensor. I wanted to use a right angled female socket connector, but sadly jlcpcba did not have any in their library of parts, and I didn't want to solder this one on myself. So I settled on a low-profile flat design. Which JLC PCBA had in stock.

I will definitely take more inspiration from this really cool product. Maybe I'll add the little pogo pin connector at the back.

---

## ~20 days ago - Designing the PCB (4.0 hours)

While the component was shipping, I set out to design my PCB so the whole construction would be smaller. I used easyeda to design the PCB.

Once I get the small microcontroller and test out the circuit on the breadboard and make sure it works, I will have JLC PCB manufacture the PCB.

---

## ~20 days ago - Researching for smaller components (0.5 hours)

Although this design worked, it was way too big to be practical. So I wanted to make it a lot smaller, but with my current ESP32 dev board and my current setup, it was just too big.

So I set out to find a smaller microcontroller.

I researched for a small esp32 microcontroller that I can use, as I am familiar with esp32 boards. I found the Seeed Studio XIAO ESP32 S3 board, it is really small! I found out that it has a built in charging circut, so I will not need an external one, thats good news I planned that the parts I will need are as follows: - A small INMP441 microphone - A small 3.9 diagonal OLED screen - Three small buttons I already have all of these components. Except the microcontroller which I ordered on Amazon.
---

## ~20 days ago - Designed and 3D printed prototype (2.0 hours)

I wanted to test how the prototype worked in the real world, I designed a case for it in on shape. I had put all the components in the case once I 3D printed it and took it with me on my day. It was really useful and without stopping, the battery lasted about 11.5 hours, so
the concept was proven.
---

## ~20 days ago - Built Large prototype (5.0 hours)

Built a working prototype using the parts I already had, and I then programmed it. It has:

A small battery
A PMS board
A microphone
Two buttons
An OLED display
An ESP32 dev board All connected to a proto board.
It successfully recorded my speech, transcribed it, sent it to AI, and then showed me the response back.

---

## ~20 days ago - Making the idea (0.1 hours)

So, I wanted a small little computer that will help me throughout the day.

I wanted it to have AI. And also an input using a microphone and some buttons and a small display.

I am in school, so I wanted it to help me track assignments as they come in from my teachers. So, I want to when I press a button, it will record my voice, transcribe it, and then have it sent to a custom website to be added to my assignments or notes, etc.



