// Spacer for Lid to upside down lid.
// to allow for an up-side-down lid to be mounted on top for tool storage.

// 70mm M4 bolt.
// 65mm thread (excludes head/nut space).
// 60mm (2x2.5mm thickness for lids)
height = 10;

$fn=180;

difference() {
    union() {
        cylinder(d=8, h=height);
    }
    union() {
        cylinder(d=4.5, h=height);
    }
}