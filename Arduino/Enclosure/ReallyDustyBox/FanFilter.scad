// Fan Filter.
// Intended to bolt to the inlet of a fan (fan horizontal)
// and be filled with filter material 
// e.g. activated charcoal for air cleaning
// or silica gel or drying.

// fan x,y
// 120mm fan.
//width = 120;
//height = 120;
//holeOffset = 7.5;

// 140mm fan
width = 140;
height = 140;
holeOffset = 7.5;

echo("fan width",width );
echo("fan height", height);
echo("fan holeOffset", holeOffset);

//totalDepth = 260; // Maxing out the 
totalDepth = 150; // Maxing out Ultimaker 2+
//totalDepth = 60; // Debugging
coneHeight = 40;
depth = totalDepth - coneHeight;
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
            roundedCube(width,height,4, 8);
        }
        union() {
            
            translate([width/2, height/2, -0.01]) {
                cylinder(d=width-4, h=4.1);
                // was 114 (width-6)
            }
            
            mountingHoles(holeOffset);
        }
    }
}

// A round filter "cartridge" that hangs from the underneath of the fan.
module filterMount() {
    difference() {
        union(){
            translate([width/2, height/2, -depth]) {
                cylinder(d=width, h=depth);
            }
            // Add a cone onto the end to make the bridging required to be printed
            // a lot more managagle.
            translate([(width/2), (height/2), -(depth + coneHeight)]) {
                cylinder(d2=width,d1=44, h=coneHeight);
            }            
        }
        union() {
            translate([(width/2), (height/2), -(depth)-0.01]) {
                cylinder(d=width-4, h=depth+0.02);
            }            
            
            translate([(width/2), (height/2), -(depth + coneHeight-2)]) {
                cylinder(d2=width-4,d1=38, h=coneHeight-1.98);
            }            
            
            for(z = [-(depth-4)-10 : 2 : -(depth/3)]) {
            
                translate([(width)/2, (height/2), z]) {
                    for(angle = [0 : 30 : 180]) {
                        rotate([0,0,angle]) {
                            cube([28, width+10, 1], center=true);
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