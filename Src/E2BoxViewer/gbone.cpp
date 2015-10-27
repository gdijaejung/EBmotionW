
#include "stdafx.h"
#include "gbone.h"



cGBone::cGBone()
{

}

cGBone::~cGBone()
{

}


bool cGBone::Init(graphic::cRenderer &renderer)
{
	m_cube.SetCube(renderer, Vector3(-1, -1, -1), Vector3(1, 1, 1));

	m_arrow[0].GetMaterial().InitRed();
	m_arrow[1].GetMaterial().InitGreen();
	m_arrow[2].GetMaterial().InitBlue();

	return true;
}


void cGBone::Render(graphic::cRenderer &renderer, const Matrix44 &tm)
{
	m_cube.Render(renderer, tm);

	const float len = 2.5f;
	Vector3 axis[3] = { Vector3(len, 0, 0), Vector3(0, len, 0), Vector3(0, 0, len) };
	for (int i = 0; i < 3; ++i)
	{
		const Vector3 p1 = Vector3(0, 0, 0) * tm;
		const Vector3 p2 = axis[i] * tm;
		m_arrow[i].SetLine(renderer, p1, p2, 0.1f);
		m_arrow[i].Render(renderer);
	}


}

