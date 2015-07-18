// Hub for 28BYJ-48 stepper motor w/ set screw
// Example for 3mm set screw:
//stepperHub(10, 7, 6, 3.1, 1.5, 2.75);
module stepperHub(height, radius, shaftHeight, shaftRadius, setScrewRadius, setScrewHeight) {
    //render(convexity = 3)
    difference() {
        // hub
        cylinder(h = height, r = radius, center = false);
        // shaft
        intersection() {
            cylinder(h = shaftHeight, r = shaftRadius, center = false);
            translate([-shaftRadius, -shaftRadius/2, 0])
                cube(size = [shaftRadius * 2., shaftRadius, shaftHeight], center = false);
        }
        // set screw hole
        translate([0, 0, setScrewHeight])
            rotate([90, 0, 0]) 
                cylinder(h = (radius * 2) + radius, r = setScrewRadius, center = true);
    }
}
