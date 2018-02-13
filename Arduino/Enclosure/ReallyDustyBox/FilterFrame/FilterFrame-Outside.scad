width = 190; // Ultimaker can't quite do 200x200.
height = 200;
$fn=180;


// Construction:
// ||||||||||||||||||||||||
// Fan
// Inside Filter Frame
// Filter
// Case Wall
// Inside Filter Retainer
// Filter
// Outside Filter Frame
// ||||||||||||||||||||||||


// Options:
// Dusty Box (DIY)
//   Intake through side fans
//   Carbon filter inside
//   Deep filter outside (primary)
//   Outer Depth = 18mm
// 
// Fume Box (Maker)
//   Outtake through side fans (solder fume / makers box)
//   Deep inside filters (primary/dust)
//   Carbon filters outside.
//   Outer Depth = 8mm

depth = 8;

// Inside mouting frame has extra holes
// to hold it in place when the outside filter
// is removed for cleaning.

// true for inside frame, false for outside frame.
isInsideFrame = false;

holesDown = 6;
holesAcross = 6;


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

insideFanSize = 185;

module mainFrame() {
    yOffset = 12;
    availableHoleHeight = height-(2*yOffset);
    echo("availableHoleHeight",availableHoleHeight);
    holeHeight = (availableHoleHeight/holesDown);
    echo("holeHeight",holeHeight);
    
    
    xOffset = 12;
    echo("width",width);
    availableHoleWidth = width-(2*xOffset);
    echo("availableHoleWidth",availableHoleWidth);
    holeWidth = availableHoleWidth/holesAcross;
    echo("holeWidth", holeWidth);
            
    
    difference() {
        union(){
            roundedCube(width,height,depth, 6);
        }
        union() {
            mountingHoles();
            
            if (isInsideFrame) {
                translate([width/2,height/2,-0.1]) {
                    #cylinder(d=insideFanSize, h=depth+0.2);
                    
                }
            } else {                        
                // Hollow out the main body.
                translate([xOffset,yOffset,-0.1]) {
                    //cube  ([width-6,height-6,depth-2.99]);
                    roundedCube(width-(2*xOffset),height-(2*yOffset),depth+0.2, 4);
                }
            }
            
            // Hollow out leaving a 3mm outer frame 
            translate([3,3,2]) {
                //cube  ([width-6,height-6,depth-2.99]);
                #roundedCube(width-6,height-6,depth, 4);
            }
        }
    }
    
    // Add the cross members to split the open space into a grid.
    // No support at first or last position.
    for(y = [yOffset + holeHeight : holeHeight : availableHoleHeight]) {
        translate([xOffset,y, 0]) {
            cube([availableHoleWidth, 2 , 2]);
        }
    }
        
    for(x = [xOffset + holeWidth : holeWidth : availableHoleWidth]) {
        translate([x,yOffset, 0]) {
            cube([2 , availableHoleHeight, 2]);
        }
    }
}

module mountingHoles() {
    mountingHole(6,6);
    mountingHole(width-6,6);
    mountingHole(width-6,height-6);
    mountingHole(6,height-6);
    
    mountingHole(6,height/2);
    mountingHole(width-6,height/2);
    
    if (isInsideFrame) {
        ReverseMountinghole(width/2,8);
        ReverseMountinghole(width/2,height -8);
    }
}

module mountingHole(x,y) {
    translate([x,y,-0.1]) {
        // Hole should be big enough for an M4 heat fit insert
        // and also for a normal M4 bolt to pass through.
        // Inside will have heat fit inserts in the "top"
        // outside counter sunk bold goes through to the inside
        cylinder(d=5.2, h=depth+2);
        
        // Countersink...
        #cylinder(d1=8.5, d2=5, h=3);
    }
}

// Used only on the inside frames to hold a M4 heat fit insert
// which is bolted through from the case to hold the inside in place
//
module ReverseMountinghole(x,y) {
       
    translate([x,y,-0.1]) {
        cylinder(d=5.2, h=depth+2);
    }
}


// Add some Structure around where we want a mounting point.
module mountingPoints() {
    mountingPoint(6,6);
    mountingPoint(width-6,6);
    mountingPoint(width-6,height-6);
    mountingPoint(6,height-6);
    
    mountingPoint(6,height/2);
    mountingPoint(width-6,height/2);
    
    if (isInsideFrame) {
        ReverseMountingPoint(width/2, 8);
        ReverseMountingPoint(width/2,height-8);
    }
    
}

fanHoleDistance = 154;
//fanHoleDistance = 170;
halfFanHoleDistance = fanHoleDistance/2;

module fanMountingHole(x,y) {
    translate([x,y,-0.1]) {
        // Hole should be big enough for an M4 heat fit insert
        // and also for a normal M4 bolt to pass through.
        // Inside will have heat fit inserts in the "top"
        // outside counter sunk bold goes through to the inside
        cylinder(d=5.2, h=depth+2);
    }
}

module fanMountingHoles() {
    translate([width/2, height/2, 0]) {
        fanMountingHole(-halfFanHoleDistance,-halfFanHoleDistance);
        fanMountingHole(halfFanHoleDistance,-halfFanHoleDistance);
        fanMountingHole(halfFanHoleDistance,halfFanHoleDistance);
        fanMountingHole(-halfFanHoleDistance,halfFanHoleDistance);
    }
}

module fanMountingPoints() {

    translate([width/2, height/2, 0]) {
        mountingPoint(-halfFanHoleDistance,-halfFanHoleDistance);
        mountingPoint(halfFanHoleDistance,-halfFanHoleDistance);
        mountingPoint(halfFanHoleDistance,halfFanHoleDistance);
        mountingPoint(-halfFanHoleDistance,halfFanHoleDistance);
    }
}


module mountingPoint(x,y) {
    translate([x,y,0]) {
        // 2mm short of the full frame
        // to allow for an inner frame to be dropped in
        // to hold the filter in place 
        cylinder(d=12, h=depth);
    }
}

module ReverseMountingPoint(x,y) {
    translate([x,y,0]) {
        // -x to allow for screw head when these are printed on the outside
        // frame
        cylinder(d=12, h=depth);
    }
}

module holeDrillingTemplate() {
    // mounting points.
    color("blue") {
        circleAt(6, 6, 4.5);
        circleAt(width-6, 6, 4.5);
        circleAt(width-6, height-6, 4.5);
        circleAt(6, height-6, 4.5);
        
        circleAt(6, height/2, 4.5);
        circleAt(width-6, height/2, 4.5);
    }
    
    yOffset = 12;
    availableHoleHeight = height-(2*yOffset);
    echo("availableHoleHeight",availableHoleHeight);
    holeHeight = (availableHoleHeight/holesDown);
    echo("holeHeight",holeHeight);
    
    
    // Big holes for air...
    xOffset = 12;
    echo("width",width);
    availableHoleWidth = width-(2*xOffset);
    echo("availableHoleWidth",availableHoleWidth);
    holeWidth = availableHoleWidth/holesAcross;
    echo("holeWidth", holeWidth);
    
    // Hole drilling template.
    for(y = [yOffset : holeHeight : availableHoleHeight]) {
        for(x = [xOffset : holeWidth : availableHoleWidth]) {
                centerX = x + (holeWidth/2)+1;
                centerY = y + (holeHeight/2)+1;
                color("red") circleAt(centerX, centerY, 2);
        }
    }
}

module circleAt(x,y, size) {
    echo("Hole at", x, y);
    translate ([x, y]) 
    {
        difference() {
            circle(d=size);
            circle(d=size/2);
        }
    }
}


module main() {
    union() {
        difference() {
            union() {
                mainFrame();
                mountingPoints();
                if (isInsideFrame) {
                    fanMountingPoints();
                    
                }
            }
            union() {
                mountingHoles();
                if (isInsideFrame) {
                    fanMountingHoles();
                }
            }
        }
    }
}

main();

//holeDrillingTemplate();