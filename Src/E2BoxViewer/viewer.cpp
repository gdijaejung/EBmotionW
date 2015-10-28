
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

	Matrix44 m_retargetTm[15];

	Quaternion m_q[15];
	Matrix44 m_localTm[15];
	Matrix44 m_offsetLocalTm[15];
	Matrix44 m_worldTm[15];


	graphic::cSphere m_sphere;

	bool m_dbgPrint;
	bool m_displayType;

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
	m_displayType = true;

	m_LButtonDown = false;
	m_RButtonDown = false;
	m_MButtonDown = false;
}

cViewer::~cViewer()
{
	Gdiplus::GdiplusShutdown(m_gdiplusToken);
	graphic::ReleaseRenderer();
}


// -1 이면 부모 뼈대가 없다는 뜻.
int GetParentBone(const int id)
{
	switch (id)
	{
	case 0: return -1;
	case 1: return 0;
	case 2: return 1;
	case 3: return 0;
	case 4: return 3;
	case 5: return 4;
	case 6: return 0;
	case 7: return 6;
	case 8: return 7;
	case 9: return 1;
	case 10: return 9;
	case 11: return 10;
	case 12: return 1;
	case 13: return 12;
	case 14: return 13;
	default:
		break;
	}
	return -1;
}


bool cViewer::OnInit()
{
	DragAcceptFiles(m_hWnd, TRUE);

	Gdiplus::GdiplusStartup(&m_gdiplusToken, &m_gdiplusStartupInput, NULL);

	m_renderer.GetDevice()->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);
	m_renderer.GetDevice()->LightEnable(0, true);

	const int WINSIZE_X = 1024;		//초기 윈도우 가로 크기
	const int WINSIZE_Y = 768;	//초기 윈도우 세로 크기
	GetMainCamera()->Init(&m_renderer);
 	GetMainCamera()->SetCamera(Vector3(30, 30, -30), Vector3(0, 0, 0), Vector3(0, 1, 0));
	GetMainCamera()->SetProjection(D3DX_PI / 4.f, (float)WINSIZE_X / (float)WINSIZE_Y, 1.f, 10000.0f);

	const Vector3 lightPos(300, 300, -300);
	GetMainLight().SetPosition(lightPos);
	GetMainLight().SetDirection(lightPos.Normal());
	GetMainLight().Bind(m_renderer, 0);
	GetMainLight().Init(cLight::LIGHT_DIRECTIONAL,
		Vector4(0.7f, 0.7f, 0.7f, 0), Vector4(0.2f, 0.2f, 0.2f, 0));

	m_udpServer.Init(0, 8888);
	m_udpServer.m_sleepMillis = 0;


	//-----------------------------------------------------------------------------------
	// 본격적인 스켈레톤 초기화 작업

	for (int i = 0; i < 15; ++i)
	{
		m_cube[i].SetCube(m_renderer, Vector3(-1, -1, -1), Vector3(1, 1, 1));
		m_characterBone[i].Init(m_renderer);
		m_sensorBone[i].Init(m_renderer);
		m_sensorBone[i].m_cube.GetMaterial().InitBlack();
		m_retargetTm[i].SetIdentity();
		m_localTm[i].SetIdentity();
		m_offsetLocalTm[i].SetIdentity();
	}

	// 	m_characterInitTm[7].SetRotationXY(Vector3(1, 0, -1).Normal(), Vector3(0, 1, 0));
	// 	m_characterInitTm[8].SetRotationXY(Vector3(1, 0, -1).Normal(), Vector3(0, 1, 0));
	// 	m_characterInitTm[9].SetRotationXY(Vector3(1, 0, -1).Normal(), Vector3(0, 1, 0));


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

			const int i = toks[0].find('-');
			if (string::npos == i)
				return;

			const string idStr = toks[0].substr(i + 1);
			const int id = atoi(idStr.c_str());
			if (id < 0)
				return;

			const float x = (float)atof(toks[1].c_str());
			const float y = (float)atof(toks[2].c_str());
			const float z = (float)atof(toks[3].c_str());
			const float w = (float)atof(toks[4].c_str());
			const Quaternion q(y, x, z, w);
  
			m_q[id] = q;
			const Matrix44 qtm = q.GetMatrix();
			m_sensorBone[id].m_worldTm = qtm;

			const int pid = GetParentBone(id);

			if (pid < 0) // root
			{
				//m_localTm[id].SetIdentity();
//				m_worldTm[id] = qtm;

				// 0번 센서의 Y축이 바라보는 방향을 정면(zAxis)으로 한다.
				// Y축은 바닥에서 수평하게 만들고, 새 좌표계를 정의한다.
				const Vector3 zAxis = Vector3(0, 1, 0).MultiplyNormal(qtm);
				Vector3 right = Vector3(0, 1, 0).CrossProduct(zAxis);
				right.Normalize();
				Vector3 front = right.CrossProduct(Vector3(0, 1, 0));
				front.Normalize();
				Matrix44 baseTm;
				baseTm.SetRotationYZ(Vector3(0, 1, 0), front);

				m_worldTm[id] = baseTm;
				m_localTm[id] = baseTm;
			}
			else
			{
				m_worldTm[id] = qtm * m_worldTm[0].Inverse(); // Root의 상대 회전값을 저장한다.
				m_localTm[id] = m_worldTm[id] * m_worldTm[pid].Inverse();

// 				if (id >= 3 && id <= 8)
// 				{
// 					m_worldTm[id] = qtm * m_worldTm[0].Inverse(); // Root의 상대 회전값을 저장한다.
// 					m_worldTm[id] = m_worldTm[id].Inverse();
// 				}
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

		if (m_displayType)
		{
			for (int i = 0; i < 15; ++i)
			{
				Vector3 sensorPos(-5.f-((i%4)*5.f), 0, (i/4)*5.f - 5.f);
				Matrix44 t0;
				t0.SetPosition(sensorPos);
				m_sensorBone[i].Render(m_renderer, m_sensorBone[i].m_worldTm * t0);
			}

			for (int i = 0; i < 15; ++i)
			{
				Vector3 charPos(20.f - ((i % 4)*5.f), 0, (i / 4)*5.f - 5.f);
				Matrix44 t0;
 				t0.SetPosition(charPos);
				m_characterBone[i].Render(m_renderer, m_worldTm[i] * t0);
			}

		}
		else
 		{
			Matrix44 paletteTm[15];
			Matrix44 incrementPaletteTm[15];
			Vector3 mov[15] = {
				Vector3(0, 0, 0), //0
				Vector3(0, 5, 0), //1
				Vector3(0, 5, 0), //2

				Vector3(-2, -3, 0),//3
				Vector3(0, -5, 0),//4
				Vector3(0, -5, 0),//5

				Vector3(2, -3, 0),//6
				Vector3(0, -5, 0),//7
				Vector3(0, -5, 0),//8

				Vector3(-5, 0, 0),//9
				Vector3(-5, 0, 0),//10
				Vector3(-3, 0, 0),//11

				Vector3(5, 0, 0),//12
				Vector3(5, 0, 0),//13
				Vector3(3, 0, 0),//14
			};

			for (int i = 0; i < 2; ++i)
			{
				const int id = i;
				const int pid = GetParentBone(id);

				Matrix44 t0;
				t0.SetPosition(mov[id]);
				//Matrix44 tm = m_retargetTm[id] * m_worldTm[id];
				Matrix44 tm = m_offsetLocalTm[id].Inverse() * m_localTm[id];
				//Matrix44 tm = m_localTm[id];
				paletteTm[id] = tm;

 				Matrix44 finalTm;
				if (pid < 0)
				{
					//finalTm = m_worldTm[id] * t0;
					//tm = m_localTm[id];
					finalTm = tm * t0;
					incrementPaletteTm[id] = finalTm;
				}
				else
				{
					//finalTm = tm * paletteTm[pid].Inverse() * t0 * incrementPaletteTm[pid];
					finalTm = tm * t0 * incrementPaletteTm[pid];
					incrementPaletteTm[id] = finalTm;
				}

				m_characterBone[i].Render(m_renderer, finalTm);
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

			for (int k = 0; k < 15; ++k)
			{
				const int id = k;
				const int pid = GetParentBone(k);
				const Matrix44 qtm = m_q[id].GetMatrix();

				if (pid < 0) // root
				{
					// 0번 센서의 Y축이 바라보는 방향을 정면(zAxis)으로 한다.
					// Y축은 바닥에서 수평하게 만들고, 새 좌표계를 정의한다.
					const Vector3 zAxis = Vector3(0, 1, 0).MultiplyNormal(qtm);
					Vector3 right = Vector3(0, 1, 0).CrossProduct(zAxis);
					right.Normalize();
					Vector3 front = right.CrossProduct(Vector3(0, 1, 0));
					front.Normalize();
					baseTm.SetRotationYZ(Vector3(0, 1, 0), front);

					m_localTm[id] = baseTm;
					m_offsetLocalTm[id] = baseTm;
					m_worldTm[id] = baseTm;
					m_retargetTm[id] = baseTm.Inverse();
				}
				else
				{
					// Root의 상대 변환 값을 world로 지정한다.
					m_worldTm[id] = qtm * baseTm.Inverse();
					m_localTm[id] = m_worldTm[id] * m_worldTm[pid].Inverse();

					m_offsetLocalTm[id] = m_worldTm[id] * m_worldTm[pid].Inverse();
					m_retargetTm[id] = m_worldTm[id].Inverse();

					//m_localTm[id] = m_worldTm[id] * m_worldTm[pid].Inverse();
				}
			}
		}
		break;

		case VK_RETURN:
			m_displayType = !m_displayType;
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

