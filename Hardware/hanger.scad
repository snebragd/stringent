
thick=2;
space=2;
depth=22;
overshoot=2;
width=20;
tab_height=10;

difference() {
	cube([depth+overshoot+thick, 2*thick+space+tab_height, width]);

	translate([thick,2*thick+space,-1]) cube([depth+thick, tab_height, width+2]);

	translate([0,thick,-1]) cube([depth+overshoot, space, width+2]);
	translate([0,0,-1]) cube([overshoot, thick, width+2]);

	translate([-1,2*thick+space+tab_height/2,width/2]) rotate([0,90,0]) cylinder(r=2, h=thick+2, $fn=20);
}