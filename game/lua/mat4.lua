local ffi = require("ffi")

ffi.cdef
[[
	typedef struct
	{
		float r1c1;
		float r2c1;
		float r3c1;
		float r4c1;
		float r1c2;
		float r2c2;
		float r3c2;
		float r4c2;
		float r1c3;
		float r2c3;
		float r3c3;
		float r4c3;
		float r1c4;
		float r2c4;
		float r3c4;
		float r4c4;
	} Mat4;

	Mat4*				mat4(float r1c1, float r2c1, float r3c1, float r1c2, float r2c2, float r3c2, float r1c3, float r2c3, float r3c3);
						
	Mat4*				mat4_add(Mat4* self, Mat4* m);

	Mat4*				mat4_subtract(Mat4* self, Mat4* m);

	Mat4*				mat4_multiply(Mat4* self, Mat4* m);

	Mat4*				mat4_multiply_by_scalar(Mat4* self, float k);

	Mat4*				mat4_divide_by_scalar(Mat4* self, float k);

	void				mat4_build_rotation_x(Mat4* self, float radians);

	void				mat4_build_rotation_y(Mat4* self, float radians);	

	void				mat4_build_rotation_z(Mat4* self, float radians);

	void				mat4_build_rotation(Mat4* self, const Vec3* n, float radians);

	void				mat4_build_projection_perspective_rh(Mat4* self, float fovy, float aspect, float near, float far);

	void				mat4_build_projection_perspective_lh(Mat4* self, float fovy, float aspect, float near, float far);

	void				mat4_build_projection_ortho_rh(Mat4* self, float width, float height, float near, float far);

	void				mat4_build_projection_ortho_lh(Mat4* self, float width, float height, float near, float far);

	void				mat4_build_projection_ortho_2d_rh(Mat4* self, float width, float height, float near, float far);

	void				mat4_build_look_at_rh(Mat4* self, const Vec3* pos, const Vec3* target, const Vec3* up);

	void				mat4_build_look_at_lh(Mat4* self, const Vec3* pos, const Vec3* target, const Vec3* up);

	void				mat4_build_viewpoint_billboard(Mat4* self, const Vec3* pos, const Vec3* target, const Vec3* up);

	void				mat4_build_axis_billboard(Mat4* self, const Vec3* pos, const Vec3* target, const Vec3* axis);

	Mat4*				mat4_transpose(Mat4* self);

	float				mat4_determinant(Mat4* self);

	Mat4*				mat4_invert(Mat4* self);

	void				mat4_load_identity(Mat4* self);

	Vec3*				mat4_get_translation(Mat4* self);

	void				mat4_set_translation(Mat4* self, const Vec3* trans);

	Vec3*				mat4_get_scale(Mat4* self);

	void				mat4_set_scale(Mat4* self, const Vec3* scale);

	void 				mat4_print(Mat4* self);

]]