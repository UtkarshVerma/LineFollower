# Line Follower 
![The front view](/Pictures/v2/DSCN2881.jpg)

This repository contains the relevant files for a Line Follower I made. It is based on PIC16F84A and is somewhat inherited from this great [instructable](https://www.instructables.com/id/Line-Follower-Robot/) by **Mohammad Yousefi**.
I'd like to add that the schematic file is built using [Proteus](https://www.labcenter.com/) and most of the boards have also been designed using it, except for the extension PCBs, which have been designed using [eagleCAD](https://www.autodesk.com/products/eagle/overview).
---
## The Differences
The robot I made differs by large from the one made in the instructable. So **you'll have to be a bit careful** if you're thinking of merging both these projects. The differences are as follows:
* I wanted to use Lithium-ion batteries to power this robot up, so I dropped the whole messy AA battery system;
* I've used a Chinese motor driver named **Mini L298 Motor Driver** rather than the **bulky L298 module or IC**. Luckily there's already an [instructable](https://www.instructables.com/id/Tutorial-for-Dual-Channel-DC-Motor-Driver-Board-PW/#discuss) for it.
    > However, there's no issue since the input mechanisms are identical in both, except for **PWM**.
* I've integrated the battery connection toggler, [**Boggler**]() in my project to enable clean and easy toggling between charging and usage modes.
* I've made the PCBs **detachable** using connectors and extension PCBs. This aids further modification of the robot, so you can build upon it easily in the future.
* The inner part of this robot has been left intentionally empty for **allowing the addition of more circuits**.
* The code for this robot has been worked up from scratch in **C programming language**, and due to this the line detection algorithm is also different.
* There's **no PWM functionality in the original code** posted in the instructable, so I added it in my code for implementing speed control for the motors to **allow a variety of motors** to work with this project.
* Instead of ironing the toner onto the top side of PCB for the silkscreen, I've pasted a white paper printed with the **mirrored silkscreen**.

---
## The Code
The code is written in **C** as stated earlier. It has been written to allow easy modification through variable values. The configuration variables are stated below:
1. `normPWM` : The PWM duty for **forward motion**.
2. `fastPWM` : The PWM duty for **extreme turns**.
3. `slowPWM` : The PWM duty for **slow turns**.
Modifying these values affects the motor speed during their respective phases.

### Algorithm
To detect lines, this robot does the following:
1. Store the value of the middle sensor in `lColour` to detect the line colour accordingly.
    **0 - Black line, 1 - White line**.
2. Begin forward motion.
3. If one sensor on either side(i.e. left or right sensor) strays off and its value differs from `lColour`, then **slow turns** have to be made towards the opposite side.
4. If one sensor on either side strays off and its value equals to `lColour`, then **extreme turns** are to be made towards the opposite site.
5. If the front sensor and a sensor on either side strays off, then a **perpendicular turn** is detected, so the robot spins in the corresponding direction. The thing to be noted here is that no the motors run at full speed during this phase and no PWM is used.

When the mode switching button is pressed and custom mode is enabled, the robot **reads last state values from the EEPROM** of PIC16F84A. While exiting the custom mode, it also **writes the contemporary state values to the EEPROM**. The memory locations used for this purpose are stated below:
* `0x00` - Stores whether the robot was turned off during custom or default mode.
* `0x01` - Stores the last settings of the left motor in custom mode.
* `0x02` - Stores the last settings of the right motor in custom mode.

This concludes the code section. I won't be going more into the code details since more than sufficient comments have been made there.

---
## Some Notes
Though there aren't much, here are some things you have to look out for while making this project:
* The motor driver used is different so **you can abandon the back PCB** if you wish. I have wired up mine using wires alone and pasted the module onto the PCB onto the back PCB using glue.
* The extension PCBs have to be well soldered with their respective boards since they undergo huge stress while detaching the PCBs.
* The extension PCBs also serve as **PCB-jumpers** to avoid the jumpers between the top and bottom boards. So, you'll have to make the jumper connection through these extension boards so as to allow the detaching of boards.