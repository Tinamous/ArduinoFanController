width = 190; // Ultimaker can't quite fo 200x200.
height = 200;
$fn=180;


// Construction:
// ||||||||||||||||||||||||
// Inside Filter Frame
// Filter
// Case Wall
// Inside Filter Retainer
// Filter
// Outside Filter Frame
// ||||||||||||||||||||||||

// Inside Filter Frame: Depth = 25mm, isInsideFrame = true
// Inside Filter Retainer: Depth = 2mm, isInsideFrame = true
// Outside Filter Frame: Depth = 20mm, isInsideFrame = false

// 3mm for frame thickness (allow depth of M3 counter sink screw.
depth = 3;

// Inside mouting frame has extra holes
// to hold it in place when the outside filter
// is removed for cleaning.

// true for inside frame, false for outside frame.
isInsideFrame = false;

holesDown = 3;
holesAcross = 3;


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
    yOffset = 6;
    availableHoleHeight = height-(2*yOffset);
    echo("availableHoleHeight",availableHoleHeight);
    holeHeight = (availableHoleHeight/holesDown);
    echo("holeHeight",holeHeight);
    
    
    xOffset = 6;
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
                                  
            // Hollow out the main body.
            translate([xOffset,yOffset,-0.1]) {
                //cube  ([width-6,height-6,depth-2.99]);
                roundedCube(width-(2*xOffset),height-(2*yOffset),depth+0.2, 4);
            }
            
            // Hollow out leaving a 3mm outer frame 
            translate([3,3,2]) {
                //cube  ([width-6,height-6,depth-2.99]);
                roundedCube(width-6,height-6,depth, 4);
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
    
    // Use these holes to screw the filter frame
    // into the other frame (inside/outside)?
    #smallMountingHole(6,height/2);
    #smallMountingHole(width-6,height/2);
    
    // These holes are for the inside frame to be screwed to the 
    // box to keep it in place when the outside frame is removed.
    if (isInsideFrame) {
        ReverseMountinghole(width/2,8);
        ReverseMountinghole(width/2,height -8);
    }
}

module smallMountingHole(x,y) {
    translate([x,y,-0.1]) {
        // Hole should be big enough for an M4 heat fit insert
        // and also for a normal M4 bolt to pass through.
        // Inside will have heat fit inserts in the "top"
        // outside counter sunk bold goes through to the inside
        cylinder(d=3.5, h=depth+2);
        
        // Countersink...
        cylinder(d1=6, d2=3.5, h=3);
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
        cylinder(d1=8.5, d2=5, h=3);
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
            }
            union() {
                mountingHoles();
            }
        }
    }
}

main();

//holeDrillingTemplate();