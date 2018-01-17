// Human side fan guard to prevent fingers touching the blades.


// fan x,y
// 120mm fan.
//width = 120;
//height = 120;
//holeOffset = 7.5;

// 140mm fan
width = 140;
height = 140;
holeOffset = 7.5; // Should be 70-7.5 (62.5) from middle of the axis

echo("fan width",width );
echo("fan height", height);
echo("fan holeOffset", holeOffset);

// Leave the top open (e.g. another lid is placed on top
// makes printing easier.
// Adding a top increases the height by coneHeight
openTop = true;

totalDepth = 60;
coneHeight = 40;
depth = totalDepth;

echo("totalDepth", totalDepth);
echo("depth", depth);

$fn=180;

module noCornersCube(width, height, depth, cornerRadius) {
    
    //cube([width, height, depth]);
    union() {
        linear_extrude(height=depth) {
            // Square with corners cut.
            polygon([
            [cornerRadius,0],
            [width-cornerRadius, 0],
            [width, cornerRadius],
            [width, height-cornerRadius],
            [width-cornerRadius, height],
            [cornerRadius, height],
            [0, height-cornerRadius],
            [0, cornerRadius],
            [cornerRadius,0]
            ]);
        }
        
    }
}

module roundedCube(width, height, depth, cornerRadius) {
    
    //cube([width, height, depth]);
    union() {
        translate([cornerRadius,cornerRadius,0]) {
            cylinder(r=cornerRadius, h=depth);   
        }
        translate([width-cornerRadius,cornerRadius,0]) {
            cylinder(r=cornerRadius, h=depth);
        }
        translate([cornerRadius,height-cornerRadius,0]) {
            cylinder(r=cornerRadius, h=depth);
        }
        translate([width-cornerRadius,height-cornerRadius,0]) {
            cylinder(r=cornerRadius, h=depth);
        }
        
        translate([cornerRadius, 0, 0]) {
            cube([width-(2*cornerRadius), height, depth]);
        }
        translate([0, cornerRadius, 0]) {
            cube([width, height-(2*cornerRadius), depth]);
        }
    }
}


module fanMount() {
    difference() {
        union(){
            roundedCube(width,height,3, 8);
        }
        union() {
            
            translate([width/2, height/2, -0.01]) {
                cylinder(d=width-4, h=3.1,$fn=12);
            }
            
            mountingHoles(holeOffset);
        }
    }
}

module filterMount() {
    difference() {
        union(){
            translate([width/2, height/2, 0]) {
                cylinder(d=width, h=depth,$fn=12);
            }
            
            if (!openTop) {
                // Add a cone onto the end to make the bridging required to be printed
                // a lot more managagle.
                translate([(width/2), (height/2), depth]) {
                    cylinder(d1=width,d2=44, h=coneHeight,$fn=12);
                }            
            }
        }
        union() {
            translate([(width/2), (height/2), -0.01]) {
                cylinder(d=width-4, h=depth+0.02,$fn=12);
            }            
            
            if (!openTop) {
                translate([(width/2), (height/2), depth-0.01]) {
                    #cylinder(d1=width-3,d2=46.5, h=coneHeight-1.98,$fn=12);
                }            
            }

            // Cutouts for air to exit.
            // If open top, extra depth won't matter.
            // If cone top, extra depth goes into cone as well.
            for(z = [8 : 8 : depth + coneHeight/2.5]) {
            
                translate([(width)/2, (height/2), z]) {
                    for(angle = [15 : 30 : 360]) {
                        rotate([0, 0, angle]) {
                            cube([20,width+10, 4],center=true);
                        }
                    }
                }
            }
        }
    }
}

module mountingHoles(holeOffset) {
    mountingHole(holeOffset,holeOffset);
    mountingHole(width-holeOffset,holeOffset);
    mountingHole(width-holeOffset,height-holeOffset);
    mountingHole(holeOffset,height-holeOffset);
}

module mountingHole(x,y) {
    translate([x,y,0]) {
        cylinder(d=5, h=depth);
    }
}

union() {
    fanMount();
    filterMount();
        
}