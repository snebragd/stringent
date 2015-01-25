
$fn=60;
//axle();
//knob();

//spool(20, 8, 3.3, 1);

//plate();
//pen_holder();
//pulley_clamp();
//carriage();
//plate_clamp();
//carriage2();
//pen_holder2();
pen_guide_extension();
//pen_guide_adaptor();

module carriage2() {
        disk_r = 55;
        disk_t = 2;
        disk_rim = 6;
        disk_spokes_w = 6;
        disk_spokes_n = 3;
        pen_guide_r = 9.7;
        pen_guide_t = 2;
        pen_guide_h = 20+disk_t;
        inner_disk_r = pen_guide_r+pen_guide_t+2;
        mount_w = 31;
        mount_l = 18;
        mount_dist = pen_guide_r+pen_guide_t+20;
        groove_depth = 0.6;
        groove_kerf = 2;
        groove_h = pen_guide_h*0.85;

        difference() {
                cylinder(r=disk_r, h=disk_t);
                cylinder(r=disk_r-disk_rim, h=disk_t);
        }

        difference() {
                union() {
                        for (i=[0:disk_spokes_n-1])
                                rotate([0, 0, i*360/disk_spokes_n])
                                        translate([0, (disk_r-disk_rim/2)/2, disk_t/2])
                                                cube([disk_spokes_w, disk_r-disk_rim/2, disk_t], center=true);
                        cylinder(r=inner_disk_r, h=disk_t);
                        cylinder(r=pen_guide_r+pen_guide_t,h=pen_guide_h);
                }
                union() {
                        cylinder(r=pen_guide_r, h=pen_guide_h);
                        translate([0, 0, groove_h]) difference() {
                                cylinder(r=pen_guide_r+pen_guide_t, h=groove_kerf, center=true);
                                cylinder(r=pen_guide_r+pen_guide_t-groove_depth, h=groove_kerf, center=true);
                        }
                }
        }
        
        translate([mount_w/2, mount_l+mount_dist, 0])
                rotate([0, 0, 180]) servo_mount();

}

module pen_guide_extension() {
	pen_guide_r = 9.7;
	pen_guide_t = 2;
	pen_guide_h = 25;
	
	difference() {
		cylinder(r=pen_guide_r+pen_guide_t, h=pen_guide_h);
		cylinder(r=pen_guide_r, h=pen_guide_h);
	}
}

module pen_guide_adaptor() {
	or = 9.4;
	ir = 6.4;
	length = 32;
	gap = 2;

	difference() {
		cylinder(r=or, h=length);
		union() {
			cylinder(r=ir, h=length);
			translate([1.05*(or-ir)/2+ir, 0, length/2])
				cube([1.1*(or-ir), gap, length], center=true);
		}
	}
}

module plate_clamp() {
	motor_screw_r = 2;
	motor_l = 9;
	thickness = 2;
	width = 15;
	depth = 21;
	length = 25;
	difference() {
		cube([thickness, motor_l, width]);
		translate([0.5*thickness, motor_l/2+1, width/2])
			rotate([0, 90, 0]) 
				cylinder(r=motor_screw_r, h=1.05*thickness, center=true);
	}
	translate([0, -motor_l, 0]) cube([thickness, motor_l, width]);
	cube([depth+thickness, thickness, width]);
	translate([depth+thickness, -length+thickness, 0]) cube([thickness, length, width]);
}

module carriage() {
	translate([9, 0, 1]) hash_grid(w=18, h=70, N=2, M=4, t=2);

	translate([-20, 0, 1]) hash_grid(w=40, h=70, N=1, M=2, t=2);

	translate([0, -38, 0]) servo_mount();	
	translate([0, 38, 0]) mirror([0, 1, 0]) servo_mount();

	translate([-40-4/2, -35+4/2, 2/2]) 
		difference() {
			cube([4, 4, 2], center=true);
			cylinder(r=1, h=2, center=true);
		}

	translate([-40-4/2, +35-4/2, 2/2]) 
		difference() {
			cube([4, 4, 2], center=true);
			cylinder(r=1, h=2, center=true);
		}

}

module hash_grid (w, h, N, M, t) {
        for (n=[0:N]) {
                translate([-(w-t)/2+(n/N)*(w-t), 0, 0]) cube([t, h, t], center=true);
        }
        for (m=[0:M]) {
                translate([0, -(h-t)/2+(m/M)*(h-t), 0]) cube([w, t, t], center=true);
        }
}

module servo_mount() {
	motor_w = 23;
	motor_t = 14;
	motor_l = 18;
	thickness = 2;
	post_t = 2;
	post_w = 4;
	post_hole_r = 0.9;

	cube([motor_w+2*post_w, motor_l, thickness]);
	for (i=[0:1]) {
		translate([i*(motor_w+post_w)+post_w/2, motor_l-thickness+thickness/2, thickness+motor_t/2]) {
			rotate([0, 0, i*180]) difference() {
				cube([post_w, thickness, motor_t], center=true);
				translate([post_w/80, 0, 0]) 
					rotate([90, 0, 0]) 
						cylinder(r=post_hole_r, h=thickness*1.05, center=true);
			}
 		}
	}
}

module pulley_clamp() {
	width = 21;
	length = 25;
	height = 20;
	thickness = 2;
	r1 = 2;
	r2 = 1;
	cube([thickness, length, height]);
	translate([width+thickness, 0, 0]) cube([thickness, length, height]);
	translate([0, -thickness, 0]) cube([width+2*thickness, thickness, height]);
	translate([-(r1-r2), length, r2]) torus(r1, r2);
}

module pen_holder() {
	pen_r = 8.7;
	thickness = 2.5;
	height = 20;	
	lever_h = 6;
	lever_l = 25;
	lever_t = 2;

	tab_t = 1;
	tab_l = 5;
	gap = 2;

	difference() {
		union() {
			cylinder(r=pen_r+thickness, h=height);
			translate([(gap+lever_t)/2, pen_r*1.1+lever_l/2, lever_h/2])
				cube([lever_t, lever_l+thickness, lever_h], center=true);
			translate([0, pen_r+(tab_l+thickness)/2, height/2]) 
				cube([tab_t*2+gap, tab_l+thickness, height], center=true);
		}
		union() {
			cylinder(r=pen_r, h=height);
			translate([0, pen_r+0.5*(thickness+tab_l), height/2])
				cube([gap, 1.2*(thickness+tab_l), height], center=true);
		}
	}
}

module pen_holder2() {
        pen_r = 5.8;
        thickness = 2.5;
        height = 16;
        gap = 2;
        tab_length = 5;
        tab_depth= 5;
        tab_height = 5;
        tab_thickness = 2;
        tab_hole_r = 1;

        difference() {
                cylinder(r=pen_r+thickness, h=height);
                union() {
                        cylinder(r=pen_r, h=height);
                        translate([0, pen_r+0.5*(thickness), height/2])
                                cube([gap, 1.2*(thickness), height], center=true);
                }
        }
        
        translate([tab_thickness/2+gap/2, tab_depth/2+pen_r+thickness, tab_height/2])
                cube([tab_thickness, tab_depth*1.2, tab_height], center=true);

        translate([-(tab_length-gap)/2, tab_thickness/2+pen_r+thickness+tab_depth, tab_height/2])
                difference() {
                        translate([tab_thickness/2, 0, 0])
                                cube([tab_length+tab_thickness, tab_thickness, tab_height], center=true);
                        rotate([90, 0, 0])
                                cylinder(r=tab_hole_r, h=tab_thickness*1.05, center=true);
                }
}

module plate () {
	knob_dist = 72.7;

	motor_screw_dist = 35;
	motor_screw_offset = 8;
	motor_axle_hole = 9.5;
	motor_screw_r = 2.5;

	plate_d = 1.5;
	plate_h = motor_screw_dist+10;
	plate_l = knob_dist+2*15;

	frame_thickness = 4;
	
	difference() { 
		cube([plate_l, plate_h, plate_d], center=true);
		union() {
			for (i=[0:1]) {
				rotate([0, 0, i*180]) {
					translate([-knob_dist/2, 0, 0]) 
						cylinder(r=motor_axle_hole/2, h=plate_d, center=true);
					translate([-knob_dist/2-motor_screw_offset, motor_screw_dist/2, 0])
						cylinder(r=motor_screw_r, h=plate_d, center=true);
					translate([-knob_dist/2-motor_screw_offset, -motor_screw_dist/2, 0])
						cylinder(r=motor_screw_r, h=plate_d, center=true);
				}
			}
			cube([knob_dist-motor_axle_hole-2*frame_thickness, plate_h-2*frame_thickness, plate_d], center=true);
		}
	}
}	


module spool(radius, height, rim_dr, rim_h) {
	difference() {
		cylinder(r=radius+1*rim_dr+0*(height/2-rim_h), h=height);
		union() {
			axle();
			translate([0, 0, 2])	difference() {
				cylinder(r=0.8*radius, h=height-2);
				cylinder(r=0.2*radius, h=height-2);
			}
			translate([0, 0, height/2]) torus(radius+(height/2-rim_h), height/2-rim_h);
//			translate([0, 0, rim_h]) difference() {
//				cylinder(r=radius+rim_dr, h=height-2*rim_h);
//				cylinder(r=radius+(height/2-rim_h), h=height-2*rim_//h);
//			}
			for (i=[0:5]) {
				rotate([0, 0, i*360/6])
					translate([radius/1.85, 0, 0]) cylinder(r=radius/4.4, h=height);
			}
		}		
	}
}

module knob () {
	knob_r1 = 10.45;
	knob_r2 = 10.6;
	knob_t = 2;
	knob_h1 = 9;
	knob_h2 = 5;

	difference() { 
		cylinder(r=knob_r1+knob_t, h=knob_h1+knob_h2);
		union() { 
			axle();
			translate([0, 0, knob_h2]) cylinder(r1=knob_r1, r2=knob_r2, h=knob_h1);
		}
	}
}

module axle () {
	stepper_axle_r = 2.5;
	stepper_axle_d = 3;
	stepper_axle_l = 10;
	eps = 0.2;

	intersection() { 
		cylinder(r=stepper_axle_r+eps, h=stepper_axle_l);
		translate([0, 0, stepper_axle_l/2])
			cube([stepper_axle_d+2*eps, 2*(stepper_axle_r+eps), stepper_axle_l], center=true);
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
