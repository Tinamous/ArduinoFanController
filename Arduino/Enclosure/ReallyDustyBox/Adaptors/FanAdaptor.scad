// fan x,y
// 120mm fan.
//width = 120;
//height = 120;
//holeOffset = 7.5;

// 140mm fan
width = 150;
height = 150;
holeOffset = 12.5;
coneHeight = 40;
fanDiameter = 140;
ductSize = 108; // to match 100mm dicting

echo("fan fanDiameter",fanDiameter);
echo("width", width);
echo("height", height);
echo("fan holeOffset", holeOffset);
echo("duct Size", ductSize);



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
                #cylinder(d=fanDiameter-4, h=4.1);
            }
            
            mountingHoles(holeOffset);
        }
    }
}

module filterMount() {
    difference() {
        union(){
            translate([width/2, height/2, 0]) {
                cylinder(d=fanDiameter, h=8);
            }
            
            // Add a cone onto the end to make the bridging required to be printed
            // a lot more managagle.
            translate([(width/2), (height/2), 8]) {
                cylinder(d1=fanDiameter,d2=ductSize, h=coneHeight);
            }        
            
            translate([width/2, height/2, coneHeight + 8]) {
                cylinder(d=ductSize, h=20);
            }    
            
            translate([(width/2), (height/2), coneHeight + 8 + 20]) {
                //cylinder(d1=ductSize, d2= ductSize-8, h=10.1);
            } 
        }
        union() {
            translate([(width/2), (height/2), -0.01]) {
                cylinder(d=fanDiameter-4, h=8.02);
            }            
            
            translate([(width/2), (height/2), 8-0.01]) {
                cylinder(d1=fanDiameter-4,d2=ductSize-4, h=coneHeight + 0.02);
            }            
            
            translate([(width/2), (height/2), coneHeight + 8]) {
                cylinder(d=ductSize-4, h=20.1);
            } 
            
            translate([(width/2), (height/2), coneHeight + 8 + 20]) {
               // #cylinder(d1=ductSize-4, d2= ductSize-10, h=10.1);
            } 
        }
    }
}

module mountingHoles(holeOffset) {
    mountingHole(holeOffset,holeOffset, 5);
    mountingHole(width-holeOffset,holeOffset, 5);
    mountingHole(width-holeOffset,height-holeOffset, 5);
    mountingHole(holeOffset,height-holeOffset, 5);
}

module mountingHole(x,y, d) {
    translate([x,y,-0.1]) {
        #cylinder(d=d, h=5);
    }
}

union() {
    fanMount();
    filterMount();
        
}