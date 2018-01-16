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
totalDepth = 160; // Maxing out Ultimaker 2+
//totalDepth = 60; // Debugging
coneHeight = 80;
depth = totalDepth - coneHeight;
echo("totalDepth", totalDepth);
echo("depth", depth);

coneMinDiameter = 44;
slotDepth = 1.5;  

$fn=80;


// TODO: Inner cylinder to reduce the amount of activated charcoal in the system
// and provide a less resistant path for air to exit.

// 

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

module cutReducingVentHoles(height, slotWidth) {
     
    //step = 2;
    step = slotDepth * 2;
    //step = 20; // Debug
    for(z = [0 : step : (height/3) *2]) {
    
        translate([0, 0, z]) {
            for(angle = [0 : 30 : 180]) {
                rotate([0,0,angle]) {
                   # cube([slotWidth - z/5, width+10 , slotDepth], center=true);
                }
            }
        }
    }
        
}

module cutVentHoles(height, slotWidth) {
     
    step = slotDepth * 2;
    //step = 20; // Debug
    for(z = [0 : step : (height/3) *2]) {
    
        translate([0, 0, z]) {
            for(angle = [0 : 30 : 180]) {
                rotate([0,0,angle]) {
                   # cube([slotWidth, width+10 , slotDepth], center=true);
                }
            }
        }
    }
        
}

module hollowCone(diameter, height) {
    difference() {
        union() {
            cylinder(d1=diameter,d2=coneMinDiameter, h=height);
        }
        union() {
            translate([0, 0, -0.01]) {
                cylinder(d1=diameter-3,d2=coneMinDiameter, h=height-1.5);
            }
            cutReducingVentHoles((height*3/2)-2, 24);            
        }
    }
}

module hollowCylinder(diameter, height) {
    difference() {
        union(){
            translate([0, 0, 0]) {
                cylinder(d=diameter, h=height);
            }
            
            // Add a cone onto the end to make the bridging required to be printed
            // a lot more managagle.
            translate([0, 0, height]) {
                hollowCone(diameter, coneHeight);           
            }            
        }
        union() {
            translate([0, 0, -0.01]) {
                cylinder(d=diameter-3, h=height+0.02);
            }                                
            
            // Skip the first 1/3
            translate([0, 0, height/3]) {
                cutVentHoles(height, 24);
            }
        }
    }
}

// A round filter "cartridge" that hangs from the underneath of the fan.
module filterMount() {
    difference() {
        union(){
            translate([width/2, height/2, 0]) {
                //cylinder(d=width, h=depth);
                hollowCylinder(width, depth);
            }
            // Add a cone onto the end to make the bridging required to be printed
            // a lot more managagle.
            //translate([(width/2), (height/2), -(depth + coneHeight)]) {
             //   cylinder(d2=width,d1=44, h=coneHeight);
            //}            
        }
        union() {
            /*
            translate([(width/2), (height/2), -(depth)-0.01]) {
                cylinder(d=width-4, h=depth+0.02);
            }            
            
            translate([(width/2), (height/2), -(depth + coneHeight-2)]) {
                cylinder(d2=width-4,d1=38, h=coneHeight-1.98);
            } 
 */           
            
            step = 2;
            step = 20; // Debug
            for(z = [-(depth-4)-10 : step : -(depth/3)]) {
            
                translate([(width)/2, (height/2), z]) {
                    for(angle = [0 : 30 : 180]) {
                        rotate([0,0,angle]) {
                            cube([24, width+10, 1], center=true);
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