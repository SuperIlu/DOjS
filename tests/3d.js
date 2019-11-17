vertex =
	[
		[-10., -10., -10., 0., 0., 72],
		[-10., 10., -10., 0., 0., 80],
		[10., 10., -10., 0., 0., 95],
		[10., -10., -10., 0., 0., 88],
		[-10., -10., 10., 0., 0., 72],
		[-10., 10., 10., 0., 0., 80],
		[10., 10., 10., 0., 0., 95],
		[10., -10., 10., 0., 0., 88]
	];


cube =
	[
		[2, 1, 0, 3],
		[4, 5, 6, 7],
		[0, 1, 5, 4],
		[2, 3, 7, 6],
		[4, 7, 3, 0],
		[1, 2, 6, 5]
	];


function draw_cube(matrix, num_poly) {
	int i, j, nv;
	int out[12];

	for (var i = 0; i < num_poly; i++) {
		for (var j = 0; j < 4; j++) {
			v[j] = vertex[cube[i][j]];
			ApplyMatrix(matrix, v[j].x, v[j].y, v[j].z,
			& v[j].x, & v[j].y, & v[j].z);
		}

		/* nv: number of vertices after clipping is done */
		nv = clip3d_f(POLYTYPE_GCOL, 0.1, 1000., 4, (AL_CONST V3D_f **)pv, pvout,
			pvtmp, out);

		if (nv) {
			for (j = 0; j < nv; j++)
				persp_project_f(vout[j].x, vout[j].y, vout[j].z, & vout[j].x,
					& vout[j].y);

			if (polygon_z_normal_f(& vout[0], & vout[1], & vout[2]) > 0)
				scene_polygon3d_f(POLYTYPE_GCOL, NULL, nv, pvout);
		}
	}
}
