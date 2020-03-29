#!/usr/bin/env python3


import json
import sys
import os
import xml.etree.ElementTree as ET
import xml.dom.minidom as minidom
import shutil

import subprocess

if len(sys.argv)!=2:
    print('FG_run.py -- bad argument count')
    exit(-1)
    
filename=sys.argv[1]

if not os.path.exists('./'+filename):
    print('FG_run.py -- file not found: '+filename)
    exit(-1)


fgdata=os.getenv("FGDATA")
if fgdata is None:
    fgdata='/usr/share/games/flightgear/'

if not os.path.exists(fgdata):
    print('Not found FG Data directory: '+ fgdata)
    exit(-1)

protocols=fgdata+'/Protocol'
if not os.access(protocols, os.W_OK):
    print('Cannot Write into direcotry: '+ protocols)
    exit(-1)

#models=fgdata+'/Aircraft'
#if not os.access(models, os.W_OK):
#    print('Cannot Write into direcotry: '+ protocols)
#    exit(-1)


############################ Parse Config ######################################

with open(filename) as json_file:
    data = json.load(json_file)    
    model=data['FgModel']
    url=data['Url']
    controls=data['Controls']

print(model)
print(url)
for c in controls:
    print(c[0]+' '+c[1]+' '+c[2])


############################ Download Model #####################################
#if not exists -- not implemend yet

############################ Create FG Input Protocol ###########################
propertyList=ET.Element('PropertyList')
generic= ET.SubElement(propertyList, 'generic')
input=ET.SubElement(generic,'input')

binary_mode=ET.SubElement(input,'binary_mode')
binary_mode.text='true'

for c in controls:
    chunk=ET.SubElement(input,'chunk')
    name=ET.SubElement(chunk,'name')
    name.text=c[0]
    type=ET.SubElement(chunk,'type')
    type.text='double'
    node=ET.SubElement(chunk,'node')
    node.text=c[1]

rough_string = ET.tostring(propertyList, 'utf-8')
reparsed = minidom.parseString(rough_string)
xmlstring=reparsed.toprettyxml(indent="  ")

with open(protocols+'/PX4toFG.xml','w') as xml_file:
    xml_file.write(xmlstring)

############################ Copy FG Output Protocol ############################
shutil.copy('px4bridge.xml',protocols+'/FGtoPX4.xml' )

############################ Run FG #############################################

os.system('fgfs \
    --aircraft='+model+' \
    --timeofday=noon \
    --disable-sound \
    --telnet=4443 \
    --generic=socket,out,1000,127.0.0.1,4444,udp,FGtoPX4 \
    --generic=socket,in,1000,,4445,udp,PX4toFG \
    --disable-terrasync \
	--disable-splash-screen \
	--model-hz=120 \
	--disable-random-objects \
	--prop:/sim/rendering/texture-compression=off \
	--prop:/sim/rendering/quality-level=0 \
	--prop:/sim/rendering/shaders/quality-level=0 \
	--disable-ai-traffic \
	--prop:/sim/ai/enabled=0 \
	--prop:/sim/rendering/random-vegetation=0 \
	--prop:/sim/rendering/random-buildings=0 \
	--disable-specular-highlight \
	--disable-ai-models \
	--disable-clouds \
	--disable-clouds3d \
	--fog-fastest \
	--visibility=5000 \
	--disable-distance-attenuation \
	--disable-real-weather-fetch \
	--prop:/sim/rendering/particles=0 \
	--prop:/sim/rendering/multi-sample-buffers=1 \
	--prop:/sim/rendering/multi-samples=2 \
	--prop:/sim/rendering/draw-mask/clouds=false \
	--prop:/sim/rendering/draw-mask/aircraft=true \
	--prop:/sim/rendering/draw-mask/models=false \
	--prop:/sim/rendering/draw-mask/terrain=true \
	--disable-random-vegetation \
	--disable-random-buildings \
	--disable-rembrandt \
	--disable-horizon-effect & echo $! > /tmp/px4fgfspid')


