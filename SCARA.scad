use <StepMotor_28BYJ-48.scad>;
use <Hub_28BYJ-48.scad>;
include <Bearing_6807-2RS.scad>;

// global rendering parameters
$fn = 24;

// library examples
// translate([100, 100, 10]) { StepMotor28BYJ(); }
// stepperHub(10, 7, 6, 3.1, 1.5, 2.75);
//translate([0, 0, 8]) bearing6807_2RS();
// bearingInnerStep(bearing6807_2RS_d - iFitAdjust, 2, 2);
//shoulderBase(2, 55, 9.1, 3.1);

// interference fit adjustment for 3D printer
iFitAdjust = .4;

// shoulder base parameters
shoulderBaseHeight = 3;
shoulderBaseDiameter = 55;
shaftBossDiameter = 10;
mountScrewDiameter = 3.1;
bearingStep = 2; // size in mm of bearing interface step
// joint parameters
hubHeight = 10;
hubRadius = 7;
shaftHeight = 6;
shaftRadius = 3.1;
setScrewRadius = 1.5;
setScrewHeight = 2.75;
spokeWidth = 3;
spokes = 6;
screwTabs = 4;
screwTabHeight = 4;
armLength = 100;
// misc
baseDeckExtension = 60;
boundingBox = 8;
boundingBoxHalf = boundingBox / 2;
// 8 is distance from shaft center to screw center on x axis
// 35 is screw center to screw center on y axis
shaftCenterToMountHoldCenterXAxis = 8;
mountHoleCenterToMountHoleCenter = 35;
leadScrewDiameter = 8;
// forearm
forearmLength = 75;

/* Pieces in this model
** Shoulder:
*** Base plate
*** Top plate
*** Lower arm
*** Upper arm
** Forearm:
*** ?
** Hand:
*** ?
*/

// shoulder base
color([.7, .7, 1]) 
    shoulderBase(bearing6807_2RS_d - iFitAdjust, bearing6807_2RS_D + iFitAdjust, shoulderBaseHeight, shoulderBaseDiameter, shaftBossDiameter, mountScrewDiameter);
// shoulder lower stepper
rotate([0, 180, 0]) 
    translate([-8, 0, 10]) 
        StepMotor28BYJ();
// lower shoulder bearing
translate([0, 0, shoulderBaseHeight + bearingStep])
    %bearing6807_2RS();
// shoulder top
color([.6, .6, .9]) 
    translate([0, 0, 2 * shoulderBaseHeight + (4 * bearing6807_2RS_B) + (8 * bearingStep)])
    rotate([180, 0, 0])
        shoulderBase(bearing6807_2RS_d - iFitAdjust, bearing6807_2RS_D + iFitAdjust, shoulderBaseHeight, shoulderBaseDiameter, shaftBossDiameter, mountScrewDiameter);
// upper shoulder bearing
translate([0, 0, shoulderBaseHeight + (3 * bearing6807_2RS_B) + (7 * bearingStep)])
    %bearing6807_2RS();
// shoulder upper stepper
rotate([0, 0, 180]) 
    translate([-8, 0, 10 + 2 * shoulderBaseHeight + (4 * bearing6807_2RS_B) + (8 * bearingStep)]) 
        StepMotor28BYJ();
// shoulder spacers
color([.7, .6, .2])
shoulderSpacers(bearing6807_2RS_D + iFitAdjust, 0 * shoulderBaseHeight + (4 * bearing6807_2RS_B) + (8 * bearingStep));
// end shoulder

// lower arm including the shoulder - arm joint 
//render()
color([1, .7, .7]) 
    translate([0, 0, shoulderBaseHeight + bearing6807_2RS_B]) 
        armLower(bearing6807_2RS_D + iFitAdjust, bearingStep, bearingStep, hubHeight, hubRadius, shaftHeight, shaftRadius, setScrewRadius, setScrewHeight, spokeWidth, spokes, screwTabs, screwTabHeight, armLength);
// upper arm
//render()
color([.9, .7, .7]) 
    translate([0, 0, shoulderBaseHeight + ( 2*bearing6807_2RS_B) + (12 * bearingStep) - (.5 * bearingStep)]) 
        rotate([180, 0, 0])
            armLower(bearing6807_2RS_D + iFitAdjust, bearingStep, bearingStep, hubHeight, hubRadius, shaftHeight, shaftRadius, setScrewRadius, setScrewHeight, spokeWidth, spokes, screwTabs, screwTabHeight, armLength);
// elbow joint pieces
// lower bearing
translate([-armLength, 0, shoulderBaseHeight + bearing6807_2RS_B + bearingStep])
    %bearing6807_2RS();
// arm upper stepper
translate([-armLength + 8, 0, shoulderBaseHeight + ( 3*bearing6807_2RS_B) + (16 * bearingStep) - (.5 * bearingStep)]) 
    rotate([0, 0, 180]) 
        StepMotor28BYJ();
// arm lower stepper
rotate([0, 180, 0]) 
    translate([armLength - 8, 0, bearingStep * 2]) 
        StepMotor28BYJ();
// arm spacers
color([.6, .6, .3])
    armSpacers(bearing6807_2RS_D + iFitAdjust, 0 * shoulderBaseHeight + (4 * bearing6807_2RS_B) + (8 * bearingStep));
// end arm
        
// forearm pieces
// lower
color([.1, .7, .1])
translate([-armLength, 0, shoulderBaseHeight + (bearing6807_2RS_B * 2) + (4 *bearingStep)])
//translate([-armLength, 0, shoulderBaseHeight + (bearing6807_2RS_B * 2) + (14 *bearingStep)])
    forearmLower(bearing6807_2RS_d - iFitAdjust, bearing6807_2RS_D + iFitAdjust, bearingStep, bearingStep, hubHeight + (bearing6807_2RS_B / 2) + (bearingStep/2), hubRadius, shaftHeight, shaftRadius, setScrewRadius, setScrewHeight, spokeWidth, spokes, forearmLength, boundingBox);
// upper
color([.2, .8, .2])
translate([-armLength, 0, shoulderBaseHeight + (bearing6807_2RS_B * 2) + (4 *bearingStep)])
    rotate([180, 0, 0])
        forearmLower(bearing6807_2RS_d - iFitAdjust, bearing6807_2RS_D + iFitAdjust, bearingStep, bearingStep, hubHeight + (bearing6807_2RS_B / 2) + (bearingStep/2), hubRadius, shaftHeight, shaftRadius, setScrewRadius, setScrewHeight, spokeWidth, spokes, forearmLength, boundingBox);
// forearm upper bearing
translate([-armLength, 0, shoulderBaseHeight + (2 * bearing6807_2RS_B) + (7 * bearingStep)])
    %bearing6807_2RS();
// forearm spacers

/*
deprecated
render() 
    translate([-armLength, 0, shoulderBaseHeight + bearing6807_2RS_B + (2* bearingStep)])
        armJointSpacer(bearing6807_2RS_d - iFitAdjust, bearing6807_2RS_D + iFitAdjust, bearingStep, shaftBossDiameter, mountScrewDiameter);
*/

//bearingInnerStep(bearing6807_2RS_d - iFitAdjust, 2, 2);
module bearingInnerStep(bearingID, stepHeight, stepWidth) {
    difference() {
        union() {
            // this fits inside of the bearing
            cylinder(h = stepHeight * 2, d = bearingID);
            // this rests on the bearing inner lip
            cylinder(h = stepHeight, d = bearingID + stepWidth);
        }
        // remove center
        cylinder(h = stepHeight * 2, d = bearingID - (stepWidth * 2));
    }
}

//translate([0, 0, 12]) bearingOuterStep(bearing6807_2RS_D + iFitAdjust, 2, 2);
module bearingOuterStep(bearingOD, stepHeight, stepWidth) {
    //render(convexivity = 3)
    difference() {
        cylinder(h = stepHeight * 2, d = bearingOD + stepWidth);
        cylinder(h = stepHeight * 2, d = bearingOD - stepWidth);
        cylinder(h = stepHeight, d = bearingOD);
    }
}

module shoulderSpacers(bearingOD, screwSpacerHeight) {
    union() {
        // screw holes for joining to arm
        rotate([0, 0, -30])
        radial_array_partial(vec = [0, 0, 1], n = 6, 2)
            translate([bearingOD / 2 + (setScrewRadius * 8), 0, shoulderBaseHeight * 1.0])
        difference() {
            union () {
                cylinder(h = screwSpacerHeight, r = setScrewRadius * 2);
                
                }
            cylinder(h = screwTabHeight, r = setScrewRadius);
        }       
    }
}

module armSpacers(bearingOD, screwSpacerHeight) {
    union() {
        // screw holes for joining to arm
        radial_array(vec = [0, 0, 1], n = 4)
            translate([bearingOD / 2 + (setScrewRadius * 2), 0, shoulderBaseHeight + bearing6807_2RS_B + 2 * bearingStep])
        difference() {
            union () {
                cylinder(h = bearing6807_2RS_B * 2 + 4 * bearingStep, r = setScrewRadius * 2);
                
                }
            cylinder(h = bearing6807_2RS_B * 2 + 4 * bearingStep, r = setScrewRadius);
        }       
    }
}

// armJointSpacer(bearing6807_2RS_D - iFitAdjust, bearing6807_2RS_D + iFitAdjust, bearingStep, shaftBossDiameter, mountScrewDiameter);
module armJointSpacer(bearingID, bearingOD, bearingStep, shaftBossDiameter, mountScrewDiameter) {
    union() {
        difference() {
            cylinder(h = bearingStep * 2, d = bearingOD + bearingStep);
            cylinder(h = bearingStep * 2, d = bearingID);
        }
        translate([0, 0, bearingStep * 2])
                bearingInnerStep(bearingID, bearingStep, bearingStep);
        // screw holes for joining to arm
        radial_array(vec = [0, 0, 1], n = screwTabs)
                translate([bearingOD / 2 + (setScrewRadius * 2), 0, (2 * bearingStep) - screwTabHeight])
                    difference() {
                        union () {
                            cylinder(h = screwTabHeight, r = setScrewRadius * 2);
                            translate([-2 * setScrewRadius, - 2 * setScrewRadius, 0])
                                cube([setScrewRadius * 2, 4 * setScrewRadius, screwTabHeight], center = false);
                        }
                        cylinder(h = screwTabHeight, r = setScrewRadius);
                    }       
    }
}


//shoulderBase(2, 55, 9.1, 3.1);
// WARNING: has some hard-coded non-parametric values in here!
module shoulderBase(bearingID, bearingOD, shoulderBaseHeight, shoulderBaseDiameter, shaftBossDiameter, mountScrewDiameter) {
    mountHoleDepth = shoulderBaseHeight;

    //render(convexivity = 3)
    difference() {
        union () {
            cylinder(h = shoulderBaseHeight, d = shoulderBaseDiameter);
            translate([0, 0, shoulderBaseHeight])
                bearingInnerStep(bearing6807_2RS_d - iFitAdjust, bearingStep, bearingStep);
            // 40 is the length of deck extension from the baseplate in x
            translate([0, -shoulderBaseDiameter / 2, 0])
                cube([baseDeckExtension, shoulderBaseDiameter, shoulderBaseHeight], center = false);
        }
        // motor shaft hole
        cylinder(h = shoulderBaseHeight + (bearingStep * 2), d = shaftBossDiameter);
        // mounting holes
        
        translate([shaftCenterToMountHoldCenterXAxis, mountHoleCenterToMountHoleCenter/2, 0]) 
            cylinder(h = mountHoleDepth, d = mountScrewDiameter);
        translate([shaftCenterToMountHoldCenterXAxis, -mountHoleCenterToMountHoleCenter/2, 0]) 
            cylinder(h = mountHoleDepth, d = mountScrewDiameter);
        // lead screw hole
        translate([baseDeckExtension - (leadScrewDiameter * 1.5), 0, 0])
            cylinder(h = shoulderBaseHeight, d = leadScrewDiameter + (leadScrewDiameter / 2));
        // upper to lower spacer screw holes
        rotate([0, 0, -30])
            radial_array_partial(vec = [0, 0, 1], n = 6, 2)
                translate([bearingOD / 2 + (setScrewRadius * 6), 0, shoulderBaseHeight * 0])
                    cylinder(h = screwTabHeight, r = setScrewRadius);
           
    }
    // NOTE: need to add LM8UU mounts
}




module armLower(bearingOD, stepHeight, stepWidth, hubHeight, hubRadius, shaftHeight, shaftRadius, setScrewRadius, setScrewHeight, spokeWidth, spokes, screwTabs, screwTabHeight, armLength) {
    armWidth = bearingOD / 2;
    //render(convexivity = 3)
    difference() { 
    union() {
        armInnerJoint(bearingOD, bearingStep, bearingStep, hubHeight, hubRadius, shaftHeight, shaftRadius, setScrewRadius, setScrewHeight, spokeWidth, spokes, screwTabs, screwTabHeight);
        translate([-armLength, 0, 0])
            armOuterJointBase(bearingOD, bearingStep, setScrewRadius, screwTabs, screwTabHeight);
        difference() {
            translate([-armLength, -armWidth / 2, 0])
                cube([armLength, armWidth, bearingStep * 2], center=false);
            cylinder(h = bearingStep * 2, d = bearingOD);
            translate([-armLength, 0, 0])
                cylinder(h = bearingStep * 2, d = bearingOD);
            translate([-armLength + (bearingOD / 2) + (boundingBox / 2), -(armWidth / 2) + boundingBoxHalf, 0])
                cube([armLength - bearingOD - (boundingBox + boundingBoxHalf), armWidth - boundingBox, bearingStep * 2], center = false);
        }
        difference() {
            intersection() {
            
                translate([-armLength + (bearingOD / 2) + (boundingBox / 2), -(armWidth / 2) + boundingBoxHalf, 0])
                    cube([armLength - bearingOD - boundingBox, armWidth - boundingBox, bearingStep * 2], center = false);
                for (i = [ bearingOD / 2 - boundingBoxHalf: armWidth : armLength]) {
                    translate([ -i, 0, bearingStep])
                        rotate([0, 0, 45])
                            cube([spokeWidth, armWidth + boundingBoxHalf, bearingStep * 2], center = true);
                    translate([ -i, 0, bearingStep])
                        rotate([0, 0, -45])
                            cube([spokeWidth, armWidth + boundingBoxHalf, bearingStep * 2], center = true);
                }
            }
        
        }
   }
           // screw holes for joining to upper
        radial_array(vec = [0, 0, 1], n = screwTabs)
                translate([bearingOD / 2 + (setScrewRadius * 2), 0, (2 * stepHeight) - screwTabHeight])
                       cylinder(h = screwTabHeight, r = setScrewRadius);
   }
}

module armOuterJointBase(bearingOD, bearingStep, setScrewRadius, screwTabs, screwTabHeight) {
    mountHoleDepth = bearingStep * 2;

    union() {
        difference() {
            union() {
                translate([0, 0, -(bearingStep * 2)])
                    cylinder(h = bearingStep * 2, d = bearingOD + bearingStep);
                translate([0, 0, bearingStep * 2])
                    rotate([0, 180, 0])
                        bearingOuterStep(bearingOD, bearingStep, bearingStep);
            }
            // motor shaft hole
            translate([0, 0, -(bearingStep * 2)])
                cylinder(h = shoulderBaseHeight + (bearingStep * 2), d = shaftBossDiameter);
            // mounting holes
            translate([0, 0, -(bearingStep * 2)]) {
                translate([shaftCenterToMountHoldCenterXAxis, mountHoleCenterToMountHoleCenter/2, 0]) 
                cylinder(h = mountHoleDepth, d = mountScrewDiameter);
                translate([shaftCenterToMountHoldCenterXAxis, -mountHoleCenterToMountHoleCenter/2, 0]) 
                cylinder(h = mountHoleDepth, d = mountScrewDiameter);
            }
        }
        
    }
}

//armInnerJoint(bearing6807_2RS_D + iFitAdjust, bearingStep, bearingStep, hubHeight, hubRadius, shaftHeight, shaftRadius, setScrewRadius, setScrewHeight, spokeWidth, spokes, screwTabs, screwTabHeight);
module armInnerJoint(bearingOD, stepHeight, stepWidth, hubHeight, hubRadius, shaftHeight, shaftRadius, setScrewRadius, setScrewHeight, spokeWidth, spokes, screwTabs, screwTabHeight) {
    //translate([0, 0, hubHeight - (2 * stepHeight)])
    difference() {
        union() {
         difference() {
            cylinder(h = bearingStep * 2, d = bearingOD + (setScrewRadius * 6));
            cylinder(h = bearingStep * 2, d = bearingOD);
//            translate([0, -bearingOD, 0])
//                cube([bearingOD, bearingOD * 2, bearingStep * 2]);
        }
        bearingOuterStep(bearingOD, stepHeight, stepWidth);
        // hub
        translate([0, 0, -(hubHeight - (2 * stepHeight))])
            stepperHub(hubHeight, hubRadius, shaftHeight, shaftRadius, setScrewRadius, setScrewHeight);
        // spokes
        radial_array(vec=[0, 0, 1], n = spokes)
                translate([hubRadius - (stepWidth / 2), -(spokeWidth / 2), stepHeight]) 
                    cube([(bearingOD / 2) - hubRadius, spokeWidth, stepHeight], center = false);
        // screw tabs for joining to upper
        radial_array(vec = [0, 0, 1], n = screwTabs)
                translate([bearingOD / 2 + (setScrewRadius * 2), 0, (2 * stepHeight) - screwTabHeight])
                    
                        union () {
                            cylinder(h = screwTabHeight, r = setScrewRadius * 2);
                            translate([-2 * setScrewRadius, - 2 * setScrewRadius, 0])
                                cube([setScrewRadius * 2, 4 * setScrewRadius, screwTabHeight], center = false);
                        }
                        
                    }
        // screw holes for joining to upper
        radial_array(vec = [0, 0, 1], n = screwTabs)
                translate([bearingOD / 2 + (setScrewRadius * 2), 0, (2 * stepHeight) - screwTabHeight])
                       cylinder(h = screwTabHeight, r = setScrewRadius);
    }
}


module forearmLower(bearingID, bearingOD, stepHeight, stepWidth, hubHeight, hubRadius, shaftHeight, shaftRadius, setScrewRadius, setScrewHeight, spokeWidth, spokes, forearmLength, boundingBox) {
    armWidth = bearingOD / 2;

    rotate([180, 0, 0]) union() {
        boundingBoxHalf = boundingBox / 2;
        difference() {
            cylinder(h = bearingStep * 2, d = bearingOD + bearingStep);
            cylinder(h = bearingStep * 2, d = bearingID - (2 * bearingStep));
        }
        translate([0, 0, bearingStep * 2])
            bearingInnerStep(bearingID, bearingStep, bearingStep);
        // screw holes for joining to arm
        radial_array(vec = [0, 0, 1], n = screwTabs)
                translate([bearingOD / 2 + (setScrewRadius * 2), 0, (2 * bearingStep) - screwTabHeight])
                    difference() {
                        union () {
                            cylinder(h = screwTabHeight, r = setScrewRadius * 2);
                            translate([-2 * setScrewRadius, - 2 * setScrewRadius, 0])
                                cube([setScrewRadius * 2, 4 * setScrewRadius, screwTabHeight], center = false);
                        }
                        cylinder(h = screwTabHeight, r = setScrewRadius);
                    }
        // hub
        translate([0, 0, hubHeight]) rotate([180, 0, 0]) 
            stepperHub(hubHeight, hubRadius, shaftHeight, shaftRadius, setScrewRadius, setScrewHeight);
        // spokes
        radial_array(vec=[0, 0, 1], n = spokes)
                translate([hubRadius - (stepWidth / 2), -(spokeWidth / 2), 0]) 
                    cube([(bearingOD / 2) - hubRadius, spokeWidth, 2 *stepHeight], center = false);   
        // forearm extension
             difference() {
            translate([-forearmLength, -armWidth / 2, 0])
                cube([forearmLength, armWidth, bearingStep * 2], center=false);
            cylinder(h = bearingStep * 2, d = bearingOD);
            translate([-forearmLength, 0, 0])
                cylinder(h = bearingStep * 2, d = bearingOD / 2);
                 // 4 on next line comes from dividing the bearingOD on the above line
            translate([-forearmLength + (bearingOD / 4) + (boundingBox / 2), -(armWidth / 2) + boundingBoxHalf, 0])
                cube([forearmLength - (bearingOD/2) - (boundingBox + boundingBoxHalf), armWidth - boundingBox, bearingStep * 2], center = false);
        }

            intersection() {
            
                translate([-forearmLength + boundingBox * 2, -(armWidth / 2) + boundingBoxHalf, 0])
                    cube([forearmLength - (bearingOD / 2) - boundingBox, armWidth - boundingBox, bearingStep * 2], center = false);
                for (i = [ bearingOD / 2 - boundingBoxHalf: armWidth : forearmLength]) {
                    translate([ -i, 0, bearingStep])
                        rotate([0, 0, 45])
                            cube([spokeWidth, armWidth + boundingBoxHalf, bearingStep * 2], center = true);
                    translate([ -i, 0, bearingStep])
                        rotate([0, 0, -45])
                            cube([spokeWidth, armWidth + boundingBoxHalf, bearingStep * 2], center = true);
                }
            }
        
       
    }
}


module copy_mirror(vec=[0,1,0]) { 
    children(); 
    mirror(vec)
        children(); 
} 

module radial_array(vec = [0,0,1], n = 4) {
    for ( i = [0 : n - 1] )
    {
        rotate( i * 360 / n, vec)
            children();
    }   
}

module radial_array_partial(vec = [0,0,1], n = 4, j = 2) {
    for ( i = [0 : n - 1] )
    {
        if (i < j)
            rotate( i * 360 / n, vec)
                children();
    }   
}
