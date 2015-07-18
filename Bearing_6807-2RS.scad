// Values from http://www.astbearings.com/product.html?product=6807-2RS
bearing6807_2RS_rs = 1.2;	// lip width
bearing6807_2RS_d = 35;		// ID
bearing6807_2RS_D = 47;		// OD
bearing6807_2RS_B = 7;		// width

module bearing6807_2RS() {
    rs = bearing6807_2RS_rs;
    d = bearing6807_2RS_d;
    D = bearing6807_2RS_D;
    B = bearing6807_2RS_B;
	//render(convexity = 3)
    translate([0, 0, B / 2])
	union() {
        // outer lip
        difference() {
            cylinder(h = B, d = D, center = true);
            cylinder(h = B, d = D - rs, center = true);
        }
        // inner lip
        difference() {
            cylinder(h = B, d = d + rs, center = true);
            cylinder(h = B, d = d, center = true);
        }
        // race
            difference() {
                cylinder(h = B - (rs * 2), d = D, center = true);
                cylinder(h = B - (rs * 2), d = d, center = true);
            };
    }
}
