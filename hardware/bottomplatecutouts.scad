draw_switch_plates = false;
projection(cut=false)
{
difference()
{
  cube([90, 42, 1], center = true);
  translate([-2, 0,0]) cube([25.5, 31, 1], center = true);
  translate([-30, 0,0]) cube([20, 27,1], center = true);
  translate([28, 0,0]) cube([25.5, 31, 1], center = true);
}
}

if(draw_switch_plates)
{
  translate([-30, 0,2]) color("blue", 1.0) cube([24, 31.1,1], center = true);
  translate([-2, 0,2]) color("red", 1.0) cube([27.8, 35.2,1], center=true);
  translate([28, 0,2]) color("green", 1.0) cube([27.8, 35.2,1], center=true);
}