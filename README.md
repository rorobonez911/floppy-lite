# floppy-lite
Simplified MIDI Controlled Floppy Drive Controller Compatible with IDE Floppies (up to 6)



# HOW TO SETUP

***Requirements***
1. IDE Floppy Drive(s)
2. Arduino Microcontroller (Mega, UNO, or similar (not sure which ones don't quite work.. please send issues if you have problems with any specific arduino board!) )
3. ATX Power Supply (With Berg Connectors for the Floppy drives)


***Build Instructions and Hardware Setup***

1. Get your ATX power supply and use a small jumper wire to ground pwr_on (**green wire**) pin to any com (**black wire**) pin. If you are unsure about your power supplies pinout Search for an ATX power supply pinout or use this one: https://www.smpspowersupply.com/connectors-pinouts.html

2. Connect all your Floppy Drives to power with berg connectors to your power supply like you would any drive.

3. For this part **use this pinout:** https://old.pinouts.ru/HD/InternalDisk_pinout.shtml now with a jumper wire connect the /DRVSB	-->	Drive Sel B pin to any ground pin (odd pins) this should make the drive LED light up when powered on.  ***DO NOT MISS THIS STEP OR YOUR DRIVE WILL NOT PLAY!!**

4. Connect the STEP pin of your first drive to pin 2 on your arduino and the Direction pin to pin 3, then for the next drive step to pin 4, direction to pin 5, and the same with the next 2 pins for the next drives.

5. Upload my sketch (floppy.ino) to your Arduino you should hear some startup noises and if you have your drive cover off you may also see the head move all the way to one side (this is to make sure the drive is at a starting point)

6. Open the Python GUI and select your Arduino's COM port, and a MIDI Port. MIDI data sent over the selected MIDI device will control the drives.

**MIDI channel pin mapping**

pins 2/3 --> Channel 1

pins 4/5 --> Channel 2

pins 6/7 --> Channel 3

pins 8/9 --> Channel 4

pins 10/11 --> Channel 5

pins 12/13 --> Channel 6
