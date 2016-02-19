// E2BoxManagerView.cpp : implementation of the CE2BoxManagerView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "E2BoxManager.h"
#endif

#include "E2BoxManagerDoc.h"
#include "E2BoxManagerView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace graphic;


// CE2BoxManagerView

IMPLEMENT_DYNCREATE(CE2BoxManagerView, CView)

BEGIN_MESSAGE_MAP(CE2BoxManagerView, CView)
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CE2BoxManagerView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

// CE2BoxManagerView construction/destruction

CE2BoxManagerView::CE2BoxManagerView()
{

}

CE2BoxManagerView::~CE2BoxManagerView()
{
}

BOOL CE2BoxManagerView::PreCreateWindow(CREATESTRUCT& cs)
{
	cController2::Get()->AddUpdateObserver(this);
	return CView::PreCreateWindow(cs);
}

// CE2BoxManagerView drawing

void CE2BoxManagerView::OnDraw(CDC* /*pDC*/)
{
	CE2BoxManagerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;
}


// CE2BoxManagerView printing


void CE2BoxManagerView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CE2BoxManagerView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CE2BoxManagerView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

void CE2BoxManagerView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

void CE2BoxManagerView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CE2BoxManagerView message handlers
bool CE2BoxManagerView::Init()
{
	const int WINSIZE_X = 800;		//초기 윈도우 가로 크기
	const int WINSIZE_Y = 600;	//초기 윈도우 세로 크기

	// Create DirectX
	if (!m_renderer.CreateDirectX(GetSafeHwnd(), WINSIZE_X, WINSIZE_Y))
	{
		::AfxMessageBox(L"DirectX Initialize Error!!");
		return 0;
	}

	m_camera.Init(&m_renderer);
	m_camera.SetCamera(Vector3(12, 20, -10), Vector3(0, 0, 0), Vector3(0, 1, 0));
	m_camera.SetProjection(D3DX_PI / 4.f, (float)WINSIZE_X / (float)WINSIZE_Y, 1.f, 1000.0f);
	m_camera.SetCamera(Vector3(-11.8f, 4.2f, -5.3f), Vector3(0, 0, 0), Vector3(0, 1, 0));

	m_renderer.GetDevice()->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);

	m_renderer.GetDevice()->LightEnable(0, true);

	// 주 광원 초기화.
	const Vector3 lightPos(300, 300, -300);
	m_light.SetPosition(lightPos);
	m_light.SetDirection(-lightPos.Normal());
	m_light.Bind(m_renderer, 0);
	m_light.Init(cLight::LIGHT_DIRECTIONAL,
		Vector4(0.7f, 0.7f, 0.7f, 0), Vector4(0.2f, 0.2f, 0.2f, 0));

	m_isInitDx = true;

	//m_box.SetBox(Vector3(0, 0, 0), Vector3(2, 2, 2));
	m_sphere.Create(m_renderer, 10, 10, 10);
	m_sphere.GetMaterial().InitBlue();
	m_sphere.GetMaterial().GetMtrl().Emissive = *(D3DCOLORVALUE*)&Vector4(1, 1, 0, 1);

	//m_cube.SetCube(Vector3(0, 0, 0), Vector3(1, 1, 1));

	return true;
}


void CE2BoxManagerView::Update(const float deltaSeconds)
{
	RET(!m_isInitDx);

//	m_IncSeconds += deltaSeconds;

// 	RET(m_isFixedFrame && (m_IncSeconds < 0.01f)); // 시간 간격이 짧으면 실행하지 않는다.
// 	if (m_IncSeconds > 0.02f) // 너무 값이 크면, 최소 값으로 설정한다.
// 		m_IncSeconds = 0.02f;

	m_renderer.Update(deltaSeconds);

	// 출력.
	Render();


	m_IncSeconds = 0;
}


void CE2BoxManagerView::Render()
{
	RET(!m_isInitDx);

	//화면 청소
	if (m_renderer.ClearScene())
	{
		m_renderer.BeginScene();

		m_light.Bind(m_renderer, 0);

		// 백그라운드 그리드, 축 출력.
		m_renderer.RenderGrid();
		m_renderer.RenderAxis();

		m_renderer.RenderFPS();

		m_renderer.EndScene();
		m_renderer.Present();
	}
}


BOOL CE2BoxManagerView::OnEraseBkgnd(CDC* pDC)
{
//	return __super::OnEraseBkgnd(pDC);
	return TRUE;
}


void CE2BoxManagerView::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetCapture();
	SetFocus();
	m_LButtonDown = true;
	m_curPos = point;
	__super::OnLButtonDown(nFlags, point);
}


void CE2BoxManagerView::OnLButtonUp(UINT nFlags, CPoint point)
{
	ReleaseCapture();
	m_LButtonDown = false;
	__super::OnLButtonUp(nFlags, point);
}


void CE2BoxManagerView::OnRButtonDown(UINT nFlags, CPoint point)
{
	SetFocus();
	SetCapture();
	m_RButtonDown = true;
	m_curPos = point;
	__super::OnRButtonDown(nFlags, point);
}

void CE2BoxManagerView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
// 	ClientToScreen(&point);
// 	OnContextMenu(this, point);
	ReleaseCapture();
	m_RButtonDown = false;
}

void CE2BoxManagerView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_LButtonDown)
	{
		const CPoint pos = point - m_curPos;
		m_curPos = point;
		Quaternion q1(m_camera.GetRight(), -pos.y * 0.01f);
		Quaternion q2(m_camera.GetUpVector(), -pos.x * 0.01f);
		m_rotateTm *= (q2.GetMatrix() * q1.GetMatrix());
	}
	else if (m_RButtonDown)
	{
		const CPoint pos = point - m_curPos;
		m_curPos = point;

		m_camera.Yaw2(pos.x * 0.005f);
		m_camera.Pitch2(pos.y * 0.005f);
	}

	__super::OnMouseMove(nFlags, point);
}


BOOL CE2BoxManagerView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	const float len = m_camera.GetDistance();
	float zoomLen = (len > 100) ? 50 : (len / 4.f);
	if (nFlags & 0x4)
		zoomLen = zoomLen / 10.f;

	m_camera.Zoom((zDelta < 0) ? -zoomLen : zoomLen);
	return __super::OnMouseWheel(nFlags, zDelta, pt);
}
