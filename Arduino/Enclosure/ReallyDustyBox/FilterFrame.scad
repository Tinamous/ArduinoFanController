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

// 3mm for frame thickness, 20mm to hold the 25mm deep filter.
depth = 20;

// Inside mouting frame has extra holes
// to hold it in place when the outside filter
// is removed for cleaning.

// true for inside frame, false for outside frame.
isInsideFrame = false;


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

module mainFrame() {
    difference() {
        union(){
            roundedCube(width,height,depth, 6);
        }
        union() {
            mountingHoles();
            
            translate([3,3,3]) {
                //cube  ([width-6,height-6,depth-2.99]);
                roundedCube(width-6,height-6,depth-2.99, 4);
            }
            
            holesDown = 4;
            holesAcross = 4;
            
            yOffset = 10;
            availableHoleHeight = height-(2*yOffset);
            echo("availableHoleHeight",availableHoleHeight);
            holeHeight = availableHoleHeight/holesDown;
            echo("holeHeight",holeHeight);
            
            
            xOffset = 10;
            echo("width",width);
            availableHoleWidth = width-(2*xOffset);
            echo("availableHoleWidth",availableHoleWidth);
            holeWidth = availableHoleWidth/holesAcross;
            echo("holeWidth", holeWidth);
            
            for(y = [yOffset : holeHeight : height - holeHeight]) {
                
                
                
                //holeWidth = 52;
                for(x = [xOffset : holeWidth : width-holeWidth]) {
            
                    // Ultimaker Wall thickness is 1.5ishmm
                    // make our walls that size to save wall + filler
                    translate([x+1,y+1,-0.01]) {
                        //cube([holeWidth-2, holeHeight-2, depth+0.02]);
                        roundedCube(holeWidth-2, holeHeight-2, depth+0.02, 4);
                        //noCornersCube(holeWidth-2, holeHeight-2, depth+0.02, 4);
                    }
                }
            }
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