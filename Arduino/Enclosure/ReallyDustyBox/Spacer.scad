// Spacer for Lid to upside down lid.
// to allow for an up-side-down lid to be mounted on top for tool storage.

// 70mm M4 bolt.
// 65mm thread (excludes head/nut space).
// 60mm (2x2.5mm thickness for lids)
height = 60;

$fn=180;

difference() {
    cylinder(d=14, h=height);
    cylinder(d=6, h=height);
}