import vfd


display = vfd.VFD({"model": "CM1564C", "test":"yay"})

display.clear()
display.send()

# display.setPins("19:17,19:16")              # set specific pins (string method)
# display.setPins(["19:17", "19:16", "19:15"])         # set specific pins (list method)
# display.clearPins("19:17,19:16")            # clear specific pins (string method)


pins = display.setText("àbcdéftt text éàçüö and more", False)              # simple set text on display
display.setPins(pins)
# display.clearGrids("")

print(display.getPins())
# display.setPins("20:17,19:16")              # set specific pins (string method)
# print(display.getPins())

# vfd.pin_check(["19:17", "19:16"])


# display.setPins(1)         # set specific pins (list method)


# display.clearPins("19:17,19:16")            # clear specific pins (string method)
# display.clearPins(["19:17", "19:16"])       # clear specific pins (list method)