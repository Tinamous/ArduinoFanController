$fn=180;

module fan40mm() {
}

module fan120mm(x,y) {
    
fanDiameter = 120;
    
    translate([x,y]) {
        
        // Indication only
        %square(fanDiameter);
        
        
        // Cutouts.
        // Fan middle
        translate([fanDiameter/2, fanDiameter/2]) {
            circle(d=fanDiameter-10);
        }
        
        // Mounting holes......
        translate([7.5, 7.5, 0]) {
            #circle(d=4.5);
        }
        translate([fanDiameter-7.5, 7.5, 0]) {
            #circle(d=4.5);
        }
        translate([fanDiameter-7.5, fanDiameter-7.5, 0]) {
            #circle(d=4.5);
        }
        translate([7.5, fanDiameter-7.5, 0]) {
            #circle(d=4.5);
        }
    }
}

module caseFront() {
    // Outline...
    // 120 fan + 10mm space to either side + 2x 5mm overlap joints, 2x10mm space.
    width = (4*120) + (5*10) + 10;
    echo("width", width);
    difference() {
        square([enclosureWidth, enclosureHeight]);
        translate([0.1, 0.1]) square([enclosureWidth-0.2, enclosureHeight-0.2]);
    }

    // Each fan...
    for (fan = [0: 1: 3]) {
        // 10mm spacing between fans, 15mm space on left.
        xOffset = (fan * 130) +  15;
        echo("xOffset", xOffset);
        fan120mm(xOffset,15);
    }
}

module caseBack() {
    // Outline...
    // 120 fan + 10mm space to either side + 2x 5mm overlap joints, 2x10mm space.
    width = (4*120) + (5*10) + 10;
    echo("width", width);
    difference() {
        square([enclosureWidth, enclosureHeight]);
        translate([0.1, 0.1]) square([enclosureWidth-0.2, enclosureHeight-0.2]);
    }

    // Or do these go on the top.
    // two "large" (ca. 190x190) openings for filter.
}

enclosureWidth = 540;
enclosureHeight = 150;
enclosureDepth = 200;

module caseTop() {
    difference() {
        // 540 wide, 150mm deep
        square([enclosureWidth, enclosureDepth]);
        translate([0.1, 0.1]) square([enclosureWidth-0.2, enclosureDepth-0.2]);
    }

}

module caseBottom() {
    difference() {
        square([enclosureWidth, enclosureDepth]);
        translate([0.1, 0.1]) square([enclosureWidth-0.2, enclosureDepth-0.2]);
    }
}

module caseLeftSide() {
    difference() {
        square([enclosureDepth, enclosureHeight]);
        translate([0.1, 0.1]) square([enclosureDepth-0.2, enclosureHeight-0.2]);
    }
}

module caseRightSide() {
    difference() {
        square([enclosureDepth, enclosureHeight]);
        translate([0.1, 0.1]) square([enclosureDepth-0.2, enclosureHeight-0.2]);
    }
}

//---------------------------------

translate([0,-(enclosureDepth+10)] ) {
    color("green") caseBottom();
}

color("red") caseFront();

translate([0, 160]) {
    color("green") caseTop();
}

translate([0,160 + enclosureDepth + 10]) {
    color("red") caseBack();
}

translate([-(enclosureDepth+10),0]) {
    color("blue") caseLeftSide();
}

translate([enclosureWidth+10,0]) {
    color("blue") caseRightSide();
}