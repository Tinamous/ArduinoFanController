@echo off

@echo Deleting old STL files.
#del *.stl

@echo Building Really Dusty Box STLs

@echo Spacers
"C:\Program Files\OpenSCAD\openscad.com" -o Spacer.stl Spacer.scad

@echo Filter Frame
"C:\Program Files\OpenSCAD\openscad.com" -o FilterFrame-Inside.stl -D "assetTrackerVersion=2;batteryCompartmentRight=true;batteryCompartmentLeft=false;includeAerialSlot=true;includeAerialPanel=false" FilterFrame.scad
"C:\Program Files\OpenSCAD\openscad.com" -o FilterFrame-Outside.stl -D "assetTrackerVersion=1;batteryCompartmentRight=true;batteryCompartmentLeft=false;includeAerialSlot=true;includeAerialPanel=false" FilterFrame.scad
"C:\Program Files\OpenSCAD\openscad.com" -o FilterFrame-Retainer.stl -D "assetTrackerVersion=1;batteryCompartmentRight=true;batteryCompartmentLeft=false;includeAerialSlot=true;includeAerialPanel=false" FilterFrame.scad

@echo Mini Fan Filter
"C:\Program Files\OpenSCAD\openscad.com" -o FanFilter-120-Mini.stl -D "width=120;height=120;totalDepth=60" FanFilter.scad
"C:\Program Files\OpenSCAD\openscad.com" -o FanFilter-140-Mini.stl -D "width=140;height=140;totalDepth=60" FanFilter.scad

@echo Normal Fan Filter
"C:\Program Files\OpenSCAD\openscad.com" -o FanFilter-120-Std.stl -D "width=120;height=120;totalDepth=150" FanFilter.scad
"C:\Program Files\OpenSCAD\openscad.com" -o FanFilter-140-Std.stl -D "width=140;height=140;totalDepth=150" FanFilter.scad

@echo Tall Fan Filter
"C:\Program Files\OpenSCAD\openscad.com" -o FanFilter-120-Tall.stl -D "width=120;height=120;totalDepth=260" FanFilter.scad
"C:\Program Files\OpenSCAD\openscad.com" -o FanFilter-140-Tall.stl -D "width=140;height=140;totalDepth=260" FanFilter.scad

@echo Fan Difuser - Partial
"C:\Program Files\OpenSCAD\openscad.com" -o FanDifuser-120-OpenTop.stl -D "width=120;height=120;totalDepth=60;openTop=true" FanFilter.scad
"C:\Program Files\OpenSCAD\openscad.com" -o FanDifuser-140-OpenTop.stl -D "width=140;height=140;totalDepth=60;openTop=true" FanFilter.scad

@echo Fan Difuser - Full
"C:\Program Files\OpenSCAD\openscad.com" -o FanDifuser-120-Std.stl -D "width=120;height=120;totalDepth=100;openTop=false" FanDifuser.scad
"C:\Program Files\OpenSCAD\openscad.com" -o FanDifuser-140-Std.stl -D "width=140;height=140;totalDepth=100;openTop=false" FanDifuser.scad

@echo Fan Adaptor - 100mm
"C:\Program Files\OpenSCAD\openscad.com" -o FanAdaptor-120-2-100.stl -D "width=120;height=120;ductSize=100;coneHeight=20" FanAdaptor.scad
"C:\Program Files\OpenSCAD\openscad.com" -o FanAdaptor-140-2-100.stl -D "width=140;height=140;ductSize=100;coneHeight=40" FanAdaptor.scad
