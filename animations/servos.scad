// Library of servos.

// Servo horn parameters
SH_sOD = 7;   //Shaft side outer diameter
SH_eOD = 4;   //Edge side outer diameter
SH_l = 17.2;  //Total length
SH_t = 1.6;   //Thickness
SH_sh = 4.5;  //Shaft side full height
SH_sd = 4.6;  //Shaft diameter
SH_si = 2.4;  //Shaft inset into the horn - how deep does the shaft fit into the horn
SH_sr = 1;    //Screw recess - the amount of recess for the screw in the horn top
SH_srd = 4.8; //Screw recess diameter
SH_shd = 2.3; //Screw hole diameter
SH_lh = 5;    //Number of link holes
SH_lhd = 1;   //Link hole diameter

// 9g Micro servo parameters
SRV_w = 22.5;   // Body width
SRV_lh = 15.9;  // Lower body height - up to bottom of mounting tabs
SRV_d = 11.8;   // Body depth
SRV_tw = 4.7;   // Width of one tab
SRV_th = 2.5;  // Height of tab
SRV_uh = 22.7-SRV_th-SRV_lh;  // Upper body height - from top of mounting tabs to body top
SRV_gh = 4;   // Height of top round gear extrusion
SRV_bgd = 5;  // Top back smaller gear extrusion diameter
SRV_sd = 4.6;     // Shaft diameter
SRV_sh = 2.75;   // Shaft height
SRV_mhd = 2;  // Mounting hole diameter
SRV_fh = SRV_lh+SRV_th+SRV_uh+SRV_gh+SRV_sh; // The full servo height
SRV_fhh = SRV_fh+SH_sh-SH_si;   // Servo full height including horn

/**
 * Module to draw a servo.
 *
 * The servo is parametric, but draws a 9g Servo shape specifically.
 *
 * @param w   Body width
 * @param lh  Lower body height - up to bottom of mounting tabs
 * @param d   Body depth
 * @param tw  Width of one tab
 * @param th  Height of tab
 * @param uh  Upper body height - from top of mounting tabs to body top
 * @param gh  Height og top round gear extrusion
 * @param bgd Top back smaller gear extrusion diameter
 * @param sd  Shaft diameter
 * @param sh  Shaft height
 * @param mhd Mounting hole diameter
 * @param wt Wire thickness
 * @param wo Wire offset from bottom
 **/
module Servo(w=SRV_w, d=SRV_d, lh=SRV_lh, uh=SRV_uh, tw=SRV_tw, th=SRV_th,
             gh=SRV_gh, bgd=SRV_bgd, sd=SRV_sd, sh=SRV_sh, mhd=SRV_mhd,
             wt=1.2, wo=4.2) {
    color("blue", 0.5) {
        // Lower body
        cube([w, d, lh]);
        // Mounting tab layer on top of lower body
        translate([-tw, 0, lh])
            difference() {
                cube([w+2*tw, d, th]);
                // Left mounting hole and opening
                translate([tw/2, d/2, -1])
                    cylinder(h=th+2, d=mhd);
                translate([0, d/2-mhd/4, -1])
                    cube([tw/2, mhd/2, th+2]);
                // Right mounting hole and opening
                translate([w+tw*2-tw/2, d/2, -1])
                    cylinder(h=th+2, d=mhd);
                translate([w+tw*2-tw/2, d/2-mhd/4, -1])
                    cube([tw/2, mhd/2, th+2]);
            }
        // Upper body on top of that
        translate([0, 0, lh+th])
            cube([w, d, uh]);
        // Gearbox extrusion top left
        translate([d/2, d/2, lh+th+uh])
            cylinder(h=gh, d=d);
        // The smaller gear extrusion behind the big one
        translate([d, d/2, lh+th+uh])
            cylinder(h=gh, d=bgd);
    }
    // The shaft
    color("white") {
        translate([d/2, d/2, lh+th+uh+gh]) 
            difference() {
                cylinder(h=sh, d=sd);
                cylinder(h=sh+1, d=sd/4);
            }
    }
    // The wires
    for (y=[[d/2-wt,"orange"], [d/2,"red"], [d/2+wt,"brown"]]) {
        translate([0, y[0], wo+wt/2])
            rotate([0, -90, 0])
                color(y[1])
                    cylinder(h=tw, d=wt);
    }
}

/**
 * A Single sided servo horn.
 *
 * @param sOD Shaft side outer diameter
 * @param eOD Edge side outer diameter
 * @param l Total length
 * @param t Thickness
 * @param sh Shaft side full height
 * @param sd Shaft diameter
 * @param si Shaft inset into the horn - how deep does the shaft fit into the horn
 * @param sr Screw recess - the amount of recess for the screw in the horn top
 * @param srd Screw recess diameter
 * @param shd Screw hole diameter
 * @param lh Number of link holes
 * @param lhd Link hole diameter
 **/
module ServoHorn(sOD=SH_sOD, eOD=SH_eOD, l=SH_l, t=SH_t, sh=SH_sh, sd=SH_sd,
                 si=SH_si, sr=SH_sr, srd=SH_srd, shd=SH_shd, lh=SH_lh,
                 lhd=SH_lhd) {
    difference() {
        union() {
            // The shaft side outer cylinder
            cylinder(h=sh, d=sOD);
            // The horn part
            translate([0, 0, sh-t])
                hull() {
                    cylinder(h=t, d=sOD);
                    translate([l-(sOD+eOD)/2, 0, 0])
                        cylinder(h=t, d=eOD);
                }
        }
        translate([0, 0, -1]) {
            // Screw hole
            cylinder(d=shd, h=sh+2);
            // Shaft insert
            cylinder(h=si+1, d=sd);
        }
        // Top screw recess
        translate([0, 0, sh-sr])
            cylinder(d=srd, h=sr+1);

        // Distance between first and last link hole center
        linkHolesDist = l - sOD - lhd/2 - eOD/2;
        // Distance between link hole centers
        lhcd = linkHolesDist/(lh-1);
        // Link holes
        translate([sOD/2+lhd/2, 0, 0])
            for(c=[0:lh-1]) {
                translate([c*lhcd, 0, 0])
                    cylinder(h=sh+1, d=lhd);
            }
    }
}

/**
 * Servo with horn using default config
 *
 * @param ha horn angle
 **/
module ServoAndHorn(ha=-90) {
    Servo();
    // The servo horn
    translate([SRV_d/2, SRV_d/2, SRV_fh-SH_si])
        rotate([0, 0, ha])
            color("white")
                ServoHorn();
}

module ServoDemos() {
    Servo();
    // Servo horn
    translate([0, -10, 0])
        ServoHorn();
    // Servo and horn
    translate([0, 30, 0])
        ServoAndHorn();
}
