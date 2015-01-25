
$fn=60;

//spool(15, 8, 3.3, 1);

//spool(20, 8, 3.3, 1);

module spool(radius, height, rim_dr, rim_h) {
	difference() {
		cylinder(r=radius+1*rim_dr+0*(height/2-rim_h), h=height);
		union() {
			axle();
			translate([0, 0, 2])	difference() {
				cylinder(r=0.8*radius, h=height-2+1);
				cylinder(r=5, h=height-2);
//				translate([0,0,height-4]) cylinder(r1=5, r2=5+1, h=2);
//				translate([0,0,0]) cylinder(r2=5, r1=5+1, h=2);
			}
			translate([0, 0, height/2]) torus(radius+(height/2-rim_h), height/2-rim_h);
			for (i=[0:5]) {
	  	 		rotate([0, 0, i*360/6])
				translate([5+4, 0, -1]) cylinder(r=2.5, h=height+2);
			}
		}		
		
		//hole for string
		rotate([0,90,0]) translate([-height/2,0,0.5*radius]) cylinder(r=1.5,h=radius,$fn=4);
	}
}


module axle () {
	stepper_axle_r = 2.5;
	stepper_axle_d = 3;
	stepper_axle_l = 10;
	eps = 0.2;

	translate([0,0,-1])
	intersection() { 
		cylinder(r=stepper_axle_r+eps, h=stepper_axle_l+2);
		translate([0, 0, stepper_axle_l/2])
			cube([stepper_axle_d+2*eps, 2*(stepper_axle_r+eps), stepper_axle_l+2], center=true);
	}
}

module torus (Radius, radius) {
	rotate_extrude() {
		translate([Radius, 0, 0]) union() {
			circle($fn=4, r=radius, center=true);
			translate([radius,0,0]) square(center=true, [radius*2, radius*2]);
		}
	}
}
