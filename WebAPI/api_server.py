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

    try:
        output = int(request.args['output']);
        value =  int(request.args['value']);
        print(bytes([output, value]))

    except Exception as e:
        data =  "Oops Something went wrong !!<br> {0}".format(str(e))
        return data, 413 # HTTP_413_REQUEST_ENTITY_TOO_LARGE
    else:
        data = "OK"
        arduino.write(bytes([output, value]));

    return data, 200 # HTTP_200_OK

# allow javascript index.js to be imported !
@app.route('/index.js')
def js():
    return render_template('index.js')

def io_thread():
    print("Started new Daemon")
    while 1:
        time.sleep(1)
        #print("slept");

# ------------------------------------------------------------------------------
# Main
# ------------------------------------------------------------------------------
if __name__ == "__main__":
    io = threading.Thread(target = io_thread, daemon = True)
    io.start();
    # run flask server
    app.run(host='0.0.0.0')
