
height = 22;

$fn=180;

difference() {
    union() {
        // 8mm works nice
        cylinder(d1=12,d2=7, h=height);
    }
    union() {
        translate([0,0,-0.01]) {
            // Cover the M4 nut from the fan
            cylinder(d=9, h=4);
            
            // 14mm of M4 bolt max sticking out from fan case.
            // Min: 4 + 4mm bold.
            // Fan + case = 30mm.
            // 40mm M4 bolt from fan side should work well....
            // M4 heat fit insert to hold the spacer on.
            cylinder(d=6.5, h=10+4);
                   
            // M3 insert to attach the cover plate.
            translate([0,0,height-8.5]) {
                cylinder(d=5, h=8.6);
            }
        }
    }
}