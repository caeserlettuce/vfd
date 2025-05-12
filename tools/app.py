from flask import Flask, Response, send_from_directory, request, redirect, jsonify
import logging
import os, sys
import json
import atexit


# NOTES:
# exec() is the python equivalent of eval()
#
# i have the streamdeck original


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

def exit_handler():
  print(bc.CYAN + "\nclosing !!!!" + bc.END)





atexit.register(exit_handler)

class NoGet(logging.Filter):
    def filter(self, record):
        return 'GET' not in record.getMessage() # will filter out any logs that include "GET"
class NoPost(logging.Filter):
    def filter(self, record):
        return 'POST' not in record.getMessage() # will filter out any logs that include "GET"

app = Flask(__name__)
log = logging.getLogger('werkzeug')
log.setLevel(logging.INFO)
log.addFilter(NoGet())
log.addFilter(NoPost())


address_listing = []
brightness = 0

# THE STUFF

# @app.route('/style.css')
# def style_css():
#   return send_from_directory(".", "style.css")



# funny file functions
def root_dir():  # pragma: no cover
    return os.path.abspath(os.path.dirname(__file__))

def get_file(filename):
    try:
        src = os.path.join(root_dir(), filename)
        return open(src).read()
    except IOError as exc:
        return str(exc)
    



@app.route('/assets/<path:path>')
def send_asset(path):
    return send_from_directory('assets', path)

@app.route("/", methods=['GET', 'POST'])
def route_home():
    
    return Response(get_file("visualizer.html"))

@app.route("/get_listing", methods=['GET', 'POST'])
def get_listing():
    global address_listing
    return_json = {
        "listing": [],
        "brightness": 0
    }
    
    return_json["listing"] = address_listing
    return_json["brightness"] = brightness

    return jsonify(return_json)

@app.route("/send_listing", methods=['GET', 'POST'])
def send_listing():
    global address_listing
    global brightness
    return_json = {
        "ja": "ja",
    }
    request_json = request.get_json()
    print(request_json)
    request_listing = request_json["listing"]
    brightness = request_json["brightness"]
    
    address_listing = request_listing

    print(address_listing)

    return jsonify(return_json)