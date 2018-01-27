// Spacer for Lid to upside down lid.
// to allow for an up-side-down lid to be mounted on top for tool storage.

// 70mm M4 bolt.
// 65mm thread (excludes head/nut space).
// 60mm (2x2.5mm thickness for lids)
height = 20;

$fn=180;

difference() {
    union() {
        cylinder(d=10, h=height);
    }
    union() {
        // 6mm perfect for a heat fit.
        cylinder(d=5, h=height);
        
        //recess part way so that the bolt on the box lid 
        // can have a nut to hold it captive when the support is not used.
        
        translate([0,0,height-4]) {
            cylinder(d=7, h=4.1);
        }
    }
}