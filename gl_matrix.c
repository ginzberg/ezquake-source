
#include "quakedef.h"
#include "gl_model.h"
#include "gl_local.h"

void GLM_OrthographicProjection(float left, float right, float top, float bottom, float zNear, float zFar);
void GLM_SetMatrix(float* target, const float* source);

static GLfloat projectionMatrix[16];
static GLfloat modelMatrix[16];
//static GLfloat viewMatrix[16];
static GLfloat identityMatrix[16] = {
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1
};

void GLM_SetIdentityMatrix(float* matrix)
{
	GLM_SetMatrix(matrix, identityMatrix);
}

// 
static const GLfloat* GL_OrthoMatrix(float left, float right, float top, float bottom, float zNear, float zFar)
{
	static GLfloat matrix[16];

	memset(matrix, 0, sizeof(matrix));
	matrix[0] = 2 / (right - left);
	matrix[5] = 2 / (top - bottom);
	matrix[10] = -2 / (zFar - zNear);
	matrix[12] = -(right + left) / (right - left);
	matrix[13] = -(top + bottom) / (top - bottom);
	matrix[14] = -(zFar + zNear) / (zFar - zNear);
	matrix[15] = 1;

	return matrix;
}

void GLM_OrthographicProjection(float left, float right, float top, float bottom, float zNear, float zFar)
{
	// Deliberately inverting top & bottom here...
	GLM_SetMatrix(projectionMatrix, GL_OrthoMatrix(left, right, bottom, top, zNear, zFar));
}

float* GL_MatrixForMode(GLenum type)
{
	static float junk[16] = { 0 };

	if (type == GL_PROJECTION || type == GL_PROJECTION_MATRIX) {
		return projectionMatrix;
	}
	else if (type == GL_MODELVIEW || type == GL_MODELVIEW_MATRIX) {
		return modelMatrix;
	}
	else {
		return junk;
	}
}

void GLM_GetMatrix(GLenum type, float* matrix)
{
	GLM_SetMatrix(matrix, GL_MatrixForMode(type));
}

float* GLM_ModelviewMatrix(void)
{
	return modelMatrix;
}

float* GLM_ProjectionMatrix(void)
{
	return projectionMatrix;
}

void GLM_SetMatrix(float* target, const float* source)
{
	memcpy(target, source, sizeof(float) * 16);
}

void GL_OrthographicProjection(float left, float right, float top, float bottom, float zNear, float zFar)
{
	if (GLM_Enabled()) {
		GLM_OrthographicProjection(left, right, top, bottom, zNear, zFar);
	}
	else {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(left, right, top, bottom, zNear, zFar);
	}
}

void GLM_RotateMatrix(float* matrix, float angle, float x, float y, float z)
{
	vec3_t vec = { x, y, z };
	double s = sin(angle * M_PI / 180);
	double c = cos(angle * M_PI / 180);
	float rotation[16];
	float result[16];

	VectorNormalize(vec);

	// Taken from https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glRotate.xml
	/*rotation[0] = x * x * (1 - c) + c;
	rotation[1] = x * y * (1 - c) - z * s;
	rotation[2] = x * z * (1 - c) + y * s;
	rotation[3] = 0;
	rotation[4] = y * x * (1 - c) + z * s;
	rotation[5] = y * y * (1 - c) + c;
	rotation[6] = y * z * (1 - c) - x * s;
	rotation[7] = 0;
	rotation[8] = x * z * (1 - c) - y * s;
	rotation[9] = y * z * (1 - c) + x * s;
	rotation[10] = z * z * (1 - c) + c;
	rotation[11] = 0;
	rotation[12] = rotation[13] = rotation[14] = 0;
	rotation[15] = 1;*/
	rotation[0] = x * x * (1 - c) + c;
	rotation[4] = x * y * (1 - c) - z * s;
	rotation[8] = x * z * (1 - c) + y * s;
	rotation[12] = 0;
	rotation[1] = y * x * (1 - c) + z * s;
	rotation[5] = y * y * (1 - c) + c;
	rotation[9] = y * z * (1 - c) - x * s;
	rotation[13] = 0;
	rotation[2] = x * z * (1 - c) - y * s;
	rotation[6] = y * z * (1 - c) + x * s;
	rotation[10] = z * z * (1 - c) + c;
	rotation[14] = 0;
	rotation[3] = rotation[7] = rotation[11] = 0;
	rotation[15] = 1;

	GLM_MultiplyMatrix(rotation, matrix, result);
	GLM_SetMatrix(matrix, result);
}

void GLM_TransformMatrix(float* matrix, float x, float y, float z)
{
	matrix[12] += matrix[0] * x + matrix[4] * y + matrix[8] * z;
	matrix[13] += matrix[1] * x + matrix[5] * y + matrix[9] * z;
	matrix[14] += matrix[2] * x + matrix[6] * y + matrix[10] * z;
	matrix[15] += matrix[3] * x + matrix[7] * y + matrix[11] * z;
}

void GLM_ScaleMatrix(float* matrix, float x_scale, float y_scale, float z_scale)
{
	matrix[0] *= x_scale;
	matrix[1] *= x_scale;
	matrix[2] *= x_scale;
	matrix[3] *= x_scale;
	matrix[4] *= y_scale;
	matrix[5] *= y_scale;
	matrix[6] *= y_scale;
	matrix[7] *= y_scale;
	matrix[8] *= z_scale;
	matrix[9] *= z_scale;
	matrix[10] *= z_scale;
	matrix[11] *= z_scale;
}

void GLM_MultiplyMatrix(const float* lhs, const float* rhs, float* target)
{
	target[0] = lhs[0] * rhs[0] + lhs[1] * rhs[4] + lhs[2] * rhs[8] + lhs[3] * rhs[12];
	target[1] = lhs[0] * rhs[1] + lhs[1] * rhs[5] + lhs[2] * rhs[9] + lhs[3] * rhs[13];
	target[2] = lhs[0] * rhs[2] + lhs[1] * rhs[6] + lhs[2] * rhs[10] + lhs[3] * rhs[14];
	target[3] = lhs[0] * rhs[3] + lhs[1] * rhs[7] + lhs[2] * rhs[11] + lhs[3] * rhs[15];

	target[4] = lhs[4] * rhs[0] + lhs[5] * rhs[4] + lhs[6] * rhs[8] + lhs[7] * rhs[12];
	target[5] = lhs[4] * rhs[1] + lhs[5] * rhs[5] + lhs[6] * rhs[9] + lhs[7] * rhs[13];
	target[6] = lhs[4] * rhs[2] + lhs[5] * rhs[6] + lhs[6] * rhs[10] + lhs[7] * rhs[14];
	target[7] = lhs[4] * rhs[3] + lhs[5] * rhs[7] + lhs[6] * rhs[11] + lhs[7] * rhs[15];

	target[8] = lhs[8] * rhs[0] + lhs[9] * rhs[4] + lhs[10] * rhs[8] + lhs[11] * rhs[12];
	target[9] = lhs[8] * rhs[1] + lhs[9] * rhs[5] + lhs[10] * rhs[9] + lhs[11] * rhs[13];
	target[10] = lhs[8] * rhs[2] + lhs[9] * rhs[6] + lhs[10] * rhs[10] + lhs[11] * rhs[14];
	target[11] = lhs[8] * rhs[3] + lhs[9] * rhs[7] + lhs[10] * rhs[11] + lhs[11] * rhs[15];

	target[12] = lhs[12] * rhs[0] + lhs[13] * rhs[4] + lhs[14] * rhs[8] + lhs[15] * rhs[12];
	target[13] = lhs[12] * rhs[1] + lhs[13] * rhs[5] + lhs[14] * rhs[9] + lhs[15] * rhs[13];
	target[14] = lhs[12] * rhs[2] + lhs[13] * rhs[6] + lhs[14] * rhs[10] + lhs[15] * rhs[14];
	target[15] = lhs[12] * rhs[3] + lhs[13] * rhs[7] + lhs[14] * rhs[11] + lhs[15] * rhs[15];
}

void GLM_MultiplyVector3f(const float* matrix, float x, float y, float z, float* result)
{
	const float vector[4] = { x, y, z, 1 };

	result[0] = matrix[0] * vector[0] + matrix[4] * vector[1] + matrix[8] * vector[2] + matrix[12] * vector[3];
	result[1] = matrix[1] * vector[0] + matrix[5] * vector[1] + matrix[9] * vector[2] + matrix[13] * vector[3];
	result[2] = matrix[2] * vector[0] + matrix[6] * vector[1] + matrix[10] * vector[2] + matrix[14] * vector[3];
	result[3] = matrix[3] * vector[0] + matrix[7] * vector[1] + matrix[11] * vector[2] + matrix[15] * vector[3];
}

void GLM_MultiplyVector(const float* matrix, const float* vector, float* result)
{
	result[0] = matrix[0] * vector[0] + matrix[4] * vector[1] + matrix[8] * vector[2] + matrix[12] * vector[3];
	result[1] = matrix[1] * vector[0] + matrix[5] * vector[1] + matrix[9] * vector[2] + matrix[13] * vector[3];
	result[2] = matrix[2] * vector[0] + matrix[6] * vector[1] + matrix[10] * vector[2] + matrix[14] * vector[3];
	result[3] = matrix[3] * vector[0] + matrix[7] * vector[1] + matrix[11] * vector[2] + matrix[15] * vector[3];
}

void GL_IdentityModelView(void)
{
	if (GLM_Enabled()) {
		GLM_SetIdentityMatrix(GLM_ModelviewMatrix());
	}
	else {
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}
}

void GL_GetMatrix(GLenum mode, GLfloat* matrix)
{
	if (GLM_Enabled()) {
		GLM_GetMatrix(mode, matrix);
	}
	else {
		glGetFloatv(mode, matrix);
	}
}

void GL_Rotate(GLenum matrix, float angle, float x, float y, float z)
{
	if (GL_ShadersSupported()) {
		GLM_RotateMatrix(GL_MatrixForMode(matrix), angle, x, y, z);
	}
	else {
		glMatrixMode(matrix);
		glRotatef(angle, x, y, z);
	}
}

void GL_Translate(GLenum matrix, float x, float y, float z)
{
	if (GL_ShadersSupported()) {
		GLM_TransformMatrix(GL_MatrixForMode(matrix), x, y, z);
	}
	else {
		glMatrixMode(matrix);
		glTranslatef(x, y, z);
	}
}

void GL_IdentityProjectionView(void)
{
	if (GL_ShadersSupported()) {
		GLM_SetIdentityMatrix(GLM_ProjectionMatrix());
	}
	else {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
	}
}

void GL_ProcessErrors(const char* message)
{
	GLenum error = glGetError();
	while (error != GL_NO_ERROR) {
		Con_Printf("%s> = %X\n", message, error);
		error = glGetError();
	}
}

void GL_PushMatrix(GLenum mode, float* matrix)
{
	if (GL_ShadersSupported()) {
		memcpy(matrix, GL_MatrixForMode(mode), sizeof(float) * 16);
	}
	else {
		glMatrixMode(mode);
		glPushMatrix();
	}
}

void GL_PopMatrix(GLenum mode, float* matrix)
{
	if (GL_ShadersSupported()) {
		memcpy(GL_MatrixForMode(mode), matrix, sizeof(float) * 16);
	}
	else {
		glMatrixMode(mode);
		glPopMatrix();
	}
}

void GL_Scale(GLenum matrix, float xScale, float yScale, float zScale)
{
	if (GL_ShadersSupported()) {
		GLM_ScaleMatrix(GL_MatrixForMode(matrix), xScale, yScale, zScale);
	}
	else {
		glScalef(xScale, yScale, zScale);
	}
}

void GL_Frustum(double left, double right, double bottom, double top, double zNear, double zFar)
{
	if (GL_ShadersSupported()) {
		float perspective[16] = { 0 };
		float projection[16];
		float new_projection[16];

		perspective[0] = (2 * zNear) / (right - left);
		perspective[8] = (right + left) / (right - left);
		perspective[5] = (2 * zNear) / (top - bottom);
		perspective[9] = (top + bottom) / (top - bottom);
		perspective[10] = -(zFar + zNear) / (zFar - zNear);
		perspective[11] = -1;
		perspective[14] = -2 * (zFar * zNear) / (zFar - zNear);

		GLM_GetMatrix(GL_PROJECTION, projection);
		GLM_MultiplyMatrix(perspective, projection, new_projection);
		GLM_SetMatrix(GL_MatrixForMode(GL_PROJECTION), new_projection);
	}
	else {
		glFrustum(left, right, bottom, top, zNear, zFar);
	}
}

void GLM_DebugMatrix(GLenum mode, const char* label)
{
	float matrix[16];
	int i;

	GL_GetMatrix(mode, matrix);

	Con_Printf("%s\n", label);
	for (i = 0; i < 4; ++i) {
		Con_Printf("  [%5.3f %5.3f %5.3f %5.3f]\n", matrix[i], matrix[i + 4], matrix[i + 8], matrix[i + 12]);
	}
}

void GLM_MultiplyMatrixVector(float* matrix, vec3_t vector, float* result)
{
	result[0] = matrix[0] * vector[0] + matrix[4] * vector[1] + matrix[8] * vector[2] + matrix[12] * vector[3];
	result[1] = matrix[1] * vector[0] + matrix[5] * vector[1] + matrix[9] * vector[2] + matrix[13] * vector[3];
	result[2] = matrix[2] * vector[0] + matrix[6] * vector[1] + matrix[10] * vector[2] + matrix[14] * vector[3];
	result[3] = matrix[3] * vector[0] + matrix[7] * vector[1] + matrix[11] * vector[2] + matrix[15] * vector[3];
}