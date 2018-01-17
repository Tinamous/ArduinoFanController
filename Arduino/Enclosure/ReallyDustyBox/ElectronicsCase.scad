// Fan Filter.
// Intended to bolt to the inlet of a fan (fan horizontal)
// and be filled with filter material 
// e.g. activated charcoal for air cleaning
// or silica gel or drying.


width = 120;
height = 120;
depth = 32;

// If the Arduino is face up
faceUp = true;

// 5mm posts to have the Arduino facing up (sampling outside the box) (faceUp = true)
// 25mm posts for Arduino face down (sampling inside the box) (faceUp = false)
pcbPostHeight = 7.5;

// PCB is 100x100
pcbXOffset = (width - 100)/2;
pcbYOffset = (height - 100)/2;

echo("width",width );
echo("height", height);
echo("depth", depth);

$fn=80;

// Opening for 40mm fan (Fan5) in the back to allow air to be dragged in 
// and over sensors from outside. Shouldn't need fan for internal air sampling
// (but will be effected by activated carbon filter so maybe not the best).
// might also want to put the sensor inside fan path before the charcoal.

// Open up DC jack socket - needs to be lower.

// Increase height (to 6mm) of standoffs.
// make standoff holes go all the way though.

// mount on back & inside for CCS811 

//todo
// Think about Arduino battery holder.....
// Holes for sensors when arduino face down

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
        
        noCornersCube(width, height, depth, cornerRadius);
    }
}



module pcbMountingPosts() {
holeOffset = 5;
// PCB is 100x100
xOffset = (width - 100)/2;
yOffset = (height - 100)/2;
    
    
    
    translate([xOffset, yOffset,0]) {
        pcbMountingPost(holeOffset,holeOffset, pcbPostHeight);
        pcbMountingPost(holeOffset+90,holeOffset,pcbPostHeight);
        pcbMountingPost(holeOffset+90,holeOffset+90,pcbPostHeight);
        pcbMountingPost(holeOffset,holeOffset+90,pcbPostHeight);
    }
}

module pcbMountingPost(x,y,height) {
    translate([x,y,0]) {
        difference() {
            cylinder(d=8, h=height);
            cylinder(d=4.5, h=height);
        }
    }
}


module pcbMountingHoles() {
holeOffset = 5;
   
    
    translate([pcbXOffset, pcbYOffset,1.5]) {
        pcbMountingHole(holeOffset,holeOffset);
        pcbMountingHole(holeOffset+90,holeOffset);
        pcbMountingHole(holeOffset+90,holeOffset+90);
        pcbMountingHole(holeOffset,holeOffset+90);
    }
}

module pcbMountingHole(x,y) {
    translate([x,y,0]) {
        cylinder(d=4.5, h=depth-1.5);
    }
}

module caseMounts() {
    translate([0, 0,1.5]) {
        caseMountingPost(5,5,depth-1.5);
        caseMountingPost(width-5,5,depth-1.5);
        caseMountingPost(5,height-5,depth-1.5);
        caseMountingPost(width-5,height-5,depth-1.5);
    }
}

module caseMountingPost(x,y,height) {
    translate([x,y,0]) {
        difference() {
            cylinder(d=10, h=height);
            cylinder(d=4.5, h=height);
        }
    }
}

module powerCableCutout() {
    
    // Power connector cutout
    translate([-5, pcbYOffset,pcbPostHeight+8]) {
        
        // DC Jack
        // Cut out both sides as the other can be used for LED cables
        translate([0, 54,0]) {
            
            rotate([0,90,0]) {
                #cylinder(d=12, h= width+10);
            }
        }
    }
        
    translate([-5, pcbYOffset,depth-8]) {
        
        // DC Jack
        // Cut out both sides as the other can be used for LED cables
        translate([0, 50,0]) {
            //#cube([width+10,8,8.1]);
        }
        
        // terminal block
        translate([0, 69.8,0]) {
            #cube([width+10,4,8.1]);
        }
    }
}

module dustSensorCableCutout() {
    translate([-5, pcbYOffset,depth-8]) {
        
        // DC Jack
        // Cut out both sides as the other can be used for LED cables
        translate([0, 80,0]) {
            #cube([width+10,8,8.1]);
            
            
        }
    }
}

module fanCableCutouts() {
    // -1 x for all 4 fans
    // +10 x for fans 3 and 5.
    translate([-1, pcbYOffset,0]) {
        fanCableCutout(25.4); // Fans 2 + 4
        fanCableCutout(40.6); // Fans 1 + 3
    }
}

module fanCableCutout(y) {
    translate([0,y,depth-8]) {
        cube([width+5, 4, 8.1]);
    }
}

module sensorHole() {
    
    if (!faceUp) {
        // Sensor hole is only needed when Arduino is facing down.
        // Guess at Y position (and assumes centered in Arduino x 
        translate([width/2, pcbYOffset+61,-0.1]) {
            cylinder(d=6, h=depth+1);
        }
    }
}

module cutoutPrototypeArduino() {
    // Arduino extends 20mm past the Pcb Top
    translate([pcbXOffset + ((100-26)/2), pcbYOffset + 100, 12 + 5]) {
        #cube([26,20,depth-17 + 1]);
    }
}

module fan40Hole(x,y) {
    translate([x,y,0]) {
        cylinder(d=4, h=8);
    }
}

module fan40() {
    translate([20,20,0]) {
        cylinder(d=40, h=5);
        fan40Hole(-17.8, -17.8);
        fan40Hole(-17.8, 17.8);
        fan40Hole(17.8, 17.8);
        fan40Hole(17.8, -17.8);
    }
}



module cutoutRearFan() {
    // put in a 40mm fan. reference by lower left corner
    translate([width/2-20, height/2-20, -0.1]) {
        fan40() ;
    }
}

module fan40Mount(x,y) {
    translate([x, y, 0]) {
        cylinder(d=8, h=6); 
    }
}

module rearFanMounts() {
    difference() {
        union() {
            translate([width/2, height/2, 0]) {
                fan40Mount(-17.8, -17.8);
                fan40Mount(-17.8, 17.8);
                fan40Mount(17.8, 17.8);
                fan40Mount(17.8, -17.8);
            }
        }
        union() {
            cutoutRearFan();
        }
    }
}

module cutoutCCS811Holes() {
    translate([width/2, height-10, -0.1]) {
        translate([(13/2), 0, 0]) {
            cylinder(d=4.2, h=10);
        }
        translate([-(13/2), 0, 0]) {
            cylinder(d=4.2, h=10);
        }
    }
}

module cCS811Mounts() {
            
    difference() {
        union() {            
            translate([width/2, height-10, 0]) {
                translate([(13/2), 0, 0]) {
                    cylinder(d=8, h=6);
                }
                translate([-(13/2), 0, 0]) {
                    cylinder(d=8, h=6);
                }
            }
        }
        union() {
           cutoutCCS811Holes();
        }
    }
}

module mainBody() {
    difference() {
        union() {
            roundedCube(width, height, depth, 4);
        }
        union() {
            translate([1.5, 1.5, 1.5]) {
                roundedCube(width-3, height-3, depth, 6);
            }
            // Ensure holes are put in
            pcbMountingHoles();
            
            powerCableCutout();
            
            fanCableCutouts();
            
            sensorHole();
            
            dustSensorCableCutout();
            
            // TODO: Dust Sensor...
            
            cutoutPrototypeArduino();
            
            cutoutRearFan();
            
            cutoutCCS811Holes();
        }
    }
}

mainBody();
pcbMountingPosts();
caseMounts();
cCS811Mounts();
rearFanMounts();