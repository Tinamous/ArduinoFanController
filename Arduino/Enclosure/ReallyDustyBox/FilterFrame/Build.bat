@echo off

@echo Deleting old STL files.
del *.stl

@echo Filter Frame Outside
"C:\Program Files\OpenSCAD\openscad.com" -o FilterFrame-Outside-8mm.stl -D "depth=8;holesDown=6;holesAcross=6" FilterFrame-Outside.scad
"C:\Program Files\OpenSCAD\openscad.com" -o FilterFrame-Outside-18mm.stl -D "depth=18;holesDown=6;holesAcross=6" FilterFrame-Outside.scad

@echo Filter Frame Inside
"C:\Program Files\OpenSCAD\openscad.com" -o FilterFrame-Inside-8mm.stl -D "depth=8;holesDown=3;holesAcross=3" FilterFrame-Inside.scad
"C:\Program Files\OpenSCAD\openscad.com" -o FilterFrame-Inside-18mm.stl -D "depth=18;holesDown=3;holesAcross=3" FilterFrame-Inside.scad