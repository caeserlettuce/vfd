import json
import sys
import os
import unicodedata
import requests

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
WEBAPP_DEBUG = True         # enable this when webapp debugging
DEBUG_PRINT = False
WEBAPP_URL = "http://127.0.0.1:5000"


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
        
        if "model" in settings_in:
            self.model = settings_in["model"]
        else:
            raise Exception(bc.BOLD + bc.FAIL + "model must be set!" + bc.END)
        
        self.addresses = [] #final pin address list

    def send(self):
        dprint("sending addresses:")
        dprint(self.addresses)

        try:

            headers = {'Content-Type': 'application/json'}
            data = {"listing": self.addresses}

            response = requests.post(WEBAPP_URL + "/send_listing", data=json.dumps(data), headers=headers)

            if response.status_code == 200:
                response_data = response.json()
                dprint(response_data)
            else:
                print(f"Error: {response.status_code}")

        except Exception:
            print(Exception)
        
        dprint(bc.BOLD + bc.HEADER + "addresses sent!" + bc.END)

    def clear(self):
        self.addresses = []

        dprint(bc.BOLD + bc.WARNING + "DISPLAY CLEARED" + bc.END)
        
    def getPins(self):

        addresses_return = json.dumps(self.addresses)
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

        temp_address_list = self.addresses[:]

        for p in temp_address_list:
            add_split = p.split(":")
            if int(add_split[0]) in grid_list:
                temp_address_list.remove(p)

        temp_address_list = list(set(temp_address_list))    # make sure there's no duplicated

        if apply == True:
            self.addresses = temp_address_list[:]
        
        return temp_address_list


    def verifyText(self, text_in, font_in="default"):
        grids = displays[self.model]["fonts"][font_in]["grids"]
        glyphs = displays[self.model]["fonts"][font_in]["glyphs"]
        replacements = displays[self.model]["fonts"][font_in]["replacements"]

        for rp in replacements:
            text_in = text_in.replace(str(rp), str(replacements[rp]))

        text_flat = flatten_text(text_in)


        text_split = list(text_in)
        text_flat_split = list(text_flat)

        index = 0
        for l in text_split:
            if l not in glyphs:
                if text_flat_split[index] not in glyphs:
                    dprint( l + " is extra not in the glyphs!")
                    text_split[index] = "unknown"
                else:
                    text_split[index] = text_flat_split[index]      # for unicode characters like éàçüö, they will be flattened down to eacuo. basically better legibility
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