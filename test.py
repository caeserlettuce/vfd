import vfd


display = vfd.VFD({"model": "CM1564C"})

display.clear()

# display.setPins("19:17,19:16")              # set specific pins (string method)
# display.setPins(["19:17", "19:16", "19:15"])         # set specific pins (list method)
# display.clearPins("19:17,19:16")            # clear specific pins (string method)


# pins = display.setText("àbcdéftt text éàçüö and more", False)              # simple set text on display
# string = display.verifyText("WORLD")
# pins = display.setText(string, False)
# display.setPins(pins)
# display.setPins("27:9")

# display.setBrightness(3)

string = display.verifyText("93", "side")

scroll_list = vfd.scrollText(string, 2)
print(scroll_list)
pins = display.setText(string, False, "side")
display.setPins(pins)

print(str(pins).replace("'", '"'))

# display.send()


# vfd.scrollText("hello world", 8)




# display.clearGrids("")

# print(display.getPins())
# display.setPins("20:17,19:16")              # set specific pins (string method)
# print(display.getPins())

# vfd.pin_check(["19:17", "19:16"])


# display.setPins(1)         # set specific pins (list method)


# display.clearPins("19:17,19:16")            # clear specific pins (string method)
# display.clearPins(["19:17", "19:16"])       # clear specific pins (list method)