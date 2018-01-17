@echo off

@echo Deleting old STL files.
del *.stl

@echo Building Really Dusty Box STLs

@echo Neopixel And Button
"C:\Program Files\OpenSCAD\openscad.com" -o NeopixelAndSwitchHolder.stl NeopixelAndSwitchHolder.scad

@echo Spacers
"C:\Program Files\OpenSCAD\openscad.com" -o Spacer.stl Spacer.scad

@echo Filter Frame 6x6
"C:\Program Files\OpenSCAD\openscad.com" -o FilterFrame-6x6-Inside.stl -D "isInsideFrame=true;depth=25;holesDown=6;holesAcross=6" FilterFrame.scad
"C:\Program Files\OpenSCAD\openscad.com" -o FilterFrame-6x6-Outside.stl -D "isInsideFrame=true;depth=20;holesDown=6;holesAcross=6" FilterFrame.scad
"C:\Program Files\OpenSCAD\openscad.com" -o FilterFrame-6x6-Retainer.stl -D "isInsideFrame=true;depth=2;holesDown=6;holesAcross=6" FilterFrame.scad

@echo Filter Frame 3x3
"C:\Program Files\OpenSCAD\openscad.com" -o FilterFrame-3x3-Inside.stl -D "isInsideFrame=true;depth=25;holesDown=3;holesAcross=3" FilterFrame.scad
"C:\Program Files\OpenSCAD\openscad.com" -o FilterFrame-3x3-Outside.stl -D "isInsideFrame=true;depth=20;holesDown=3;holesAcross=3" FilterFrame.scad
"C:\Program Files\OpenSCAD\openscad.com" -o FilterFrame-3x3-Retainer.stl -D "isInsideFrame=true;depth=2;holesDown=3;holesAcross=3" FilterFrame.scad

@echo Mini Fan Filter
#"C:\Program Files\OpenSCAD\openscad.com" -o FanFilter-120-Mini.stl -D "width=120;height=120;totalDepth=95" FanFilter.scad
"C:\Program Files\OpenSCAD\openscad.com" -o FanFilter-140-Mini.stl -D "width=140;height=140;totalDepth=95" FanFilter.scad

@echo Normal Fan Filter
#"C:\Program Files\OpenSCAD\openscad.com" -o FanFilter-120-Std.stl -D "width=120;height=120;totalDepth=160" FanFilter.scad
"C:\Program Files\OpenSCAD\openscad.com" -o FanFilter-140-Std.stl -D "width=140;height=140;totalDepth=160" FanFilter.scad

@echo Tall Fan Filter
#"C:\Program Files\OpenSCAD\openscad.com" -o FanFilter-120-Tall.stl -D "width=120;height=120;totalDepth=260" FanFilter.scad
"C:\Program Files\OpenSCAD\openscad.com" -o FanFilter-140-Tall.stl -D "width=140;height=140;totalDepth=260" FanFilter.scad

@echo Fan Difuser - Partial
#"C:\Program Files\OpenSCAD\openscad.com" -o FanDifuser-120-OpenTop.stl -D "width=120;height=120;totalDepth=60;openTop=true" FanDifuser.scad
"C:\Program Files\OpenSCAD\openscad.com" -o FanDifuser-140-OpenTop.stl -D "width=140;height=140;totalDepth=60;openTop=true" FanDifuser.scad

@echo Fan Difuser - Full
#"C:\Program Files\OpenSCAD\openscad.com" -o FanDifuser-120-Std.stl -D "width=120;height=120;totalDepth=100;openTop=false" FanDifuser.scad
"C:\Program Files\OpenSCAD\openscad.com" -o FanDifuser-140-Std.stl -D "width=140;height=140;totalDepth=100;openTop=false" FanDifuser.scad

@echo Fan Adaptor - 100mm
#"C:\Program Files\OpenSCAD\openscad.com" -o FanAdaptor-120-2-100.stl -D "width=120;height=120;ductSize=100;coneHeight=20" FanAdaptor.scad
"C:\Program Files\OpenSCAD\openscad.com" -o FanAdaptor-140-2-100.stl -D "width=140;height=140;ductSize=100;coneHeight=40" FanAdaptor.scad

@echo Electronics Enclosure
"C:\Program Files\OpenSCAD\openscad.com" -o ElectronicsCase-Up.stl -D "pcbPostHeight=7.5;faceUp=true" FanAdaptor.scad
"C:\Program Files\OpenSCAD\openscad.com" -o ElectronicsCase-Down.stl -D "pcbPostHeight=26.5;faceUp=false" FanAdaptor.scad

@echo FanSizeAdaptors
"C:\Program Files\OpenSCAD\openscad.com" -o FanSizeAdaptor-140-120.stl -D "outerSize=140;thickness=3" FanSizeAdaptor.scad