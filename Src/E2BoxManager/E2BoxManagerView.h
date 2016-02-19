
// E2BoxManagerView.h : interface of the CE2BoxManagerView class
//

#pragma once


class CE2BoxManagerView : public CView, public iUpdateObserver
{
protected: // create from serialization only
	CE2BoxManagerView();
	DECLARE_DYNCREATE(CE2BoxManagerView)

// Attributes
public:
	CE2BoxManagerDoc* GetDocument() const;
	virtual bool Init() override;
	virtual void Update(const float deltaSeconds) override;
	void Render();


// Operations
public:
	bool m_isInitDx = false;
	bool m_isFixedFrame = true; // 고정 프레임모드시 true, (100 frame)
	bool m_isRenderCube = false;
	graphic::Box m_box;
	graphic::cSphere m_sphere;
	graphic::cCamera m_camera;
	graphic::cLight m_light;
	graphic::cRenderer m_renderer;

	Matrix44 m_cubeTm;
	Matrix44 m_rotateTm;
	bool m_LButtonDown = false;
	bool m_RButtonDown = false;
	bool m_MButtonDown = false;
	CPoint m_curPos;

	float m_IncSeconds = 0;


// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CE2BoxManagerView();

protected:

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
};

inline CE2BoxManagerDoc* CE2BoxManagerView::GetDocument() const
   { return reinterpret_cast<CE2BoxManagerDoc*>(m_pDocument); }

