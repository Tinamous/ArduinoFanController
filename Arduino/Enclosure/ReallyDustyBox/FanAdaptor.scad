// fan x,y
// 120mm fan.
//width = 120;
//height = 120;
//holeOffset = 7.5;

// 140mm fan
width = 140;
height = 140;
holeOffset = 7.5;
coneHeight = 40;

echo("fan width",width );
echo("fan height", height);
echo("fan holeOffset", holeOffset);

ductSize = 100; // to match 100mm dicting

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
            }
            
            mountingHoles(holeOffset);
        }
    }
}

module filterMount() {
    difference() {
        union(){
            translate([width/2, height/2, 0]) {
                cylinder(d=width, h=8);
            }
            
            // Add a cone onto the end to make the bridging required to be printed
            // a lot more managagle.
            translate([(width/2), (height/2), 8]) {
                cylinder(d1=width,d2=ductSize, h=coneHeight);
            }        
            
            translate([width/2, height/2, coneHeight + 8]) {
                cylinder(d=ductSize, h=20);
            }    
        }
        union() {
            translate([(width/2), (height/2), -0.01]) {
                cylinder(d=width-4, h=8.02);
            }            
            
            translate([(width/2), (height/2), 8-0.01]) {
                cylinder(d1=width-4,d2=ductSize-4, h=coneHeight + 0.02);
            }            
            
            translate([(width/2), (height/2), coneHeight + 8]) {
                cylinder(d=ductSize-4, h=20.1);
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
    translate([x,y,-0.1]) {
        #cylinder(d=5, h=5);
    }
}

union() {
    fanMount();
    filterMount();
        
}