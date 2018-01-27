// Spacer for Lid to upside down lid.
// to allow for an up-side-down lid to be mounted on top for tool storage.

// 70mm M4 bolt.
// 65mm thread (excludes head/nut space).
// 60mm (2x2.5mm thickness for lids)
height = 20;

$fn=180;

difference() {
    union() {
        cylinder(d=14, h=height);
    }
    union() {
        // Cover the M4 nut from the fan
        cylinder(d=9, h=4);
        
        // M4 heat fit insert to hold the spacer on.
        cylinder(d=7, h=15);
               
        // M3 insert to attach the cover plate.
        translate([0,0,height-10]) {
            cylinder(d=4.3, h=height);
        }
    }
}