// 24 ring Neopixel and switch holder.

$fn=100;
neoPixelDepth=3.2; // 3.1 (round as .5)
depth = 6;
switchCutoutDiameter = 29;
//outerDiameter = 68.5; // When circular
outerDiameter = 73; // When 12 sided

neoPixelRingOuter = 66.6;
neoPixelRingInner = 52.2;

module neoPixelRing(outer, inner, height) {
    difference() {
        union() {
            cylinder(d=outer +0.2,h= height+0.2);
        }
        
        union() {
            cylinder(d=inner - 0.2,h=height+0.2);
        }
    }
}

module neoPixelWires() {
    // Holes are fixed regarless of side.
    translate([(65.6-3)/2, 0, -depth]) {
        cylinder(d=2.5, h=depth*2);
    }
    
    rotate([0,0,210]) {
        translate([(65.6-3)/2, 0, -depth]) {
            cylinder(d=2.5, h=depth*2);
        }
    }
    
    rotate([0,0,150]) {
        translate([(65.6-3)/2, 0, -depth]) {
            cylinder(d=2.5, h=depth*2);
        }
    }
}

module main() {
    difference() {
        union() {
            cylinder(d=outerDiameter, h=depth,$fn=12);
        }
        union() {
            // Switch
            translate([0,0,-0.1]) {
                cylinder(d=switchCutoutDiameter, h=depth+0.2);
                neoPixelWires();
                
                translate([0,0,depth - neoPixelDepth]) {
                    neoPixelRing(neoPixelRingOuter, neoPixelRingInner, neoPixelDepth);

                    translate([0,0,-1.5]) {
                        // recess for wires.
                        neoPixelRing(65.6-1, 65.6-5, 2);
                    }
                }
            }
        }
    }
}

main();
    
module test() {

    neoPixelRing(65.6, 52, neoPixelDepth);

    translate([0,0,neoPixelDepth]) {
        // recess for wires.
        neoPixelRing(65.6-1, 65.6-3, 1.5);
    }
}