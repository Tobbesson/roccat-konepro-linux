# roccat-konepro-linux
Change dpi, lighting and stuff for roccat kone pro mouse in Linux
# Build
Install dependency: https://libusb.info/ You can probably download it using your distros package managert\
Then run: \
`git clone https://github.com/Tobbesson/roccat-konepro-linux.git \
test`
# Usage
Run as sudo or setup a udev rule.\
`konepro ARGUMENTS`
# Arguments and example
`-prf x` // x = 0-4 // Specify which profile to change. Defaults to profile 0 if not specified.\
`-l R G B` // Change RGB(Range of 0-255) of Left click\
`-r R G B` // Change RGB(Range of 0-255) of Right click\
`-lm x` // x = 0-4,9,10 (0=Off, 1=Fully lit, 2=Blinking, 3=Breathing, 4=Heartbeat, 9=Aimo Intelligent ,10=Wave) // Change LED mode\
`-lb x` // x = 0-255 // Change LED brightness\
`-ls x` // x = 1-11 // Change cycle speed of some LED modes\
`-list profile` // profile = 0-4 // List current values of profile in question\
`-p x` // x = 0-3 (0=125,1=250,2=500,3=1000)// Change Polling rate\
`-d x` // x = 50-19,000(in increments of 50) // Change DPI(Currently only Switch 0 can be changed)\
`-ds x` // x = 0-4 // Change DPI switcher\
`-default` // Factory reset device\
`-dbt x` // x = 0-10 // Sets debounce time in milliseconds (Global setting not profile specific)

Example:\
`konepro -l 255 0 255 -r 0 255 0 -lm 1` // Sets Profile 0 left click to purple, right click to green and LED mode to fully lit.\
`konepro -prf 1 -l 255 0 0 -lm 2 -dpi 800` // Sets Profile 1 left click to red, LED mode to blinking and DPI to 800\
`konepro -list 3` // list settings in profile 3.
