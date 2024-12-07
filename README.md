# Lidar-trigger
Small arduino device using LIDAR to send a signal to release a camera shutter

# Operation
The LIDAR device is currently set up to be connected to a Pluto trigger, through Pluto's AUX port.   When the shutter release is triggered, the device sends a 3v pulse to the Pluto trigger.  The Pluto trigger then sends a command to the camera to release the shutter.    This does require that the two devices be connected via an audio jack, and the Pluto app set to AUX mode.

One powered on, the Lidar device will begin recieiving distance measurements.   The user must set a two points (near and far) that define the range in which the shutter release is triggered.   Currently these are set by using a hand held radio transmitter (see component list) with buttons dedicated to setting near and far distances. To set a distance, hold an object at the desired distance in the LIDAR's beam path, and press the appropriate set button.

The handheld radio transmitter has two additional buttons.  One triggers a sighting later, which can assist in aiming the LIDAR.   The TFLUNA LIDAR works using infrared light, which is mostly invisible to human eyes.   The sighting laser can be mounted to approximate the beam path of the LIDAR.  At night, some cameras, including cell phone camera, may be able to detect the IR signal.  If you point your camera at the LIDAR target, you will see a trapezoidal grid of IR light spots. 

The final button is a shutter release.  This simply allows you to trigger a picture without any consideration for the LIDAR range-finding.  Given the long distance the radio transmitter can travel,  this allows pictures to be triggered from up to 200 ft (60 meters).  

# Construction
This is a hobbyist prototype, and not really intended to be directly copied.  In general, the circuit diagram can be used to build the circuit on breadboards, using the materials found in the component lists. The prototype was converted to a PCB using https://jlcpcb.com .  The GERBER files necessary to manufacture the PCB are currently not part this repository, but can be included if needed.

If using a PCB I highly recommend soldering headers into the board rather than directly soldering the more expensive components.  This allows the components to be removed easily if needed.

A basic understanding of electronics and access to a typical hobbyist electronic equipment is necessary. But this is not an especially sophisticated build.   Additional equipment involved in this build included soldering equipment, digital multimeter, dremel tool and drill for modifying the project box, hot glue gun for securing components. nylon or brass standoff spacers for physically supporting components like the OLED display.  Zip ties and electric tape because, again, this is not an especially sophisticated build.  

# Things that could be improved
* The radio control is the only way to currently interface with the device.  On board physical keys were planned, but a bug in the circuit design needs to be corrected
* Power management has not been tested throughly.  Roght now the best way to make it last longer is get a bigger battery.  This can probably be optimized in the code, by doing small things like turning off lights and displays when no activity is happening.
* A way of "dialing in" the set points is conceivably useful, and would probably require additional buttons or a potentiometer
* The laser and Lidar are currently not aligned exceot by duct tape and trial and error. They could be designed in a way that keepys them from drifting.
* The LIDAR beam is pretty broad (2 degrees) for small objects, but too small to catch movement outside of the 2 degree beam path.  Short of including additional LIDAR devices, it's not immediately clear that anything can be done about that.

# Disclaimer
This is offered as is. I am not an expert and there may be errors or bugs in the approach described here that could cause any number of problems. If you are replicating it, you are agreeing to be responsible for any negative outcomes.   
