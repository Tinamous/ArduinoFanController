// fan x,y
// 120mm fan.
//width = 120;
//height = 120;
//holeOffset = 7.5;

// 140mm fan
width = 150;
height = 150;
fanDiameter = 140;
holeOffset = 12.5; // for a 140 fan with a 140 base.
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
            roundedCube(width,height,3, 8);
        }
        union() {
            
            translate([width/2, height/2, -0.01]) {
                cylinder(d=fanDiameter-4, h=4.1);
            }
            
            mountingHoles(holeOffset);
        }
    }
}

module hulled(diameter, ductWidth, ductHeight) {
    
    hull() {
        translate([width/2, height/2, 0]) {
            cylinder(d=diameter, h=8);
        }
        translate([(width-ductWidth)/2, (height-ductHeight)/2, 60]) {
            cube([ductWidth,ductHeight,1]);
        }
    }
    translate([(width-ductWidth)/2, (height-ductHeight)/2, 60]) {
            cube([ductWidth,ductHeight,10]);
    }
}

module hollowHull() {
    
diameter = fanDiameter;
    
// For the vent to drop into.
// These are OUTER measurements of the duct.
ductWidth = 107; // 106.0
ductHeight = 51; // 49.9
    
    difference() {
        hulled(diameter, ductWidth+4, ductHeight+4);
        translate([0, 0, 0]) {
            hulled(diameter-4,ductWidth, ductHeight);
        }
    }
}

module mountingHoles(holeOffset) {
    // 2 large hoes to go around the fan mount strew
    // so the fan can stay attached. then 2 small
    // to run the fan bold through
    mountingHole(holeOffset,holeOffset, 11);
    mountingHole(width-holeOffset,holeOffset, 5);
    mountingHole(width-holeOffset,height-holeOffset, 10);
    mountingHole(holeOffset,height-holeOffset,5);
}

module mountingHole(x,y, d) {
    translate([x,y,-0.1]) {
        #cylinder(d=d, h=5);
    }
}

module showDuct() {
    // For the vent to drop into.
// These are OUTER measurements of the duct.
ductWidth = 107; // 106.0
ductHeight = 51; // 49.9
    translate([(width-ductWidth)/2, (height-ductHeight)/2, 50]) {
            %cube([ductWidth,ductHeight,50]);
    }
}

showDuct();

union() {
    fanMount();
    hollowHull();       
}