import json
import sys
import os
import unicodedata
import requests
import serial

class bc:
    HEADER = '\033[95m'
    BLUE = '\033[94m'
    CYAN = '\033[96m'
    GREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    END = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

script_directory = os.path.dirname(os.path.abspath(__file__))

displays = json.load(open(os.path.join(script_directory, 'displays.json').encode()))



# param varaibles
WEBAPP_DEBUG = False         # enable this when webapp debugging
DEBUG_PRINT = False
WEBAPP_URL = "http://127.0.0.1:5000"
SERIAL_SEND = True




def dprint(input):
    if DEBUG_PRINT == True:
        print(input)

def intj(str_in):
    try:
        return int(str_in)
    except Exception:
        print(bc.BOLD + bc.FAIL + "the string " + bc.END + bc.WARNING + bc.UNDERLINE + "'" + str(str_in) + "'" + bc.END + bc.BOLD + bc.FAIL + " is an invalid number!" + bc.END)
        return -100
    
def flatten_text(text):
    normalized_text = unicodedata.normalize('NFKD', text)
    ascii_text = ''.join([c for c in normalized_text if ord(c) < 128])
    return ascii_text


def pin_check(pins_in, model_in):
    if (type(pins_in) == str or type(pins_in) == list):
            
        if (type(pins_in) == str): # string "19:17,19:6"
            sep_list = pins_in.split(",")
            pins_in = sep_list
        
        check_out = []
        for p in pins_in:
            pin_address = p.split(":")
            index = 0
            pin_good = True

            if len(pin_address) != len(displays[model_in]["address support"]):
                print(bc.BOLD + bc.FAIL + "address length " + bc.END + bc.WARNING + bc.UNDERLINE + str(len(pin_address)) + bc.END + bc.BOLD + bc.FAIL + " does not match what's required by display " + bc.END + bc.WARNING + bc.UNDERLINE + "(" + str(len(displays[model_in]["address support"])) + ")" + bc.END + bc.BOLD + bc.FAIL + " !" + bc.END)
            else:
                

                for c in pin_address:
                    pin_num = intj(c)        # pin number
                    add_plc = intj(index)    # placement within address (address placements would be: 0:1:2 etc..)
                    # print("checking pin num " + str(pin_num) + " in place " + str(add_plc))
                    if pin_num not in displays[model_in]["address support"][add_plc]:
                        pin_good = False
                        print(bc.BOLD + bc.FAIL + "pin number " + bc.END + bc.WARNING + bc.UNDERLINE + str(c) + bc.END + bc.BOLD + bc.FAIL + " is not supported!" + bc.END)

                    index += 1

                if (pin_good == True):
                    check_out.append(p)
        
        return check_out
    
    else:
        print(bc.BOLD + bc.FAIL + "pin list must be 'str' or 'list', not " + bc.END + bc.WARNING + str(type(pins_in)) + bc.END)

        
    


class VFD:
    def __init__(self, settings_in={}):
        global SERIAL_SEND
        
        if "model" in settings_in:
            self.model = settings_in["model"]
        else:
            raise Exception(bc.BOLD + bc.FAIL + "model must be set!" + bc.END)
        
        if "serial_port" in settings_in:
            self.serial_port = settings_in["serial_port"]
            if (self.serial_port == False):
                SERIAL_SEND = False
        

        self.addresses = [] # final pin address list
        self.brightness = displays[self.model]["max brightness"]
        self.prev_sent = "{}"
        
        if SERIAL_SEND == True:
            self.ser = serial.Serial(self.serial_port, 9600, timeout=0.050)

    def send(self, return_out=False):
        dprint("sending addresses:")

        out_addresses = {
            "brightness": self.brightness,
            "listing": []
        }

        for i in self.addresses:
            split_tm = i.split(":")
            out_addresses["listing"].append([])
            for e in split_tm:
                out_addresses["listing"][-1].append(int(e))

        # sort the listing
        # this is specifically where support for 3-pin addresses will probably most likely be nonexistent
        listing_og = out_addresses["listing"]

        clump_leads = {}
        clump_order = []
        clump_order_out = []
        clumps = {}
        listing_sorted = []
        index = 0
        
        # create clump leads
        for a in listing_og:
            clump_init = (str(listing_og[index][1]) in clump_leads) # check if anode pin is already defined

            if clump_init == False:
                clump_leads[str(listing_og[index][1])] = 0  # define it in clump leads
            
            clump_leads[str(listing_og[index][1])] += 1 # iterate up 1
            index += 1

        # figure out clump order
        for a in clump_leads:
            clump_order.append(str(clump_leads[a]) + "_" + str(100 - int(a)))
            
            # why 100 - int(a)???
            # i want the clump sizes to be sorted from largest to smallest,
            # but i want the clump leads to be sorted from smallest to largest.
            # so i needed an easy and reversible way to make the largest the smallest, and the smallest the largest
            # so, subtract it from 100.
        
        clump_order.sort()
        clump_order.reverse()

        for a in clump_order:
            clump_order_out.append(str(100 - int(a.split("_")[1])))

        index = 0
        # assemble the clumps object
        for a in clump_order_out:
            index  = 0

            if a not in clumps:
                clumps[a] = []

            for y in listing_og:
                if int(a) in listing_og[index]:
                    clumps[a].append(listing_og[index][0])
                index += 1
            clumps[a].sort()

        # print(clumps)
                
        binary_listing = []
        init_append = []
        registers = int(displays[self.model]["outputs"] / 8)
        for i in range(displays[self.model]["outputs"]):
            init_append.append("0")

        for a in clumps:
            binary_append = init_append[:]

            binary_append[int(a) - 1] = "1"

            # print(init_append)
            
            # print(clumps[a])
            for p in clumps[a]:
                # print(p)
                binary_append[int(p) - 1] = "1"       # setting byte to 1 according to pin number

            binary_append.reverse()                 # reversing all bytes (replacement for LSBFIRST on teensy side. makes things simpler if i do it here)
            binary_append = "".join(binary_append)  # append the list of bytes into one big string

            binary_split = []
            split_append = ""

            index = 0
            for char in binary_append:              # this for loop splits the string into 8-bit sections
                
                if index % 8 == 0:
                    if index > 0:
                        binary_split.append(split_append)
                    split_append = ""
                split_append += char

                index += 1
            binary_split.append(split_append)
            
            index = 0
            for b in binary_split:                  # this for loop converts the binary bits into integer bits (representing the binary as an int from 0-255)
                binary_split[index] = int(binary_split[index], 2)
                index += 1
            binary_listing.append(binary_split)

        out_addresses["listing"] = binary_listing[:]

        # print(json.dumps(out_addresses))

        # print(data)

        if (return_out == True):
            return out_addresses

        else:

            if SERIAL_SEND == True:

                data = self.ser.readline().decode('utf-8').strip()
                # print("yaya")
                if (data != ""):
                    print(data)
                
                out_dumps = json.dumps(out_addresses)

                if out_dumps != self.prev_sent:
                    self.ser.write(json.dumps(out_addresses).encode("ascii", errors='ignore'))
                
                self.prev_sent = out_dumps


            



            # WEBAPP !!!!!!!!!!!!!!!!!!!!!!!!!!!! BELOW !!!!!!!!!!!!!
            if WEBAPP_DEBUG == True:
                try:

                    headers = {'Content-Type': 'application/json'}
                    
                    response = requests.post(WEBAPP_URL + "/send_listing", data=json.dumps(out_addresses), headers=headers)

                    if response.status_code == 200:
                        response_data = response.json()
                        dprint(response_data)
                    else:
                        print("Error: {response.status_code}")

                except Exception:
                    print(Exception)
            
            dprint(bc.BOLD + bc.HEADER + "addresses sent!" + bc.END)

    def clear(self):
        self.addresses = []

        dprint(bc.BOLD + bc.WARNING + "DISPLAY CLEARED" + bc.END)

    def setBrightness(self, brightness):
        if brightness > displays[self.model]["max brightness"] or brightness < 0:
            print(bc.BOLD + bc.FAIL + "brightness level " + bc.END + bc.UNDERLINE + bc.WARNING + str(brightness) + bc.END + bc.BOLD + bc.FAIL + " is out of range!" + bc.END)
        else:
            self.brightness = brightness
    
    def getBrightness(self):
        return self.brightness

    def getPins(self):

        addresses_return = self.addresses
        return addresses_return
    
    def setPins(self, pin_list, apply=True):
        
        plist_check = pin_check(pin_list, self.model)   # check and make sure that all pins are real and goodly and bigly
        pin_list = plist_check[:]
        temp_address_list = self.addresses[:]

        for p in pin_list:
            temp_address_list.append(p)
        
        temp_address_list = list(set(temp_address_list))    # make sure there's no duplicated

        if apply == True:
            self.addresses = temp_address_list[:]
        
        return temp_address_list
        
    def clearPins(self, pin_list, apply=True):
        
        plist_check = pin_check(pin_list, self.model)   # check and make sure that all pins are real and goodly and bigly
        pin_list = plist_check[:]
        temp_address_list = self.addresses[:]

        for p in pin_list:
            if p in temp_address_list:
                temp_address_list.remove(p)

        temp_address_list = list(set(temp_address_list))    # make sure there's no duplicated

        if apply == True:
            self.addresses = temp_address_list[:]
        
        return temp_address_list
        
    def clearGrids(self, grid_list, apply=True):

        if (type(grid_list) == str): # string "19,20"
            sep_list = grid_list.split(",")
            grid_list = sep_list
        elif (type(grid_list) == int):
            grid_list = [grid_list]
        
        tmp_grid_list = []
        for i in grid_list:
            tmp_grid_list.append(int(i))

        grid_list = tmp_grid_list[:]


        addresses_out = []

        for p in self.addresses:
            add_split = p.split(":")
            if int(add_split[0]) not in grid_list:
                addresses_out.append(p)

        addresses_out = list(set(addresses_out))    # make sure there's no duplicated

        if apply == True:
            self.addresses = addresses_out[:]
        
        return addresses_out


    def verifyText(self, text_in, font_in="default"):
        grids = displays[self.model]["fonts"][font_in]["grids"]
        glyphs = displays[self.model]["fonts"][font_in]["glyphs"]
        replacements = displays[self.model]["fonts"][font_in]["replacements"]

        if displays[self.model]["fonts"][font_in]["caps"] == True:
            text_in = text_in.upper()

        for rp in replacements:
            text_in = text_in.replace(str(rp), str(replacements[rp]))


        text_flat = flatten_text(text_in)

        font_length = len(displays[self.model]["fonts"][font_in]["grids"])

        text_split = list(text_in)
        text_flat_split = list(text_flat)

        index = 0
        grid_index = 0
        scroll_warned = False
        
        for l in text_split:
            if l not in glyphs:
                if text_flat_split[index] not in glyphs:
                    dprint( l + " is extra not in the glyphs!")
                    text_split[index] = "unknown"
                else:
                    text_split[index] = text_flat_split[index]      # for unicode characters like éàçüö, they will be flattened down to eacuo. basically better legibility
                    l = text_flat_split[index]
            
            # grid check
                    
            if (text_split[index] != "unknown"):
                if grid_index < font_length:
                    add_grid = grids[grid_index]
                    glyph_good = True
                    for a in glyphs[l]:
                        if str(add_grid) in displays[self.model]["fonts"][font_in]["missing"]:
                            if int(a) in displays[self.model]["fonts"][font_in]["missing"][str(add_grid)]:
                                glyph_good = False
                                dprint("glyph " + l + " on grid " + str(add_grid) + " is not good!")

                    if glyph_good == False:
                        text_split[index] = "unknown"
                    else:
                        grid_index += 1
                else:
                    if len(displays[self.model]["fonts"][font_in]["missing"]) > 0:
                        text_split[index] = "unknown"
                        if scroll_warned == False:
                            print(bc.WARNING + "[verifyText]" + bc.BOLD + bc.FAIL + " scrolling is not supported on grid fonts with missing segments." + bc.END)
                            scroll_warned = True


            index += 1




        temp_text_split = []
        for l in text_split:
            if l != "unknown":
                temp_text_split.append(l)
        
        text_split = temp_text_split

        text_out = "".join(text_split)

        return text_out
        


    def setText(self, text_in, apply=True, font_in="default"):
        
        grids = displays[self.model]["fonts"][font_in]["grids"]
        glyphs = displays[self.model]["fonts"][font_in]["glyphs"]
        
        font_length = len(displays[self.model]["fonts"][font_in]["grids"])

        if len(text_in) > font_length:
            text_in = text_in[:font_length]

        text_split = list(text_in)

        address_list_out = []
        grid_index = 0

        for l in text_split:
            add_grid = grids[grid_index]
            for a in glyphs[l]:
                address_list_out.append(str(add_grid) + ":" + str(a))

            grid_index += 1
        
        address_list_out = list(set(address_list_out))    # make sure there's no duplicated

        if apply == True:
            self.addresses = address_list_out[:]
        
        return address_list_out


def scrollText(text, length):

    full_len = len(text)
    if (full_len <= length):
        text_list_out = []
        text_list_out.append(text)
        return text_list_out
    
    else:
        frames = full_len - length + 1

        text_list = list(text)
        text_list_out = []

        for i in range(frames):
            # print(text_list[i:][:8])
            text_list_out.append("".join(text_list[i:][:length]))

        # print(text_list_out)
        return text_list_out

# 12345678901
# hello world