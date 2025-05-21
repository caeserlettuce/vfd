import vfd


# /dev/tyACM0

display = vfd.VFD({"model": "CM1564C", "outputs": 32, "serial_port": False})

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

string = display.verifyText("hello")

# scroll_list = vfd.scrollText(string, 2)
# print(scroll_list)
pins = display.setText(string, False)
display.setPins(pins)

# ALL the pins!!!!!!!!!!!!
# 
# display.setPins(["18:5","18:7","18:8","18:9","18:10","18:11","18:12","18:13","18:14","18:15","18:16","18:17","19:4","19:5","19:6","19:7","19:8","19:9","19:10","19:11","19:12","19:13","19:14","19:15","19:16","19:17","20:4","20:5","20:6","20:7","20:8","20:9","20:10","20:11","20:12","20:13","20:14","20:15","20:16","20:17","21:4","21:5","21:6","21:7","21:8","21:9","21:10","21:11","21:12","21:13","21:14","21:15","21:16","21:17","22:4","22:5","22:6","22:7","22:8","22:9","22:10","22:11","22:12","22:13","22:14","22:15","22:16","22:17","23:2","23:3","23:4","23:5","23:6","23:7","23:8","23:9","23:10","23:11","23:12","23:13","23:14","23:15","23:16","23:17","24:3","24:4","24:5","24:6","24:7","24:8","24:9","24:10","24:11","24:12","24:13","24:14","24:15","24:16","24:17","25:4","25:5","25:6","25:7","25:8","25:9","25:10","25:11","25:12","25:13","25:14","25:15","25:16","25:17","26:4","26:5","26:6","26:7","26:8","26:9","26:10","26:11","26:12","26:13","26:14","26:15","26:16","26:17","27:8","27:9","27:10","27:11","27:12","27:13","27:14","27:15","27:16","27:17","28:7","28:8","28:9","28:10","28:11","28:12","28:13","28:14","28:15","28:16","28:17","29:9","29:10","29:11","29:12","29:13","29:14","29:15","29:16","29:17"])

display.setPins(["18:5","18:6","18:7","18:8","18:9","18:10","18:11","18:12","18:13","18:14","18:15","18:16","18:17"])

# print(str(pins).replace("'", '"'))

jaja = display.send(True)

print(jaja)


# vfd.scrollText("hello world", 8)




# display.clearGrids("")

# print(display.getPins())
# display.setPins("20:17,19:16")              # set specific pins (string method)
# print(display.getPins())

# vfd.pin_check(["19:17", "19:16"])


# display.setPins(1)         # set specific pins (list method)


# display.clearPins("19:17,19:16")            # clear specific pins (string method)
# display.clearPins(["19:17", "19:16"])       # clear specific pins (list method)