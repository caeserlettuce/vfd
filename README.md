# vfd

a custom python module that talks to a connected microcontroller in order to drive a vacuum fluorescent display

note: originally this repo was just going to have the python module, but i realised how easy it would be to also just include my microcontroller code along with it. especially after trying to figure out how all of this works. so this README is separated into two sections, a python and a c++ section. oh, and a subsection under python for the displays.json and the development webapp.

<br>
so the python script, as mentioned, is a module that you can import into whatever other script you want to use to control what the display says.

<br>

fair warning, not *everything*  will work directly out of the box. i expect you will have to change values in some of the scripts in order to make things work for your display.

the python module, i have tried to make as dynamic as possible, and will be the least of any issues.

the development webapp is separate from the module functionality, and is just there to help you debug any python-related issues. it's also a tool to help make glyphs for your display. (trust me, it made it 10x faster and i had the whole font done in 10 minutes)


<br>

the microcontroller code is written for a teensy 3.2, and the shift register is an HV518. Thank you to MikeDombo for writing [this repo](https://github.com/MikeDombo/HV518_Arduino) which i studied in order to figure out how to actually drive the display. I'm writing this part of the readme before i've actually started writing the final teensy code, so i will try my best to make it as modular as possible.


<br>
please refer to each individual section for limitations and usage (of course)


## todo
- [ ] actually implement everything below
- [x] write a "font" for my vfd's pinout
- [ ] make it connect to an arduino/teensy via serial, then make it so you set that up within the init function
- [ ] see if it actually works with the display

## use (python)

```python
import vfd

display = vfd.VFD({"model": "CM1564C"})     # create new VFD with model CM1564C

display.setBrightness(4)                    # set brightness of display
display.getBrightness()                     # returns display brightness

display.verifyText("insert text")           # properly formats the input text for displaying
display.setText("insert text")              # simple set text on display

display.setPins("19:17,19:16")              # set specific pins (string method)
display.setPins(["19:17", "19:16"])         # set specific pins (list method)

display.clearPins("19:17,19:16")            # clear specific pins (string method)
display.clearPins(["19:17", "19:16"])       # clear specific pins (list method)

display.clearGrids("19, 20, 21")            # clear specific grids (string method
display.clearGrids([19, 20, 21])            # clear specific grids (list method)

dispay.getPins()                            # returns current pin address list

display.clear()                             # clear all pins
display.send()                              # sends pin address list to screen

vfd.scrollText("hello world!", 8)           # creates a list of address lists, basically an animation of the text scrolling


```

<br>

### display.setPins() and display.clearPins()

```python
display.setPins(pins_in: str|list, apply: bool)
display.clearPins(pins_in: str|list, apply: bool)
```
arguments for both of these functions are identical.

`pins_in` can either be a `str` or `list`, like shown above.

`apply` determines if the pin address list is automatically changed or not. this is set to `True` by default.

returns a `list` of the new changed pin list.

<br>

### display.setText() and display.verifyText()

```python
display.verifyText(text_in | str, font_in | str)
display.setText(text_in | str, apply | bool, font_in | str)
```

always use `display.verifyText()` *before* `display.setText()`.
`verifyText()` returns a string that is properly formatted for inputting to setText.

`display.verifyText()` removes any characters that are not inlcuded in the selected font, and will flatten down any non-included unicode characters as well.

<br>

### display.setBrightness() and display.getBrightness()

```python
display.setBrightness(brightness | int)
display.getBrightness()
```

`display.setBrightness()` will set the display brightness to any even number between `0` and whatever the maximum brightness is set to in `displays.json`.

`display.getBrightness()` just returns the display's brightness.

<br>

### display.getPins()

```python
display.getPins()
```

returns the pin address list.

<br>

### display.clearPins()

```python
display.clearPins()
```

clears the pin address list.

<br>

### scrollText()
```python
vfd.scrollText(text: str, length: int)
```

outputs a list of address lists, basically like an animation of the text scrolling. `length` is how many characters max can be displayed before scrolling

<br>

## displays.json

have a different vfd that's not in here? `displays.json` is your answer.
<br>
theoretically, if i have written my code right, you should be able to just add a new entry to `displays.json` (albeit with a large amount of setup) and then you can have this module work for your own VFD.
<br><br>
this module is designed for VFD's that are controlled by three pins (anode, grid, and cathode)
<br><br>
if your VFD doesn't have grids, that makes things easier. just lie to the program and make the program think everything is on grid 0. easy.
<br><br>
if your VFD has more pins than just anode, grid, and cathode; good luck! 
<br>setting the individual pins should work fine, but i'm not sure if the text creation scripts will work. maybe in the future i'll make sure that works.

<br>semi-future me here: i'm not going to actively be adding support for multiple grids, however, i'll try not to make it a pain for anyone else to add support.

<br>
okay so now to the json file. let's go through the properties:

```jsonc
{   // as an example, im using the properties for my CM1564C.
    "CM1564C": {    // this is the name
        "address support": [
            // these are the allowed pin numbers for each place in the pin address.
            // each list within this one represents a place in the address.
            // for example, if i had three lists in here, the address could be formatted:
            // pin:pin:pin
            // ^   ^   ^ list #2
            // |   \ list #1
            // \ list #0
            //
            // the numbers contained inside of these lists are the pin numbers that are allowed within that part of the address, if that makes sense
            // for instance:
            // list #0 are all of the grid pins on the VFD
            // list #1 are all of the anode pins on the VFD
            //
            // though, in reality, you can have them be in whatever order you want them to be in.
            // the only reason i have this is just so that the program knows if the wrong numbers are in the wrong places.

            [18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29],
            [2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17]
        ],
        "max brightness": 4,    // this number can be pretty much whatever number you want it to be, it basically just determines how many steps there are between minimum and maximum brightness.
        "fonts": {
            // fonts is the fun part. this is both a way you can have different typefaces programmed in,
            // but also a way to have multiple different text generators for different parts of the display.
            // 
            // for example, my VFD has 8 14-segment digits, but it also has two 7-segment digits off to the right.
            // the default font is written for the 14-segment digits, and then i can write a separate font for the 7-segment digits.
            // and i can also write multiple fonts for each one.
            // go crazy

            "default": {
                "grids": [19, 20, 21, 22, 23, 24, 25, 26],
                // these are all the grid pins that the text will be in. make sure these are in order of how they appear visually on the VFD, because the text function writes characters based on the order of this list.

                "replacements": {
                    "A": "T"
                    // this entry is not in the actual font, but is just an example.
                    // if you have multiple characters that use the same pattern (like 0 of O for instance),
                    // just define one of them down below in glyphs, and then up here you can define a replacement.
                    // in this example, al instances of "A" will be replaced with "T"
                },
                "missing": {
                    "29": [9, 10]
                    // this entry is also another example one.
                    // above i lied about the two digits being 7 segment displays.
                    // the first of the two actually has two extra vertical segments in the center. other than that, both digits are identical.
                    // for this, i didn't want to create two separate fonts, so i came up with this.
                    // the extra two segments are controlled by pins 9 and 10.
                    // the pin that controls the grid of the digit that's missing those sections is pin 29.
                    // so basically, for every grid with missing pins, it has the list of those missing pins.
                    // the setText function will see that, and if a it tried to display a character that can't be displayed on it, it will just skip that character and move on to the next one.
                    // the only downside to a font having missing segments is that you can't use the scrollText function on that font. because that's way too much to code, and i don't want to.
                },
                "caps": false,  // if you set this to true, any text inputted will be automatically capitalised
                "glyphs": {
                    // here's the fun part 2: electric boogaloo
                    // these are the glyphs!
                    "unknown": ["?"],   // i dont think this needs to be here anymore but im too lazy to remove it
                    " ": [],    // space
                    "a": [10,11,13,14,15,16,17],
                    // here's the fun stuff:
                    // the numbers in this list are the pin numbers for all of the segments needed to create the specified character. this does mean though that all of the grids' segments' pinouts are identical. which, i would hope most of them would be.

                    "b": [10,11,12,13,14,15],
                    "c": [10,11,13,14]
             
                }
            },
            
        }
    }
}

// i hope that was a fun read. if you would like to see a fully implemented font, just go take a look in displays.json.

```

## debug tools

i have created debug tools along with this module.

now, directly out of the box, i'm sure they will not be helpful to anyone, unless you have the exact same VFD's i do.

`visualizer.html` has an interactive SVG of my VFD(s) (i only have one at the moment, but i will probably be reusing these tools when i eventually get more)

that interactive SVG lets me select specific segments of the display, and then it will output a list of the corresponding pin addresses.

if you enable the `exclude grid numbers` toggle, it will output a list of the pins without the grids, so that you can just copy and paste that into the glyph definitions.

there are also keyboard shortcuts listed at the bottom of the page.

<br>
the other debug thing i did, is if you have flask installed, you can run `flask run` in the `tools` directory, and it will start a webapp that talks to both the html file, and to the vfd module.

in `vfd.py`, you need to set `WEBAPP_DEBUG` to `True` in order for the webapp debug to work. and you need to enable it on the webpage. but it *should* talk to the vfd module, and when you run `display.send()` it should send that to the webapp, where it'll be displayed on the page.

### modifying the page

i didn't really write this webpage with easy modification in mind. what that means is, i wrote it to work for what i was doing in the current moment. but it shouldn't be *too* difficult to modify.

```html
<g>
  <g pin="4" grid="19" style="transform: translate(1080px, 1125px)">
    <image x="0" y="0" href="assets/19-4.png"></g>
  <g pin="5" grid="19" style="transform: translate(1231px, 1125px)">
    <image x="0" y="0" href="assets/19-5.png"></g>
  <g pin="6" grid="19" style="transform: translate(1170px, 1111px)">
    <image x="0" y="0" href="assets/19-6.png"></g>
  <g pin="7" grid="19" style="transform: translate(1118px, 895px)">
    <image x="0" y="0" href="assets/19-7.png"></g>
  <g pin="8" grid="19" style="transform: translate(1250px, 895px)">
    <image x="0" y="0" href="assets/19-8.png"></g>
  <g pin="9" grid="19" style="transform: translate(1192px, 902px)">
    <image x="0" y="0" href="assets/19-9.png"></g>
  <g pin="10" grid="19" style="transform: translate(1224px, 1071px)">
    <image x="0" y="0" href="assets/19-10.png"></g>
  <g pin="11" grid="19" style="transform: translate(1077px, 1071px)">
    <image x="0" y="0" href="assets/19-11.png"></g>
  <g pin="12" grid="19" style="transform: translate(1048px, 853px)">
    <image x="0" y="0" href="assets/19-12.png"></g>
  <g pin="13" grid="19" style="transform: translate(1018px, 1108px)">
    <image x="0" y="0" href="assets/19-13.png"></g>
  <g pin="14" grid="19" style="transform: translate(1030px, 1308px)">
    <image x="0" y="0" href="assets/19-14.png"></g>
  <g pin="15" grid="19" style="transform: translate(1311px, 1107px)">
    <image x="0" y="0" href="assets/19-15.png"></g>
  <g pin="16" grid="19" style="transform: translate(1335px, 854px)">
    <image x="0" y="0" href="assets/19-16.png"></g>
  <g pin="17" grid="19" style="transform: translate(1085px, 832px)">
    <image x="0" y="0" href="assets/19-17.png"></g>
</g>
```

i know this looks daunting, but just trust me.
<br> this is a copy of one of the 14-segment digits.
<br><br>
the only thing that the javascript cares about is this structure:

```html

    <g pin="17" grid="19">
<!--        ^ pin #   ^ grid #       -->
<!--   (remember: the default format of the pin addresses are like this -> grid:pin ) -->
      <image x="0" y="0" href="assets/19-4.png">  <!-- the image filename doesn't matter either. -->
    </g>

<!--
    basically the most basic bare bones of what you need is:
    an <image> inside of a <g> ,
    and that <g> has to have the attributes "pin" and "grid"
    set to that segments corresponding pin and grid number.
    all the script does is makes all the segments grayscale
    except for the ones that should be on.
-->
```

the only value that you should have to edit in the javascript should be `max_brightness` at the top of the `<script>` block.

this number should match whatever you set the max brightness to in `displays.json`


## use (c++)

i have yet to write the final code so this section is blank for now









<br>
i think thats everything i need to write down

