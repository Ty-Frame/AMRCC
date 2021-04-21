import math
import sys
from math import sin, sqrt, cos, atan2

#####
# USE THIS ONE
#####
# Mlat - Mortar Latitude (comes from turret script)
# Mlong - Mortar Longitude (comes from turret script)
# Tlat - Target Latitude (comes from bot)
# Tlong - - Target Longitude (comes from bot) 
input = sys.argv[1]
marker, turret = input.split("!")
Mlong, Mlat = marker.split(",")
Tlong, Tlat = turret.split(",")
Mlat = math.radians(float(Mlat))
Mlong = math.radians(float(Mlong))
Tlat = math.radians(float(Tlat))
Tlong = math.radians(float(Tlong))
Dlat = Tlat - Mlat
Dlong = Tlong - Mlong
a = (math.sin(Dlat/2))**2+cos(Mlat)*cos(Tlat)*(math.sin(Dlong/2))**2
c = 2*math.atan2(math.sqrt(a), math.sqrt(1-a))
R = 6371000
distance = R*c

print(distance)

