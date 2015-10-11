# timelapsr

Time lapse remote control for NEX cameras.

## Installation

You need these Arduino libraries in order to flash the code:

1. Install the IR command [library](http://sebastian.setz.name/arduino/my-libraries/multi-camera-ir-control/) from Sebastian Setz.
2. Install the 7 segment display [library](https://github.com/sparkfun/SevSeg) from Sparkfun.

## Use

Compile and upload the code to your Arduino or Pro Trinket (or any compatible device). Start using the interface.

*Important* Make sure your NEX camera is in _remote_ mode.

Currently, there are three menu options:

1. time lapse (TL)
2. bulb mode (LS)
3. fast time lapse (FTL)

The following submenus exist:

1. DUR: set frame interval (number of seconds between frames)
2. RUN / FOR: set duration of recording
3. SHUT: time the shutter should remain open

Increase the value (if possible) with the up button, decrease with down button. If the correct number has been reached, confirm with right button.

In order to correctly read the numbers displayed, you must be aware of the following convention: the number is always in seconds except if there is a decimal dot active. In that last case, the number displayed is in hertz. When transitioning from one unit to the other, the new unit will be displayed.

While in record mode, you can use the right button to deactivate the display. The left button cancels the recording and returns to the main menu.

## Wish list

* Add additional camera control options: video mode, presentation etc.
* Add multi-camera support.
* Use the left key in any menu to return to the previous menu.


## Components

See `component-list.md`.

## Contribute

Fork this repo and send a pull request if you have an enhancement or bug fix.

## License

See `LICENSE` file.
