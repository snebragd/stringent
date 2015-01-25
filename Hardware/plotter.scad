//todo
//hål för kontakter

include <arduino.scad>
include <spool2.scad>

plate_w = 100;
plate_h = 135;
plate_t = 1;

distances_h=17;

plate_board_dist = 20;

$fn=40;

//base
difference() {
	union() {
		cube(center=true, [plate_w,plate_h,plate_t]);
		edge();
	}

	//stepper holes
	translate([-25,plate_h/2-10,0]) stepperholes();
	translate([25, plate_h/2-10,0]) stepperholes();

	//uno holes
	translate([35,plate_h/2-91,0])
		rotate([0,0,90])
			union() {
				uno_holes();
				translate([0,0,-5]) components(component=HEADER_F);	
			}

	//pen hole
	translate([0,plate_h/2-105,-5]) cylinder(r=9, h=10);

	//extra holes
	translate([-34,plate_h/2-117,-5]) cylinder(r=10, h=10);
	translate([34,plate_h/2-117,-5]) cylinder(r=10, h=10);
	translate([-16,5,-5]) cylinder(r=13, h=10);
	translate([16,5,-5]) cylinder(r=13, h=10);

	//servo zip-tie holes
	translate([-12.5,plate_h/2-127,0]) cube(center=true,[2,4,10]);	
	translate([12.5,plate_h/2-127,0]) cube(center=true,[2,4,10]);	

	//stepper driver holes 
	translate([-34,-49]) rotate([0,0,90]) stepper_driver_holes();	
	translate([34,-49]) rotate([0,0,90]) stepper_driver_holes();		

	//9v zip tie holes
	translate([0,plate_h/2-52]) cube(center=true,[6,3,10]);	
	translate([0,plate_h/2-35]) cube(center=true,[6,3,10]);	

	//corner holes
	corner_holes();
}

//spools
translate([-25,plate_h/2-10,5]) spool(20, 8, 3.3, 1);
translate([25,plate_h/2-10,5]) spool(20, 8, 3.3, 1);


//uno components
//translate([35,plate_h/2-91,-distances_h]) rotate([0,0,90]) components();

//stepper driver distances 
difference() {
	union() {
		translate([-34,-49]) rotate([0,0,90]) stepper_driver_pins();	
		translate([34,-49]) rotate([0,0,90]) stepper_driver_pins();	
	}
	translate([-34,-49]) rotate([0,0,90]) stepper_driver_holes();	
	translate([34,-49]) rotate([0,0,90]) stepper_driver_holes();		
}


//wire guide
difference() {
	union() {
		translate([-3.5,-plate_h/2,0]) cylinder(r=3,h=16);
		translate([3.5,-plate_h/2,0]) cylinder(r=3,h=16);
		translate([-3.5,-plate_h/2+1.5,3]) rotate([0,90,0]) cylinder(r=1,h=7);
		translate([-3.5,-plate_h/2+1.5,7]) rotate([0,90,0]) cylinder(r=1,h=7);
		translate([-3.5,-plate_h/2+1.5,11]) rotate([0,90,0]) cylinder(r=1,h=7);
		translate([-3.5,-plate_h/2+1.5,15]) rotate([0,90,0]) cylinder(r=1,h=7);
		translate([-0.5,-plate_h/2+1.5,0]) cube([1,19,16]);
	}
	translate([-50,-plate_h/2-100,-1]) cube([100,100,100]);
	translate([-1,-plate_h/2-5,20]) rotate([-33,0,0]) cube([2,40,16]);

	translate([0,plate_h/2-127,plate_t]) cube(center=true,[10,4,2]);		
}

//pen guide
difference() {
	translate([0,plate_h/2-105,0]) cylinder(r1=12, r2=10, h=50);
	translate([0,plate_h/2-105,-5]) cylinder(r=9, h=200);
	translate([0,plate_h/2-105,120]) rotate([-30,0,0]) cube(center=true,[100,100,1000]);	
	translate([-7.5,plate_h/2-105,20]) cube(center=true,[2,20,4]	);
	translate([7.5,plate_h/2-105,20]) cube(center=true,[2,20,4]	);
}

translate([-40,-plate_h/2+60,0]) distance(distances_h);
translate([-40,-plate_h/2+70,0]) distance(distances_h);
translate([40,-plate_h/2+60,0]) distance(distances_h);
translate([40,-plate_h/2+70,0]) distance(distances_h);

//distances
module distance(height=17) {
difference() {
	translate([0,0,0]) cylinder(r=3, h=height);
	translate([0,0,-1]) cylinder(r=2, h=distances_h+2);
}
}

// ************ modules **************

module corner_holes() {
	translate([plate_w/2-4, plate_h/2-4, -50]) cylinder(r=2, h=100);
//	translate([plate_w/2-4, -plate_h/2+4, -50]) cylinder(r=2, h=100);
//	translate([-plate_w/2+4, -plate_h/2+4, -50]) cylinder(r=2, h=100);
	translate([-plate_w/2+4, plate_h/2-4, -50]) cylinder(r=2, h=100);
}

module stepper_driver_holes() {
	dx=30;
	dy=26;
	union() {
		translate([-dx/2,dy/2,-5]) cylinder(r=1.7, h=10);
		translate([-dx/2,-dy/2,-5]) cylinder(r=1.7, h=10);
		translate([dx/2,-dy/2,-5]) cylinder(r=1.7, h=10);
		translate([dx/2,dy/2,-5]) cylinder(r=1.7, h=10);
	}
}
 
module edge() {
	difference() {
		union() {
			difference() {
				translate([0,0,2]) cube(center=true, [plate_w,plate_h, plate_t+3]);
				translate([0,0,-1]) cube(center=true, [plate_w-4,plate_h-4, 20]);
			}
			translate([0,plate_h/2-46,1.5]) cube(center=true, [3,92, 3]);
			translate([0,24,1.5]) cube(center=true, [plate_w,3, 3]);
			translate([0,-15,1.5]) cube(center=true, [plate_w,3, 3]);
		}
		translate([0,70,6]) cube(center=true, [plate_w+1,plate_h/2, 10]);
	}
}

module driver_pin() {	
	cylinder(r=2.5, h=4);
//	cylinder(r=1.3, h=4.7+2);
}

module stepper_driver_pins() {
	dx=30;
	dy=26;
	union() {
		translate([-dx/2,dy/2,0]) driver_pin();
		translate([-dx/2,-dy/2,0]) driver_pin();
		translate([dx/2,-dy/2,0]) driver_pin();
		translate([dx/2,dy/2,0]) driver_pin();
	}
}

//arduino
//import("Slim_Arduino_Uno_Case/arduino-case_bottom.stl");

module uno_holes() {
	holes = [
		[  2.54, 15.24,-5 ],
		[  17.78, 66.04,-5 ],
		[  45.72, 66.04,-5 ],
		[  50.8, 13.97,-5 ]
		];
	union(){
		for(i = holes ) {
			translate(i) 
				union() {
					cylinder(r=1.7,h=10);
					translate([0,0,5.5]) cylinder(r=4,h=10);
				}				
		}
	}
}

module uno_connector_holes() {

}

module stepperholes() {
	translate([0,0,-50])cylinder(r=5, h=100);
	translate([35/2,  -8, -50]) cylinder(r=2, h=100);
	translate([-35/2, -8, -50]) cylinder(r=2, h=100);
}

