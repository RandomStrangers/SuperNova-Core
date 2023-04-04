#if 0
#include "ModelBuilder.h"
#include "ExtMath.h"

void BoxDesc_TexOrigin(BoxDesc* desc, Int32 x, Int32 y) {
	desc->TexX = x; desc->TexY = y;
}

void BoxDesc_SetBounds(BoxDesc* desc, Real32 x1, Real32 y1, Real32 z1, Real32 x2, Real32 y2, Real32 z2) {
	desc->X1 = x1 / 16.0f; desc->X2 = x2 / 16.0f;
	desc->Y1 = y1 / 16.0f; desc->Y2 = y2 / 16.0f;
	desc->Z1 = z1 / 16.0f; desc->Z2 = z2 / 16.0f;
}

void BoxDesc_Expand(BoxDesc* desc, Real32 amount) {
	amount /= 16.0f;
	desc->X1 -= amount; desc->X2 += amount;
	desc->Y1 -= amount; desc->Y2 += amount;
	desc->Z1 -= amount; desc->Z2 += amount;
}

void BoxDesc_Scale(BoxDesc* desc, Real32 scale) {
	desc->X1 *= scale; desc->X2 *= scale;
	desc->Y1 *= scale; desc->Y2 *= scale;
	desc->Z1 *= scale; desc->Z2 *= scale;
	desc->RotX *= scale; desc->RotY *= scale; desc->RotZ *= scale;
}

void BoxDesc_RotOrigin(BoxDesc* desc, Int8 x, Int8 y, Int8 z) {
	desc->RotX = (Real32)x / 16.0f;
	desc->RotY = (Real32)y / 16.0f;
	desc->RotZ = (Real32)z / 16.0f;
}

void BoxDesc_MirrorX(BoxDesc* desc) {
	Real32 temp = desc->X1; desc->X1 = desc->X2; desc->X2 = temp;
}


#define BoxDesc_InitBox(desc)\
BoxDesc_TexOrigin(desc, 0, 0);\
BoxDesc_RotOrigin(desc, 0, 0, 0);\
BoxDesc_SetBounds(desc, (Real32)x1, (Real32)y1, (Real32)z1, (Real32)x2, (Real32)y2, (Real32)z2);\

void BoxDesc_Box(BoxDesc* desc, Int32 x1, Int32 y1, Int32 z1, Int32 x2, Int32 y2, Int32 z2) {
	BoxDesc_InitBox(desc)
	desc->SidesW = Math_AbsI(z2 - z1);
	desc->BodyW = Math_AbsI(x2 - x1);
	desc->BodyH = Math_AbsI(y2 - y1);
}

void BoxDesc_RotatedBox(BoxDesc* desc, Int32 x1, Int32 y1, Int32 z1, Int32 x2, Int32 y2, Int32 z2) {
	BoxDesc_InitBox(desc)
	desc->SidesW = Math_AbsI(y2 - y1);
	desc->BodyW = Math_AbsI(x2 - x1);
	desc->BodyH = Math_AbsI(z2 - z1);
}


ModelPart BoxDesc_BuildBox(IModel* m, BoxDesc* desc) {
	Int32 sidesW = desc->SidesW, bodyW = desc->BodyW, bodyH = desc->BodyH;
	Real32 x1 = desc->X1, y1 = desc->Y1, z1 = desc->Z1;
	Real32 x2 = desc->X2, y2 = desc->Y2, z2 = desc->Z2;
	Int32 x = desc->TexX, y = desc->TexY;

	BoxDesc_YQuad(m, x + sidesW, y, bodyW, sidesW, x2, x1, z2, z1, y2, false); /* top */
	BoxDesc_YQuad(m, x + sidesW + bodyW, y, bodyW, sidesW, x2, x1, z2, z1, y1, false); /* bottom */
	BoxDesc_ZQuad(m, x + sidesW, y + sidesW, bodyW, bodyH, x2, x1, y1, y2, z1, false); /* front */
	BoxDesc_ZQuad(m, x + sidesW + bodyW + sidesW, y + sidesW, bodyW, bodyH, x1, x2, y1, y2, z2, false); /* back */
	BoxDesc_XQuad(m, x, y + sidesW, sidesW, bodyH, z2, z1, y1, y2, x2, false); /* left */
	BoxDesc_XQuad(m, x + sidesW + bodyW, y + sidesW, sidesW, bodyH, z1, z2, y1, y2, x1, false); /* right */

	ModelPart part;
	ModelPart_Init(&part, m->index - IModel_BoxVertices, IModel_BoxVertices,
		desc->RotX, desc->RotY, desc->RotZ);
	return part;
}

ModelPart BoxDesc_BuildRotatedBox(IModel* m, BoxDesc* desc) {
	Int32 sidesW = desc->SidesW, bodyW = desc->BodyW, bodyH = desc->BodyH;
	Real32 x1 = desc->X1, y1 = desc->Y1, z1 = desc->Z1;
	Real32 x2 = desc->X2, y2 = desc->Y2, z2 = desc->Z2;
	Int32 x = desc->TexX, y = desc->TexY;

	BoxDesc_YQuad(m, x + sidesW + bodyW + sidesW, y + sidesW, bodyW, bodyH, x1, x2, z1, z2, y2, false); /* top */
	BoxDesc_YQuad(m, x + sidesW, y + sidesW, bodyW, bodyH, x2, x1, z1, z2, y1, false); /* bottom */
	BoxDesc_ZQuad(m, x + sidesW, y, bodyW, sidesW, x2, x1, y1, y2, z1, false); /* front */
	BoxDesc_ZQuad(m, x + sidesW + bodyW, y, bodyW, sidesW, x1, x2, y2, y1, z2, false); /* back */
	BoxDesc_XQuad(m, x, y + sidesW, sidesW, bodyH, y2, y1, z2, z1, x2, false); /* left */
	BoxDesc_XQuad(m, x + sidesW + bodyW, y + sidesW, sidesW, bodyH, y1, y2, z2, z1, x1, false); /* right */

	/* rotate left and right 90 degrees	*/
	Int32 i;
	for (i = m->index - 8; i < m->index; i++) {
		ModelVertex vertex = m->vertices[i];
		Real32 z = vertex.Z; vertex.Z = vertex.Y; vertex.Y = z;
		m->vertices[i] = vertex;
	}

	ModelPart part;
	ModelPart_Init(&part, m->index - IModel_BoxVertices, IModel_BoxVertices,
		desc->RotX, desc->RotY, desc->RotZ);
	return part;
}


#define UV_MAX IModel_UVMaxBit
void BoxDesc_XQuad(IModel* m, Int32 texX, Int32 texY, Int32 texWidth, Int32 texHeight,
	Real32 z1, Real32 z2, Real32 y1, Real32 y2, Real32 x, bool swapU) {
	Int32 u1 = texX, u2 = texX + texWidth | UV_MAX;
	if (swapU) { Int32 tmp = u1; u1 = u2; u2 = tmp; }

	ModelVertex_Init(&m->vertices[m->index], x, y1, z1, u1, texY + texHeight | UV_MAX); m->index++;
	ModelVertex_Init(&m->vertices[m->index], x, y2, z1, u1, texY); m->index++;
	ModelVertex_Init(&m->vertices[m->index], x, y2, z2, u2, texY); m->index++;
	ModelVertex_Init(&m->vertices[m->index], x, y1, z2, u2, texY + texHeight | UV_MAX); m->index++;
}

void BoxDesc_YQuad(IModel* m, Int32 texX, Int32 texY, Int32 texWidth, Int32 texHeight,
	Real32 x1, Real32 x2, Real32 z1, Real32 z2, Real32 y, bool swapU) {
	Int32 u1 = texX, u2 = texX + texWidth | UV_MAX;
	if (swapU) { Int32 tmp = u1; u1 = u2; u2 = tmp; }

	ModelVertex_Init(&m->vertices[m->index], x1, y, z2, u1, texY + texHeight | UV_MAX); m->index++;
	ModelVertex_Init(&m->vertices[m->index], x1, y, z1, u1, texY); m->index++;
	ModelVertex_Init(&m->vertices[m->index], x2, y, z1, u2, texY); m->index++;
	ModelVertex_Init(&m->vertices[m->index], x2, y, z2, u2, texY + texHeight | UV_MAX); m->index++;
}

void BoxDesc_ZQuad(IModel* m, Int32 texX, Int32 texY, Int32 texWidth, Int32 texHeight,
	Real32 x1, Real32 x2, Real32 y1, Real32 y2, Real32 z, bool swapU) {
	Int32 u1 = texX, u2 = texX + texWidth | UV_MAX;
	if (swapU) { Int32 tmp = u1; u1 = u2; u2 = tmp; }

	ModelVertex_Init(&m->vertices[m->index], x1, y1, z, u1, texY + texHeight | UV_MAX); m->index++;
	ModelVertex_Init(&m->vertices[m->index], x1, y2, z, u1, texY); m->index++;
	ModelVertex_Init(&m->vertices[m->index], x2, y2, z, u2, texY); m->index++;
	ModelVertex_Init(&m->vertices[m->index], x2, y1, z, u2, texY + texHeight | UV_MAX); m->index++;
}
#endif