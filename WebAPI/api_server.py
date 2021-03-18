# ------------------------------------------------------------------------------
# name : api_server.js
# author : Noe Flatreaud (Retr0)
# description:
#   simple API server for the IHC-controller project
#   it provide an API url as well as a basic dashboard so you can
#   use it every were by using the web browser.
#   you can as well create a third-pary app and use the api url.
# ------------------------------------------------------------------------------

import serial
import threading
import time
from flask import *

app = Flask(__name__)

# ------------------------------------------------------------------------------
# Arduino Serial
# ------------------------------------------------------------------------------

output = [ 0xC3 ]

arduino = serial.Serial(port='COM4', baudrate=9600, timeout=.1)

#def write_read(x):
#    arduino.write(bytes(x, 'utf-8'))
#    time.sleep(0.05)
#    data = arduino.readline()
#    return data

# ------------------------------------------------------------------------------
# Routes
# ------------------------------------------------------------------------------

# default route
@app.route('/')
def index():
    return render_template('index.html')

# API route
@app.route('/api', methods=['GET'])
def api():
    return write_read(request.args["state"])

# allow javascript index.js to be imported !
@app.route('/index.js')
def js():
    return render_template('index.js')

def io_thread():
    print("Started new Daemon")
    while 1:
        arduino.write(0x00)
        time.sleep(0.05)

# ------------------------------------------------------------------------------
# Main
# ------------------------------------------------------------------------------
if __name__ == "__main__":
    io = threading.Thread(target = io_thread, daemon = True)
    io.start();
    # run flask server
    app.run(host='0.0.0.0')
