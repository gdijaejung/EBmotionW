#pragma once


class cGBone
{
public:
	cGBone();
	virtual ~cGBone();

	bool Init(graphic::cRenderer &renderer);
	void Render(graphic::cRenderer &renderer, const Matrix44 &tm);


	graphic::cCube2 m_cube;
	graphic::cLine m_arrow[3];
	Matrix44 m_worldTm;
};

