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
width = 150;
height = 150;
fanDiameter = 140;
holeOffset = 12.5;

echo("fan width",width );
echo("fan height", height);
echo("fan holeOffset", holeOffset);

totalDepth = 260; // Maxing out the 
//totalDepth = 160; // Maxing out Ultimaker 2+
//totalDepth = 95; // Debugging
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
        
        noCornersCube(width, height, depth, cornerRadius);
    }
}


module fanMount() {
    
    difference() {
        union(){
            // to pad it out +10
            roundedCube(width,height,40, 8);
        }
        union() {
            
            // 2mm thick base with a fan size round cone inside
            translate([(width)/2, (height)/2, -0.1]) {
                cylinder(d=fanDiameter, h=40.2);
                // was 114 (width-6)
            }
            
            mountingHoles(holeOffset);
        }
    }
}

module filter() {
    intersection() {
        union() {
            for(x = [5 : 1.8 : width-5]) {
                translate([x, 4, 0]) {
                    echo("x",x);
                    #cube([1.2, height-5, 2]);
                }
            } 
            
            for(y = [5 : 10 : height-5]) {
                translate([4, y, 2]) {
                    echo("y",y);
                    #cube([width-5, 1.2, 2]);
                }
            } 
        }
        union() {
            translate([(width)/2, (height)/2, -0.1]) {
                cylinder(d=fanDiameter+2, h=4);
            // was 114 (width-6)
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
        // 6mm for M4 heatfit
        cylinder(d=6, h=depth);
    }
}

union() {
    fanMount();
    filter();
        
}