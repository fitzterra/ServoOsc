use <./servos.scad>
$fn=100;

angle=0; //[-90:5:90]
inverted=false;

ServoAndHorn(angle+(inverted?180:0));

