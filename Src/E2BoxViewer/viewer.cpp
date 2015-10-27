
#include "stdafx.h"
#include "gbone.h"


#include <objidl.h>
#include <gdiplus.h> 
#pragma comment( lib, "gdiplus.lib" ) 
using namespace Gdiplus;
using namespace graphic;


class cViewer : public framework::cGameMain
{
public:
	cViewer();
	virtual ~cViewer();

	virtual bool OnInit() override;
	virtual void OnUpdate(const float elapseT) override;
	virtual void OnRender(const float elapseT) override;
	virtual void OnShutdown() override;
	virtual void MessageProc(UINT message, WPARAM wParam, LPARAM lParam) override;


private:
	network::cUDPServer m_udpServer;

	graphic::cCube2 m_cube[15];
	cGBone m_characterBone[15];
	cGBone m_sensorBone[15];

	Matrix44 m_characterInitTm[15];
	Matrix44 m_retargetTm[15]; // sensorTM -> characterTM 으로 변환하는 행렬


	Quaternion m_q[15];
	Matrix44 m_offset[15];
	Matrix44 m_heading[15];
	Matrix44 m_localTm[15];
	Matrix44 m_localTm2[15];

	Matrix44 m_firstLocalTm[15];
	Matrix44 m_worldTm[15];



	graphic::cSphere m_sphere;

	bool m_dbgPrint;
	bool m_localDisplay;

	string m_filePath;
	POINT m_curPos;
	bool m_LButtonDown;
	bool m_RButtonDown;
	bool m_MButtonDown;
	Matrix44 m_rotateTm;

	// GDI plus
	ULONG_PTR m_gdiplusToken;
	GdiplusStartupInput m_gdiplusStartupInput;
};

INIT_FRAMEWORK(cViewer);


cViewer::cViewer()
{
	m_windowName = L"E2Box Viewer";
	const RECT r = { 0, 0, 1024, 768 };
	m_windowRect = r;

	m_dbgPrint = false;
	m_localDisplay = true;

	m_LButtonDown = false;
	m_RButtonDown = false;
	m_MButtonDown = false;
}

cViewer::~cViewer()
{
	Gdiplus::GdiplusShutdown(m_gdiplusToken);
	graphic::ReleaseRenderer();
}


bool cViewer::OnInit()
{
	DragAcceptFiles(m_hWnd, TRUE);

	Gdiplus::GdiplusStartup(&m_gdiplusToken, &m_gdiplusStartupInput, NULL);

	m_renderer.GetDevice()->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);
	m_renderer.GetDevice()->LightEnable(0, true);

	for (int i = 0; i < 15; ++i)
	{
		m_cube[i].SetCube(m_renderer, Vector3(-1, -1, -1), Vector3(1, 1, 1));
		m_characterBone[i].Init(m_renderer);
		m_sensorBone[i].Init(m_renderer);
		m_sensorBone[i].m_cube.GetMaterial().InitBlack();
		m_retargetTm[i].SetIdentity();
	}

	m_characterInitTm[7].SetRotationXY(Vector3(1, 0, -1).Normal(), Vector3(0, 1, 0));
	m_characterInitTm[8].SetRotationXY(Vector3(1, 0, -1).Normal(), Vector3(0, 1, 0));
	m_characterInitTm[9].SetRotationXY(Vector3(1, 0, -1).Normal(), Vector3(0, 1, 0));




	const int WINSIZE_X = 1024;		//초기 윈도우 가로 크기
	const int WINSIZE_Y = 768;	//초기 윈도우 세로 크기
	GetMainCamera()->Init(&m_renderer);
	GetMainCamera()->SetCamera(Vector3(20, 20, -20), Vector3(0, 0, 0), Vector3(0, 1, 0));
	GetMainCamera()->SetProjection(D3DX_PI / 4.f, (float)WINSIZE_X / (float)WINSIZE_Y, 1.f, 10000.0f);

	const Vector3 lightPos(300, 300, -300);
	GetMainLight().SetPosition(lightPos);
	GetMainLight().SetDirection(lightPos.Normal());
	GetMainLight().Bind(m_renderer, 0);
	GetMainLight().Init(cLight::LIGHT_DIRECTIONAL,
		Vector4(0.7f, 0.7f, 0.7f, 0), Vector4(0.2f, 0.2f, 0.2f, 0));

	m_udpServer.Init(0, 8888);
	m_udpServer.m_sleepMillis = 0;

	for (int i = 0; i < 15; ++i)
	{
		m_offset[i].SetIdentity();
		m_heading[i].SetIdentity();
		m_localTm[i].SetIdentity();
	}

	return true;
}


void cViewer::OnUpdate(const float elapseT)
{
	char buff[512];
	const int len = m_udpServer.GetRecvData(buff, sizeof(buff));
	if (len > 0)
	{
		if (len < sizeof(buff))
			buff[len] = NULL;

		vector<string> toks;
		common::tokenizer(buff, ",", "", toks);

		if (toks.size() >= 6)
		{
			static int cnt = 0;
			if (m_dbgPrint)
				dbg::Print(common::format("{%d} %s", cnt++, buff));

			int nid = 0;
			const int i = toks[0].find('-');
			if (string::npos == i)
				return;

			const string id = toks[0].substr(i + 1);
			nid = atoi(id.c_str());
			if (nid < 0)
				return;

			const float x = (float)atof(toks[1].c_str());
			const float y = (float)atof(toks[2].c_str());
			const float z = (float)atof(toks[3].c_str());
			const float w = (float)atof(toks[4].c_str());
			Quaternion q(y, x, z, w);

			m_q[nid] = q;
			const Matrix44 qtm = q.GetMatrix();
			//m_localTm[nid] = m_offset[nid] * q.GetMatrix();
			m_sensorBone[nid].m_worldTm = qtm;

			if (nid == 0)
			{
// 				const Vector3 zAxis = Vector3(0, 1, 0).MultiplyNormal(qtm);
// 				const Vector3 xAxis = Vector3(1, 0, 0).MultiplyNormal(qtm);
// 				Matrix44 baseTm;
// 				baseTm.SetRotationXZ(xAxis, zAxis);
				//m_localTm2[nid] = baseTm;
				m_localTm[nid].SetIdentity();
			}
			else
			{
				m_worldTm[nid] = qtm * m_worldTm[0].Inverse();

				if (nid == 9)
				{
					Matrix44 local = m_worldTm[nid];
					m_localTm2[nid] = local;
				}
				else if (nid == 8)
				{
					Matrix44 local = m_worldTm[nid] * m_worldTm[9].Inverse();
					m_localTm2[nid] = m_localTm[nid].Inverse() * local;
				}
				else if (nid == 7)
				{
					Matrix44 local = m_worldTm[nid] * m_worldTm[8].Inverse();
					m_localTm2[nid] = m_localTm[nid].Inverse() * local;
				}
				if (nid == 4)
				{
					Matrix44 local = m_worldTm[nid];
					m_localTm2[nid] = local;
				}
				else if (nid == 6)
				{
					Matrix44 local = m_worldTm[nid] * m_worldTm[4].Inverse();
					m_localTm2[nid] = m_localTm[nid].Inverse() * local;
				}
				else if (nid == 14)
				{
					Matrix44 local = m_worldTm[nid] * m_worldTm[6].Inverse();
					m_localTm2[nid] = m_localTm[nid].Inverse() * local;
				}
				else
				{
					m_localTm2[nid] = m_worldTm[nid];
				}
			}

		}
	}
}


void cViewer::OnRender(const float elapseT)
{
	if (m_renderer.ClearScene())
	{
		m_renderer.BeginScene();

		GetMainLight().Bind(m_renderer, 0);

		m_renderer.RenderGrid();
		m_renderer.RenderAxis();

		Quaternion q;
		q.SetRotationArc(Vector3(1, 0, 0), Vector3(-1, 0, 0), Vector3(0, 1, 0));
		Matrix44 worldTm = q.GetMatrix();

		if (m_localDisplay)
		{
			//m_cube[7].Render(m_renderer, m_localTm[7]);
			
// 			Matrix44 t1;
// 			t1.SetPosition(Vector3(-5, 0, 0));
// 			m_cube[8].Render(m_renderer, m_localTm[8] * t1);
// 
// 			Matrix44 t2;
// 			t2.SetPosition(Vector3(-10, 0, 0));
// 			m_cube[9].Render(m_renderer, m_localTm[9] * t2);

			{
				Vector3 sensorPos(-10, 0, 0);
				Matrix44 t0;
				t0.SetPosition(sensorPos);
				m_sensorBone[0].Render(m_renderer, m_sensorBone[0].m_worldTm * t0);
				Matrix44 t1;
				t1.SetPosition(sensorPos + Vector3(-15, 0, 0));
				m_sensorBone[7].Render(m_renderer, m_sensorBone[7].m_worldTm * t1);
				Matrix44 t2;
				t2.SetPosition(sensorPos + Vector3(-10, 0, 0));
				m_sensorBone[8].Render(m_renderer, m_sensorBone[8].m_worldTm * t2);
				Matrix44 t3;
				t2.SetPosition(sensorPos + Vector3(-5, 0, 0));
				m_sensorBone[9].Render(m_renderer, m_sensorBone[9].m_worldTm * t2);
			}

			{
				Vector3 charPos(10, 0, 0);
				Matrix44 t0;
 				t0.SetPosition(charPos);
				m_characterBone[0].Render(m_renderer, m_worldTm[0] * t0);
				Matrix44 t1;
				t1.SetPosition(charPos + Vector3(-15, 0, 0));
				m_characterBone[7].Render(m_renderer, m_worldTm[7] * t1);
				Matrix44 t2;
				t2.SetPosition(charPos + Vector3(-10, 0, 0));
				m_characterBone[8].Render(m_renderer, m_worldTm[8] * t2);
				Matrix44 t3;
				t3.SetPosition(charPos + Vector3(-5, 0, 0));
				m_characterBone[9].Render(m_renderer, m_worldTm[9] * t3);
			}

		}
		else
		{
			{
				Matrix44 t0;
				t0.SetPosition(Vector3(-2, 0, 0));
				Matrix44 p1 = m_retargetTm[9] * m_worldTm[9];
				m_cube[9].Render(m_renderer, p1 * t0); 

 				Matrix44 t1;
 				t1.SetPosition(Vector3(-5, 0, 0));
				Matrix44 p2 = m_retargetTm[8] * m_worldTm[8];
				m_cube[8].Render(m_renderer, p2 * p1.Inverse() * t1 * p1);

				Matrix44 t2;
  				t2.SetPosition(Vector3(-5, 0, 0));
				Matrix44 p3 = m_retargetTm[7] * m_worldTm[7];
  				m_cube[7].Render(m_renderer, 
					p3 * p2.Inverse() * t2 *
					p2 * p1.Inverse() * t1 * p1);

			}



			{
				Matrix44 t0;
				t0.SetPosition(Vector3(2, 0, 0));
				Matrix44 p1 = m_retargetTm[4] * m_worldTm[4];
				m_cube[4].Render(m_renderer, p1 * t0);

				Matrix44 t1;
				t1.SetPosition(Vector3(5, 0, 0));
				Matrix44 p2 = m_retargetTm[6] * m_worldTm[6];
				m_cube[6].Render(m_renderer, p2 * p1.Inverse() * t1 * p1);

				Matrix44 t2;
				t2.SetPosition(Vector3(5, 0, 0));
				Matrix44 p3 = m_retargetTm[14] * m_worldTm[14];
				m_cube[14].Render(m_renderer,
					p3 * p2.Inverse() * t2 *
					p2 * p1.Inverse() * t1 * p1);

			}


		}	

		m_renderer.RenderFPS();

		m_renderer.EndScene();
		m_renderer.Present();
	}
}


void cViewer::OnShutdown()
{

}


void cViewer::MessageProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DROPFILES:
	{
		HDROP hdrop = (HDROP)wParam;
		char filePath[MAX_PATH];
		const UINT size = DragQueryFileA(hdrop, 0, filePath, MAX_PATH);
		if (size == 0)
			return;// handle error...

		m_filePath = filePath;

		const graphic::RESOURCE_TYPE::TYPE type = graphic::cResourceManager::Get()->GetFileKind(filePath);
		switch (type)
		{
		case graphic::RESOURCE_TYPE::MESH:
			break;

		case graphic::RESOURCE_TYPE::ANIMATION:
			break;
		}
	}
	break;

	case WM_MOUSEWHEEL:
	{
		int fwKeys = GET_KEYSTATE_WPARAM(wParam);
		int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		dbg::Print("%d %d", fwKeys, zDelta);

		const float len = graphic::GetMainCamera()->GetDistance();
		float zoomLen = (len > 100) ? 50 : (len / 4.f);
		if (fwKeys & 0x4)
			zoomLen = zoomLen / 10.f;

		graphic::GetMainCamera()->Zoom((zDelta < 0) ? -zoomLen : zoomLen);
	}
	break;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_F5: // Refresh
		{
			if (m_filePath.empty())
				return;
		}
		break;
		case VK_BACK:
			// 회전 행렬 초기화.
			m_dbgPrint = !m_dbgPrint;
			break;
		case VK_TAB:
		{
			static bool flag = false;
			m_renderer.GetDevice()->SetRenderState(D3DRS_CULLMODE, flag ? D3DCULL_CCW : D3DCULL_NONE);
			m_renderer.GetDevice()->SetRenderState(D3DRS_FILLMODE, flag ? D3DFILL_SOLID : D3DFILL_WIREFRAME);
			flag = !flag;
		}
		break;

		case VK_SPACE:
		{
			Matrix44 baseTm;

			const int indices[] = { 0, 9, 8, 7, 3, 4, 6, 14, 2, 1, 0, 5, 13, 12, 11, 10 };
			for (int k = 0; k < 15; ++k)
			{
				const int idx = indices[k];
				Quaternion q;
 				//q.SetRotationArc(Vector3(1, 0, 0), Vector3(-1, 0, 0), Vector3(0, 1, 0));
				Vector3 front = Vector3(1, 0, 0).MultiplyNormal(m_q[idx].GetMatrix());
				front.Normalize();
				q.SetRotationArc(front, Vector3(1, 0, 0), Vector3(0, 1, 0));

				Vector3 up = Vector3(0, 1, 0).MultiplyNormal(m_q[idx].GetMatrix());
				up.Normalize();
				Quaternion qUp;
				qUp.SetRotationArc(up, Vector3(0, 1, 0), Vector3(0, 0, 1));

// 				Matrix44 rt;
// 				rt = (q *qUp).GetMatrix();
				//rt.SetRotationXY(Vector3(1,0,0), 

				Matrix44 rt;
				rt.SetRotationXY(Vector3(1, 0, 0), Vector3(0, 1, 0));
				// m_q[i] * T = rt
				// inverse(m_q[i]) * rt = T
				 
				m_offset[idx] = m_q[idx].GetMatrix().Inverse();
				m_heading[idx] = m_q[idx].GetMatrix().Inverse() * rt;

				if (idx == 0)
				{
					const Matrix44 t = m_q[idx].GetMatrix();
					const Vector3 zAxis = Vector3(0,1,0).MultiplyNormal(t);
					Vector3 right = Vector3(0, 1, 0).CrossProduct(zAxis);
					right.Normalize();
					Vector3 front = right.CrossProduct(Vector3(0, 1, 0));
					front.Normalize();

//					const Vector3 xAxis = Vector3(0,0,1).MultiplyNormal(t);
					baseTm.SetRotationYZ(Vector3(0, 1, 0), front);

// 					Matrix44 basis;
// 					basis.SetRotationXY(Vector3(1, 0, 0), Vector3(0, 1, 0));
					
					m_worldTm[idx] = baseTm;
				}
				else
				{
 					m_worldTm[idx] = m_q[idx].GetMatrix() * baseTm.Inverse();

					Matrix44 basisTm;
					basisTm.SetRotationXY(Vector3(1, 0, 0), Vector3(0, 1, 0));
					m_retargetTm[idx] = m_worldTm[idx].Inverse();// *basisTm;

					if (idx == 9)
					{
						m_localTm[idx] = m_worldTm[idx] * m_worldTm[0].Inverse();
					}
					else if (idx == 8)
					{
						m_localTm[idx] = m_worldTm[idx] * m_worldTm[9].Inverse();
					}
					else if (idx == 7)
					{
						m_localTm[idx] = m_worldTm[idx] * m_worldTm[8].Inverse();
					}
					if (idx == 4)
					{
						m_localTm[idx] = m_worldTm[idx] * m_worldTm[0].Inverse();
					}
					else if (idx == 6)
					{
						m_localTm[idx] = m_worldTm[idx] * m_worldTm[4].Inverse();
					}
					else if (idx == 14)
					{
						m_localTm[idx] = m_worldTm[idx] * m_worldTm[6].Inverse();
					}
					else
					{
						m_localTm[idx] = m_worldTm[idx];
					}

				}

			}
		}
		break;

		case VK_RETURN:
			m_localDisplay = !m_localDisplay;
			break;

		}
		break;

	case WM_LBUTTONDOWN:
	{
		m_LButtonDown = true;
		m_curPos.x = LOWORD(lParam);
		m_curPos.y = HIWORD(lParam);
	}
	break;

	case WM_LBUTTONUP:
		m_LButtonDown = false;
		break;

	case WM_RBUTTONDOWN:
	{
		m_RButtonDown = true;
		m_curPos.x = LOWORD(lParam);
		m_curPos.y = HIWORD(lParam);
	}
	break;

	case WM_RBUTTONUP:
		m_RButtonDown = false;
		break;

	case WM_MBUTTONDOWN:
		m_MButtonDown = true;
		m_curPos.x = LOWORD(lParam);
		m_curPos.y = HIWORD(lParam);
		break;

	case WM_MBUTTONUP:
		m_MButtonDown = false;
		break;

	case WM_MOUSEMOVE:
	{
		if (m_LButtonDown)
		{
			POINT pos = { LOWORD(lParam), HIWORD(lParam) };
			const int x = pos.x - m_curPos.x;
			const int y = pos.y - m_curPos.y;
			m_curPos = pos;

			Quaternion q1(graphic::GetMainCamera()->GetRight(), -y * 0.01f);
			Quaternion q2(graphic::GetMainCamera()->GetUpVector(), -x * 0.01f);

			m_rotateTm *= (q2.GetMatrix() * q1.GetMatrix());
		}
		else if (m_RButtonDown)
		{
			POINT pos = { LOWORD(lParam), HIWORD(lParam) };
			const int x = pos.x - m_curPos.x;
			const int y = pos.y - m_curPos.y;
			m_curPos = pos;

			//if (GetAsyncKeyState('C'))
			{
				graphic::GetMainCamera()->Yaw2(x * 0.005f);
				graphic::GetMainCamera()->Pitch2(y * 0.005f);
			}
		}
		else if (m_MButtonDown)
		{
			const POINT point = { LOWORD(lParam), HIWORD(lParam) };
			const POINT pos = { point.x - m_curPos.x, point.y - m_curPos.y };
			m_curPos = point;

			const float len = graphic::GetMainCamera()->GetDistance();
			graphic::GetMainCamera()->MoveRight(-pos.x * len * 0.001f);
			graphic::GetMainCamera()->MoveUp(pos.y * len * 0.001f);
		}
		else
		{
		}

	}
	break;
	}
}

