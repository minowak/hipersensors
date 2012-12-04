hipersensors
============

Sensors architecture for monitoring PC specs

Compiling: make

Run sensor_runner. Arguments are paths to sensors (shared libraries that implements funcion
from include/sensors.h)

Sample usage: ./sensor_runner lib/cpusensor.so lib/memorysensor.so

Type 'help' in prompt to see available commands

Monitor address is in settings file.

After registering sensor in monitor it is expected to return id of the sensor.
