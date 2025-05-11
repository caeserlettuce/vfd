# vfd

a custom library to help drive a vacuum fluorescent display

disclaimer: this library doesnt *actually* drive the display, it basically is a middle man between my own python script and the arduino.

## todo
- [ ] actually implement everything below
- [ ] write a "font" for my vfd's pinout
- [ ] make it connect to an arduino/teensy via serial, then make it so you set that up within the init function
- [ ] see if it actually works with the display

## use

```python
import vfd

display = vfd.VFD({"model": "CM1564C"})     # create new VFD with model CM1564C

display.setText("insert text")              # simple set text on display

display.setPins("19:17,19:16")              # set specific pins (string method)
display.setPins(["19:17", "19:16"])         # set specific pins (list method)

display.clearPins("19:17,19:16")            # clear specific pins (string method)
display.clearPins(["19:17", "19:16"])       # clear specific pins (list method)

dispay.getPins()                            # returns current pin address list

display.clear()                             # clear all pins
display.send()                              # sends pin address list to screen

```

<br>

### display.setPins() and display.clearPins

```python
display.setPins(pins_in: str|list, apply: bool)
display.clearPins(pins_in: str|list, apply: bool)
```
arguments for both of these functions are identical.

`pins_in` can either be a `str` or `list`, like shown above.

`apply` determines if the pin address list is automatically changed or not. this is set to `True` by default.

returns a `list` of the new changed pin list.

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

