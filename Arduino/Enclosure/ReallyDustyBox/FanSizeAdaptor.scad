// Adaptor for a 120mm fan to sit in a 140mm fan slot.

thickness = 3;

$fn=80;

// These are ignored at repsent.
outerSize = 140;
innerSize = 120;

module fanMountinghole(x,y) {
    translate([x,y,-0.1]) {
        cylinder(d=5, h=thickness+0.2);
    }
}

module fan140Mountingholes() {
offset = 7.5;
    fanMountinghole(offset,offset);
    fanMountinghole(140-offset,offset);
    fanMountinghole(140-offset,140-offset);
    fanMountinghole(offset,140-offset);
}

module fan120MountingHoles() {
offset = 7.5;
    fanMountinghole(offset,offset);
    fanMountinghole(120-offset,offset);
    fanMountinghole(120-offset,120-offset);
    fanMountinghole(offset,120-offset);
}

module fan120MainHole() {
    translate([60,60,-0.1]) {
        cylinder(d=114, h=thickness+0.2);
    }
}

difference() {
    union() {
        cube([140,140,thickness]);
    }
    union() {
        fan140Mountingholes();
        translate([10,10,0]) {
            fan120MountingHoles();
            fan120MainHole();
        }
    }
}