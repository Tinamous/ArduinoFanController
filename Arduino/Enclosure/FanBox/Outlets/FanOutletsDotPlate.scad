$fn=360;
dotDiameter = 84;

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

module mountinghole(xOffset, yOffset) {
    // Mounting holes...
    // First hole, 10mm from left edge
    translate([5 + xOffset, 15 + yOffset, -0.01]) {
        cylinder(d=4, h=20, $fn=80);
    }
    
    // Bolt extends 10.5mm from top of acrylic.
    // first place is 3mm deep so we have 7.5mm of bolt.
    // a nut is 2.5mm thick so we need to ensure that
    // at 7.5 - 2.5 (5mm) we allow a big enough hole for a nut
    // and some way to get raround it
    translate([5 + xOffset, 15 + yOffset, -0.01]) {
        cylinder(d=4, h=20, $fn=80);
    }
    
    translate([5 + xOffset, 15 + yOffset, 5]) { 
        cylinder(d=9, h=20, $fn=80);
    }
        
}

module mountingHoles() {
    // Holes 90mm appart on X (110mm x hole, 10mm in each side).
    // Holes -5 and +5 from Y edges. Y is 120 long
    translate([0,0,0]) {
        mountinghole(10, -5);
        mountinghole(100, -5);
        mountinghole(10, 120+5);
        mountinghole(100, 120+5);
    }
}

width = 120;
height = 145;

// Face peice
// face is 5mm from edge in x (+/-5mm side to side
// 10mm in the y (+/-10mm back to front).
difference() {
    union() {
        #roundedCube(120, 145, 8, 4);
        
        translate([width/2, height/2, 0]) {
            //cylinder(d=90, h=15);
            cylinder(d1=104, d2=dotDiameter+6, h=20);
        }
        
        translate([width/2, height/2, 15]) {
            //cylinder(d=90, h=15);
            cylinder(d=dotDiameter+6, h=40);
        }
    }
    union() {
        translate([7,17,-0.01]) {
            //roundedCube(120-(4+10), 130-(4+10), 3.1, 4);
        }
        
        // Hole for Alexa...
        translate([width/2, height/2+4, 28]) {
            rotate([35,0,0]) {
                cylinder(d=dotDiameter, h=50); // 30mm
                #cylinder(d=dotDiameter, h=30); // 30mm
            }
        }
        
        translate([0, 20, 15]) {
            rotate([35,0,0]) {
                cube([width, height, 50]);
            }
        }
        
        // cable exit
        translate([(width/2)-7, (height/2) + 32, -0.1]) {
            cube([14, 3, 10]);
        }
        
        // cut out for sensor
        translate([(width/2)-14, (height/2) + 32, 8.1]) {
            cube([28, 20, 39]);
        }
        
        // Screw holes for BME680 sensor.
        translate([(width/2)-10, (height/2) + 40-8, 48-18]) {
            rotate([90,0,0]) {
                #cylinder(d=4.2, h=8.1);
            }
        }
        
        translate([(width/2)+10, (height/2) + 40-8, 48-18]) {
            rotate([90,0,0]) {
                #cylinder(d=4.2, h=8.1);
            }
        }
        
        mountingHoles();
    }
}