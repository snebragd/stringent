//todo
//hål för kontakter

include <arduino.scad>
include <spool2.scad>

draw_components=0;

plate_w = 100;
plate_h = 135;
plate_t = 1;

distances_h=6;

plate_board_dist = 20;

$fn=40;

arduino_x=30;
arduino_y=73;
arduino_rotation=180;

pen_y=plate_h/2-105;
pen_angle=-45;
pen_radius=10;

stepper_x=27;
stepper_y=-(plate_h/2-15);

driver_x=34;
driver_y=-15;

battery_x=0;
battery_y=0;

servo_x=0;
servo_y=-87;

//base
difference() {
	union() {
		cube(center=true, [plate_w,plate_h,plate_t]);
		edge();

		//servo mount
		translate([0,-plate_h/2-23/2,0]) cube(center=true, [23,23,plate_t]);		
	}

	//stepper holes
	translate([-stepper_x, stepper_y,0]) stepperholes();
	translate([stepper_x, stepper_y,0]) stepperholes();

	//uno holes
	translate([arduino_x,arduino_y,0]) {
		rotate([0,0,arduino_rotation])
			union() {
				uno_holes();
			}
	}

	//pen hole
	translate([0,pen_y,0]) rotate([pen_angle,0,0]) translate([0,0,-50]) cylinder(r=pen_radius, h=100);

	//extra holes
	translate([-34,-15,-5]) cylinder(r=12, h=10);
	translate([34,-15,-5]) cylinder(r=12, h=10);
	translate([-11,-15,-5]) cylinder(r=8, h=10);
	translate([11,-15,-5]) cylinder(r=8, h=10);
	translate([0,36,-5]) cylinder(r=25, h=10);
	translate([-36,18,-5]) cylinder(r=10, h=10);
	translate([36,18,-5]) cylinder(r=10, h=10);
	translate([-36,47,-5]) cylinder(r=10, h=10);
	translate([36,47,-5]) cylinder(r=10, h=10);
	translate([0,-79,-5]) cylinder(r=8, h=10);

	//servo zip-tie holes
//	translate([-12.5,plate_h/2-127,0]) cube(center=true,[2,4,10]);	
//	translate([12.5,plate_h/2-127,0]) cube(center=true,[2,4,10]);	

	//stepper driver holes 
	translate([-driver_x,driver_y]) rotate([0,0,90]) stepper_driver_holes();	
	translate([driver_x,driver_y]) rotate([0,0,90]) stepper_driver_holes();		

	//9v zip tie holes
//	translate([0,plate_h/2-52]) cube(center=true,[6,3,10]);	
//	translate([0,plate_h/2-35]) cube(center=true,[6,3,10]);	

	//corner holes
	corner_holes();
}

if(draw_components) {
	//spools
	color("White", alpha=0.2) {
		translate([-stepper_x,stepper_y,2]) spool(15, 8, 3.3, 1);
		translate([stepper_x,stepper_y,2]) spool(15, 8, 3.3, 1);	
	}

	//steppers
	translate([-stepper_x,stepper_y,0]) stepper(); 
	translate([stepper_x,stepper_y,0]) stepper();	
	
		
	//uno components
	translate([arduino_x,arduino_y,distances_h]) rotate([0,0,arduino_rotation]) arduino();
	
	//battery
	translate([battery_x,battery_y,-17*3/2]) {
		battery();
	}

	//stepper drivers
	color("Green", alpha=0.5) {	
		translate([driver_x, driver_y,5]) cube(center=true, [32,35,1]);
		translate([-driver_x, driver_y,5]) cube(center=true, [32,35,1]);
	}	

	//servo
	translate([servo_x, servo_y, -12/2-plate_t/2]) servo();

	//pen
	color("Red", alpha=0.5) {	
		translate([0,pen_y,0]) rotate([pen_angle,0,0]) union() {
			translate([0,0,-10]) cylinder(r=pen_radius-1, h=100);
			translate([0,0,-30]) cylinder(r1=1, r2=pen_radius-1, h=20);
		}
	}
}

//stepper driver distances 
difference() {
	union() {
		translate([-driver_x, driver_y]) rotate([0,0,90]) stepper_driver_pins();	
		translate([driver_x,driver_y]) rotate([0,0,90]) stepper_driver_pins();	
	}
	translate([-driver_x,driver_y]) rotate([0,0,90]) stepper_driver_holes();	
	translate([driver_x,driver_y]) rotate([0,0,90]) stepper_driver_holes();		
}


//wire guide
difference() {
	union() {
		translate([-6.0,-plate_h/2,0]) cylinder(r=5,h=24);
		translate([6.0,-plate_h/2,0]) cylinder(r=5,h=24);
		translate([-3.5,-plate_h/2+1.5,3]) rotate([0,90,0]) cylinder(r=1,h=7);
		translate([-3.5,-plate_h/2+1.5,7]) rotate([0,90,0]) cylinder(r=1,h=7);
		translate([-3.5,-plate_h/2+1.5,11]) rotate([0,90,0]) cylinder(r=1,h=7);
		translate([-3.5,-plate_h/2+1.5,15]) rotate([0,90,0]) cylinder(r=1,h=7);
		translate([-3.5,-plate_h/2+1.5,19]) rotate([0,90,0]) cylinder(r=1,h=7);
		translate([-3.5,-plate_h/2+1.5,23]) rotate([0,90,0]) cylinder(r=1,h=7);
		translate([-0.5,-plate_h/2+1.5,0]) cube([1,29,24]);
	}
	translate([-50,-plate_h/2-100,-1]) cube([100,100,100]);
	translate([-1,-plate_h/2+5,20]) rotate([-33,0,0]) translate([0,-10,2]) cube([2,40,24]);

	translate([0,pen_y,0]) rotate([pen_angle,0,0]) {
			translate([0,0,-50]) cylinder(r=pen_radius, h=250);
	}
}

//pen guide
intersection() {
	translate([0,pen_y,0]) rotate([pen_angle,0,0]) {
		difference() {
			union() {
				translate([0,0,-50]) cylinder(r1=pen_radius+2, r2=pen_radius+1, h=120);
				translate([-2,20,0]) rotate([-pen_angle,0,0]) cube([4,20,40]);
			}
			translate([0,0,-50]) cylinder(r=pen_radius, h=250);
			translate([0,0,150]) rotate([-25,0,0]) cube(center=true,[100,100,1000]);	
			translate([-7.5,5,20]) cube(center=true,[3,10,6]	);
			translate([7.5,5,20]) cube(center=true,[3,10,6]	);
		}
	}
	translate([-100,-100,0]) cube(200,200,200);
}

//arduino distances
translate([arduino_x,arduino_y,0])
	rotate([0,0,arduino_rotation])
		uno_holes(distances_h);

// ************ modules **************

//distances
module distance(height=17) {
	difference() {
		translate([0,0,0]) cylinder(r=3, h=height);
		translate([0,0,-1]) cylinder(r=1.7, h=distances_h+2);
	}
}

module driver_pin() {	
	cylinder(r=2.8, h=5);
}

module corner_holes() {
	translate([plate_w/2-4, plate_h/2-4, -50]) cylinder(r=2, h=100);
   translate([-plate_w/2+4, plate_h/2-4, -50]) cylinder(r=2, h=100);
}

module stepper_driver_holes() {
	dx=30;
	dy=26;
	union() {
		translate([-dx/2,dy/2,-5]) cylinder(r=1.7, h=11);
		translate([-dx/2,-dy/2,-5]) cylinder(r=1.7, h=11);
		translate([dx/2,-dy/2,-5]) cylinder(r=1.7, h=11);
		translate([dx/2,dy/2,-5]) cylinder(r=1.7, h=11);
	}
}
 
module edge() {

	//servo tab
	difference() {
		translate([0,-plate_h/2-23/2,plate_t/2+2-0.1]) cube(center=true, [23,23,4]);		
		translate([0,-plate_h/2-23/2,plate_t/2+2]) cube(center=true, [19,19,4.5]);		
		translate([0,-plate_h/2-23/2-4,plate_t/2+4]) cube(center=true, [40,6,4.5]);		
	}

	difference() {
		union() {
			difference() {
				translate([0,0,2.4]) cube(center=true, [plate_w,plate_h, plate_t+3]);
				translate([0,0,-1]) cube(center=true, [plate_w-4,plate_h-4, 20]);
			}
			translate([0,plate_h/2-46,1.5]) cube(center=true, [3,92, 3]);
			translate([0,3,1.5]) cube(center=true, [plate_w,3, 3]);
			translate([0,-32,1.5]) cube(center=true, [plate_w,3, 3]);
		}
		translate([-stepper_x,stepper_y,1.5]) cylinder(r=19, h=10); 
		translate([stepper_x,stepper_y,1.5]) cylinder(r=19, h=10);	
		
	}
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

module uno_holes(make_distances=0) {
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
					if(make_distances > 0) {
						translate([0,0,5]) distance(make_distances);
					}
					else {
						cylinder(r=1.7,h=10);
						//translate([0,0,5.5]) cylinder(r=4,h=10);
					}
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

module stepper() {
	color("Goldenrod", alpha=0.8) {
		translate([0,0,0])cylinder(r=2.5, h=8);
	}
	color("Silver", alpha=0.5) {
		translate([0,0,-1])cylinder(r=5, h=1);
		translate([0, -8, -20]) cylinder(r=14, h=19);
	
		difference() {
			union() {
				translate([-35/2,  -8, -2]) cylinder(r=3.5, h=1);
				translate([35/2,  -8, -2]) cylinder(r=3.5, h=1);
				translate([-35/2,  -8-3.5, -2]) cube([35,7,1]);
			}
			translate([-35/2,  -8, -4]) cylinder(r=2, h=10);
			translate([35/2,  -8, -4]) cylinder(r=2, h=10);		
		}	
	}
	color("Blue", alpha=0.5) {
		translate([-7.5,-8-18,-18]) cube([15,18,17]);
	}
}

module battery() {
	translate([-45/2, -26/2,17/2]) {
		color("Black", alpha=0.5) {
			cube([45,26,17]);
		}
		color("Silver", alpha=0.5) {
			translate([0,0,17/2]) rotate([0,-90,0]) {
				translate([0,13-6,0]) cylinder(r=3,h=3);
				translate([0,13+6,0]) difference() {
					cylinder(r=4,h=3,$fn=6);
					cylinder(r=3,h=4,$fn=6);
				}
			}
		}
	}
}

module servo() {

	color("Black", alpha=0.5) {	
		cube(center=true, [23,21,12]);		
		translate([0,21/2-3,0]) cube(center=true, [32,1,12]);		
		translate([23/2-11.5/2,21/2,0]) rotate([-90,0,0]) {
			union() {
				cylinder(r=11.5/2, h=5);
				cylinder(r=2, h=10);
				translate([-3,-3, 9]) cube([6,20,1]);
			}
		}
	}
	color("White", alpla=0.5) {

	}
}
