#include "stdafx.h"
#include "evolve.h"
#include "ViewOrganismDialog.h"


// ViewOrganismDialog dialog

IMPLEMENT_DYNAMIC(ViewOrganismDialog, CDialog)

ViewOrganismDialog::ViewOrganismDialog(CWnd* pParent /*=NULL*/)
	: CDialog(ViewOrganismDialog::IDD, pParent)
{
	SimulateSteps = 0;

	win_rect.left	= 273 + 138;
	win_rect.top	= 180 + 168;
	win_rect.right	= win_rect.left + 318;
	win_rect.bottom	= win_rect.top + 182;

	tf = TF_Make();

	kfd = NULL;
	m_breakpoints = NULL;
	organism = NULL;
	cell = NULL;
	disassembly_updated = FALSE;
}

ViewOrganismDialog::~ViewOrganismDialog()
{
	if( tf != NULL ) {
		TF_Delete(tf);
	}

	if( kfd != NULL ) {
		kforth_disassembly_delete(kfd);
		kfd = NULL;
	}

	if( m_breakpoints != NULL ) {
		BPLIST *curr, *nxt;
		for(curr=m_breakpoints; curr; curr=nxt) {
			nxt = curr->next;
			delete curr;
		}
		m_breakpoints = NULL;
	}
}

void ViewOrganismDialog::DDX_Cell(CDataExchange* pDX)
{
	char buf[1000];
	CString str;
	CELL *curr;
	int idx;

	DDX_machine_registers(pDX);
	DDX_call_stack(pDX);
	DDX_data_stack(pDX);

	if( cursor_start >= 0 ) {
		CEdit *pED = (CEdit*) GetDlgItem(IDC_VO_INSTRUCTIONS);
		nice_select(cursor_start, cursor_end);
	}

	DDX_CommaValue(pDX, IDC_VO_MOOD, cell->mood);
	DDX_CommaValue(pDX, IDC_VO_MESSAGE, cell->message);

	sprintf(buf, "(%d, %d)", cell->x, cell->y);
	str = buf;
	DDX_Text(pDX, IDC_VO_LOCATION, str);

	idx = 0;
	for(curr=organism->cells; curr; curr=curr->next) {
		if( curr == cell ) {
			sprintf(buf, "(%d of %d)", idx+1, organism->ncells);
			str = buf;
			DDX_Text(pDX, IDC_VO_CELLIDX, str);
		}
		idx++;
	}
}

void ViewOrganismDialog::DDX_machine_registers(CDataExchange* pDX)
{
	KFORTH_MACHINE *kfm;
	char buf[1000];
	CString str;

	kfm = cell->kfm;

	sprintf(buf, "%I64d", kfm->R[0]);
	str = buf;
	DDX_Text(pDX, IDC_VO_R0, str);

	sprintf(buf, "%I64d", kfm->R[1]);
	str = buf;
	DDX_Text(pDX, IDC_VO_R1, str);

	sprintf(buf, "%I64d", kfm->R[2]);
	str = buf;
	DDX_Text(pDX, IDC_VO_R2, str);

	sprintf(buf, "%I64d", kfm->R[3]);
	str = buf;
	DDX_Text(pDX, IDC_VO_R3, str);

	sprintf(buf, "%I64d", kfm->R[4]);
	str = buf;
	DDX_Text(pDX, IDC_VO_R4, str);

	sprintf(buf, "%I64d", kfm->R[5]);
	str = buf;
	DDX_Text(pDX, IDC_VO_R5, str);

	sprintf(buf, "%I64d", kfm->R[6]);
	str = buf;
	DDX_Text(pDX, IDC_VO_R6, str);

	sprintf(buf, "%I64d", kfm->R[7]);
	str = buf;
	DDX_Text(pDX, IDC_VO_R7, str);

	sprintf(buf, "%I64d", kfm->R[8]);
	str = buf;
	DDX_Text(pDX, IDC_VO_R8, str);

	sprintf(buf, "%I64d", kfm->R[9]);
	str = buf;
	DDX_Text(pDX, IDC_VO_R9, str);

	if( kfm->terminated ) {
		str = "TERMINATED";
		DDX_Text(pDX, IDC_VO_PC, str);
		DDX_Text(pDX, IDC_VO_CB, str);
	} else {
		sprintf(buf, "%d", kfm->pc);
		str = buf;
		DDX_Text(pDX, IDC_VO_PC, str);

		sprintf(buf, "%d", kfm->cb);
		str = buf;
		DDX_Text(pDX, IDC_VO_CB, str);
	}
}

void ViewOrganismDialog::DDX_data_stack(CDataExchange* pDX)
{
	KFORTH_MACHINE *kfm;
	KFORTH_STACK_NODE *curr;
	int i;
	CString str;
	char buf[1000];

	kfm = cell->kfm;

	CListBox *pLB = (CListBox*) GetDlgItem(IDC_VO_DATA_STACK);

	pLB->ResetContent();

	i = 0;
	for(curr=kfm->data_sp; curr; curr=curr->prev) {
		sprintf(buf, "%I64d", curr->u.data.value);

		str = buf;

		pLB->InsertString(i, str);

		i++;
	}
}

void ViewOrganismDialog::DDX_call_stack(CDataExchange* pDX)
{
	KFORTH_MACHINE *kfm;
	KFORTH_STACK_NODE *curr;
	int i;
	CString str;
	char buf[1000];

	kfm = cell->kfm;

	CListBox *pLB = (CListBox*) GetDlgItem(IDC_VO_CALL_STACK);

	pLB->ResetContent();

	i = 0;
	for(curr=kfm->call_sp; curr; curr=curr->prev) {
		sprintf(buf, "cb=%d pc=%d",
			 curr->u.call.cb,
			 curr->u.call.pc);

		str = buf;

		pLB->InsertString(i, str);

		i++;
	}
}

void ViewOrganismDialog::draw_cell(CDC* pDC, CELL *cell, bool active, int idx, TF_RECT& ugwin)
{
	CRect rect;

	rect.left	= (int) ugwin.left;
	rect.top	= (int) ugwin.top;
	rect.right	= (int) ugwin.right;
	rect.bottom	= (int) ugwin.bottom;

	if( cell->kfm->terminated ) {
		pDC->SelectObject(&m_appearance.deadcellBrush);
		if( cell->organism->oflags & ORGANISM_FLAG_RADIOACTIVE ) {
			pDC->SelectObject( &m_appearance.dyePen );
		} else {
			pDC->SelectObject(&m_appearance.deadcellPen);
		}
	} else {
		pDC->SelectObject( m_appearance.cellBrush(cell->organism->strain) );
		if( cell->organism->oflags & ORGANISM_FLAG_RADIOACTIVE ) {
			pDC->SelectObject( &m_appearance.dyePen );
		} else {
			pDC->SelectObject( m_appearance.cellPen(cell->organism->strain) );
		}
	}

	pDC->Rectangle(rect);

	if( active ) {
		pDC->SelectObject(&m_appearance.activeBrush);
		pDC->SelectObject(&m_appearance.activePen);
		pDC->Rectangle(rect);
	}
}

void ViewOrganismDialog::repaint_organism()
{
	InvalidateRect(&win_rect);
}

void ViewOrganismDialog::make_transform()
{
	TF_RECT win, world, ob;
	bool first;
	CELL *curr;
	double window_ratio, world_ratio;

	ob.left		= 0.0;
	ob.right	= 0.0;
	ob.top		= 0.0;
	ob.bottom	= 0.0;

	/*
	 * Compute bounds of organism in 'ob'
	 */
	first = true;
	for(curr=organism->cells; curr; curr=curr->next) {
		if( first ) {
			first = false;
			ob.left = curr->x;
			ob.top = curr->y;
			ob.right = curr->x;
			ob.bottom = curr->y;
		} else {
			if( curr->x > ob.right )
				ob.right = curr->x;

			if( curr->x < ob.left )
				ob.left = curr->x;

			if( curr->y > ob.bottom )
				ob.bottom = curr->y;

			if( curr->y < ob.top )
				ob.top = curr->y;
		}
	}

	world = ob;

	/*
	 * Grow bounds by 1.5
	 */
	world.left	-= 1.5;
	world.right	+= 1.5;
	world.top	-= 1.5;
	world.bottom	+= 1.5;

	win.left	= win_rect.left;
	win.right	= win_rect.right;
	win.top		= win_rect.top;
	win.bottom	= win_rect.bottom;

	/*
	 * Preserve aspect ratio.
	 */
	window_ratio = (win.bottom - win.top) / (win.right - win.left);
	world_ratio = (world.bottom - world.top) / (world.right - world.left);

	if( world_ratio > window_ratio ) {
		win.right = win.left +
				(win.bottom - win.top) / world_ratio;
	} else {
		win.bottom = win.top +
				(win.right - win.left) * world_ratio;
	}

	TF_Set(tf, &win, &world);
}

void ViewOrganismDialog::draw_organism()
{
	CDC *pDC;
	CELL *curr;
	TF_RECT ugworld, ugwin;
	CBrush *pOldBrush;
	CPen *pOldPen;
	int i;
	PAINTSTRUCT ps;
	bool active;

	if( organism == NULL )
		return;

	/*
	 * top=430
	 * bottom=593
	 * left=461
	 * right=719
	 *
	 * (width=258, height=163)
	 */
	pDC = BeginPaint(&ps);

	pOldBrush = pDC->SelectObject(&m_appearance.defaultBrush);
	pOldPen = pDC->SelectObject(&m_appearance.defaultPen);

	/*
	 * Draw/erase background
	 */
	pDC->SelectObject(&m_appearance.organicBrush);
	pDC->SelectObject(&m_appearance.organicPen);
	pDC->Rectangle(&win_rect);

	i = 1;
	for(curr=organism->cells; curr; curr=curr->next) {
		ugworld.left	= curr->x - 0.5;
		ugworld.top	= curr->y - 0.5;
		ugworld.right	= curr->x + 0.5;
		ugworld.bottom	= curr->y + 0.5;

		TF_WorldToWin(tf, ugworld.left, ugworld.top,
					&ugwin.left, &ugwin.top);

		TF_WorldToWin(tf, ugworld.right, ugworld.bottom,
					&ugwin.right, &ugwin.bottom);

		if( curr == cell )
			active = true;
		else
			active = false;

		draw_cell(pDC, curr, active, i, ugwin);
		i++;
	}

	pDC->SelectObject(pOldBrush);
	pDC->SelectObject(pOldPen);

	EndPaint(&ps);
}

void ViewOrganismDialog::DoDataExchange(CDataExchange* pDX)
{
	CString str;
	int strain;
	int num_living_children;
	UNIVERSE *u;
	ORGANISM *o;

	CDialog::DoDataExchange(pDX);

	if( organism == NULL )
		return;

	if( pDX->m_bSaveAndValidate )
		return;

	DDX_CommaValue(pDX, IDC_VO_ID, organism->id);
	DDX_CommaValue(pDX, IDC_VO_PARENT1, organism->parent1);

	if( organism->parent1 == organism->parent2 ) {
		str = "";
		DDX_Text(pDX, IDC_VO_PARENT2, str);
	} else {
		DDX_CommaValue(pDX, IDC_VO_PARENT2, organism->parent2);
	}

	/*
	 * Count number of organisms which have me as a parent,
	 * this is the number of living children I have.
	 */
	u = organism->universe;
	num_living_children = 0;
	for(o = u->organisms; o; o=o->next) {
		if( o->parent1 == organism->id || o->parent2 == organism->id ) {
			num_living_children += 1;
		}
	}

	DDX_CommaValue(pDX, IDC_VO_ENERGY, organism->energy);
	DDX_CommaValue(pDX, IDC_VO_AGE, organism->age);
	DDX_CommaValue(pDX, IDC_VO_NUM_CELLS, organism->ncells);
	DDX_CommaValue(pDX, IDC_VO_NUM_LCHILD, num_living_children);
	DDX_CommaValue(pDX, IDC_VO_GENERATION, organism->generation);
	strain = organism->strain+1;
	DDX_CommaValue(pDX, IDC_VO_STRAIN, strain);

	CString xytype;
	if( organism->oflags & ORGANISM_FLAG_SEXONLY ) {
		xytype = "XY";
	} else {
		xytype = "";
	}
	DDX_Text(pDX, IDC_VO_XYTYPE, xytype);

	if( ! disassembly_updated ) {
		DDX_Text(pDX, IDC_VO_INSTRUCTIONS, disassembly);
		disassembly_updated = TRUE;
	}

	DDX_Cell(pDX);
	repaint_organism();
}

void ViewOrganismDialog::set_new_position()
{
	KFORTH_MACHINE *kfm;
	int i;

	cursor_start = -1;
	cursor_end = -1;

	if( kfd == NULL )
		return;

	if( cell == NULL )
		return;

	kfm = cell->kfm;

	for(i=0; i < kfd->pos_len; i++) {
		if( kfd->pos[i].cb == kfm->cb && kfd->pos[i].pc == kfm->pc ) {
			cursor_start = kfd->pos[i].start_pos;
			cursor_end = kfd->pos[i].end_pos + 1;
			break;
		}
	}
}

void ViewOrganismDialog::disassemble_program()
{
	KFORTH_PROGRAM *kfp;
	KFORTH_MACHINE *kfm;

	if( kfd != NULL ) {
		kforth_disassembly_delete(kfd);
		kfd = NULL;
	}

	kfp = organism->program;
	kfm = cell->kfm;

	kfd = kforth_disassembly_make(kfp, 60, 1);

	disassembly = kfd->program_text;
	disassembly_updated = FALSE;
}

void ViewOrganismDialog::SetOrganism(ORGANISM *o)
{
	ASSERT( o != NULL );

	organism = o;
	cell = o->cells;

	disassemble_program();
	set_new_position();
	make_transform();
}

void ViewOrganismDialog::Help()
{
	CString str;
	CWinApp *app;

	app = AfxGetApp();
	str = app->GetProfileString("help", "path");
	str = str + "\\view_organism_dialog.html";
	ShellExecute(m_hWnd, "open", str, NULL, NULL, SW_SHOWNORMAL);
}

void ViewOrganismDialog::Vision()
{
	VisionDialog dlg;

	dlg.SetCell(cell);
	dlg.DoModal();
}

void ViewOrganismDialog::Insert()
{
	KforthInstructionDialog dlg;
	CString str;

	str = find_instruction();
	dlg.insert_button(FALSE);
	dlg.go_to(str);
	dlg.DoModal();
}

void ViewOrganismDialog::Simulate()
{
	UNIVERSE *u;
	CELL *curr;
	int found;

	u = organism->universe;

	Universe_Simulate(u);

	SimulateSteps += 1;

	if( Universe_GetSelection(u) == NULL ) {
		//
		// The organism has died, because the selection
		// is gone. (message box here???)
		//
		AfxMessageBox("Organism has died.", MB_OK, 0);
		EndDialog(IDOK);
	} else {
		//
		// 'cell' may be gone, so we need to check that
		// cell still exists, and if not set cell to
		// something that does exist.
		//

		found = 0;
		for(curr=organism->cells; curr; curr=curr->next) {
			if( curr == cell ) {
				found = 1;
			}
		}
		if( ! found ) {
			AfxMessageBox("Cell has died.", MB_OK, 0);
			cell = organism->cells;
		}
		set_new_position();
		make_transform();
		UpdateData(false);
	}
}

void ViewOrganismDialog::StepOver()
{
	CevolveApp *app;
	KFORTH_MACHINE *kfm;
	UNIVERSE *u;
	int css, pc, cb;
	int step, step_limit;
	CELL *curr;
	int found;

	u = organism->universe;
	kfm = cell->kfm;

	if( kfm->terminated ) {
		return;
	}

	app = (CevolveApp *) AfxGetApp();

	css = kfm->call_stack_size;
	cb = kfm->cb;
	pc = kfm->pc;

	/*
	 * Do the normal SingleStep operation if this
	 * instruction is not a call, if, ifelse instruction.
	 */
	if( ! is_call_instruction(kfm, cb, pc) ) {
		Simulate();
		return;
	}

	step_limit = 1000;

	app->DoWaitCursor(1);

	step = 0;
	for(;;) {
		if( step > 0 && breakpoint_reached() ) {
			break;
		}

		step++;
		if( step > step_limit ) {
			AfxMessageBox("Step limit reached.", MB_OK, 0);
			break;
		}

		Universe_Simulate(u);

		SimulateSteps += 1;

		if( Universe_GetSelection(u) == NULL ) {
			//
			// The organism has died, because the selection
			// is gone.
			//
			AfxMessageBox("Organism has died.", MB_OK, 0);
			EndDialog(IDOK);
			app->DoWaitCursor(-1);
			return;
		} else {
			//
			// 'cell' may be gone, so we need to check that
			// cell still exists, and if not set cell to
			// something that does exist.
			//
			found = 0;
			for(curr=organism->cells; curr; curr=curr->next) {
				if( curr == cell ) {
					found = 1;
				}
			}
			if( ! found ) {
				//
				// Cell is gone, so pick another one.
				//
				AfxMessageBox("Cell has died.", MB_OK, 0);
				cell = organism->cells;
				break;
			}

		}

		if( kfm->call_stack_size == css
				&& kfm->cb == cb && kfm->pc == pc+1 ) {
			break;
		}
	}

	app->DoWaitCursor(-1);

	set_new_position();
	make_transform();
	UpdateData(false);

}

void ViewOrganismDialog::Breakpoint()
{
	KFORTH_MACHINE *kfm;
	CEdit *pED = (CEdit*) GetDlgItem(IDC_VO_INSTRUCTIONS);
	bool found;
	int schar, echar;
	int i;
	int dot_pos, cb, pc;
	BPLIST *curr, *prev;

	kfm = cell->kfm;

	pED->GetSel(schar, echar);

	if( schar < 0 ) {
		return;
	}

	found = FALSE;
	for(i=0; i < kfd->pos_len; i++) {
		if( kfd->pos[i].pc == -1 )
			continue;

		if( kfd->pos[i].end_pos >= schar ) {
			found = TRUE;
			break;
		}
	}

	if( ! found )
		return;

	cb = kfd->pos[i].cb;
	pc = kfd->pos[i].pc;

	if( pc == kfm->program->block_len[cb] ) {
		/*
		 * Caret is pointed to past the last instruction
		 * don't handle yet.
		 */
		dot_pos = kfd->pos[i].start_pos-3;
	} else {
		dot_pos = kfd->pos[i].start_pos-1;
	}

	found = FALSE;
	prev = NULL;
	for(curr=m_breakpoints; curr; curr=curr->next) {
		if( curr->cb == cb && curr->pc == pc ) {
			found = TRUE;
			break;
		}
		prev = curr;
	}

	if( ! found ) {
		curr = new BPLIST();
		curr->cb = cb;
		curr->pc = pc;
		curr->next = m_breakpoints;
		m_breakpoints = curr;
	} else {
		if( prev ) {
			prev->next = curr->next;
			delete curr;
		} else {
			m_breakpoints = curr->next;
			delete curr;
		}
	}

	pED->SetSel(dot_pos, dot_pos+1, FALSE);
	if( found )
		pED->ReplaceSel(" ");
	else
		pED->ReplaceSel("|");

	pED->SetSel(dot_pos, dot_pos+1, TRUE);
}

//
// Actually this button will just clear all the breakpoints.
//
void ViewOrganismDialog::Watchpoint()
{
	CEdit *pED = (CEdit*) GetDlgItem(IDC_VO_INSTRUCTIONS);
	KFORTH_PROGRAM *kfp;
	BPLIST *curr, *nxt;
	BOOL found;
	int dot_pos, i, count;

	kfp = cell->kfm->program;

	count = 0;
	for(curr=m_breakpoints; curr; curr=nxt) {
		nxt = curr->next;

		found = FALSE;
		for(i=0; i < kfd->pos_len; i++) {
			if( kfd->pos[i].cb == curr->cb && kfd->pos[i].pc == curr->pc ) {
				found = TRUE;
				break;
			}
		}

		if( found ) {
			count++;

			if( curr->pc == kfp->block_len[curr->cb] ) {
				dot_pos = kfd->pos[i].start_pos - 3;
			} else {
				dot_pos = kfd->pos[i].start_pos - 1;
			}
			pED->SetSel(dot_pos, dot_pos+1, TRUE);
			pED->ReplaceSel(" ");
		}

		delete curr;
	}

	m_breakpoints = NULL;

	nice_select(cursor_start, cursor_end);
}

void ViewOrganismDialog::Run()
{
	CevolveApp *app;
	KFORTH_MACHINE *kfm;
	UNIVERSE *u;
	int step, step_limit;
	CELL *curr;
	int found;

	u = organism->universe;
	kfm = cell->kfm;

	if( kfm->terminated ) {
		return;
	}

	app = (CevolveApp *) AfxGetApp();

	step_limit = 1000;

	app->DoWaitCursor(1);

	step = 0;
	for(;;) {
		if( step > 0 && breakpoint_reached() ) {
			break;
		}

		step++;
		if( step > step_limit ) {
			AfxMessageBox("Step limit reached.", MB_OK, 0);
			break;
		}

		Universe_Simulate(u);

		SimulateSteps += 1;

		if( Universe_GetSelection(u) == NULL ) {
			//
			// The organism has died, because the selection
			// is gone.
			//
			AfxMessageBox("Organism has died.", MB_OK, 0);
			EndDialog(IDOK);
			app->DoWaitCursor(-1);
			return;
		} else {
			//
			// 'cell' may be gone, so we need to check that
			// cell still exists, and if not set cell to
			// something that does exist.
			//
			found = 0;
			for(curr=organism->cells; curr; curr=curr->next) {
				if( curr == cell ) {
					found = 1;
				}
			}
			if( ! found ) {
				//
				// Cell is gone, so pick another one.
				//
				AfxMessageBox("Cell has died.", MB_OK, 0);
				cell = organism->cells;
				break;
			}

		}

	}

	app->DoWaitCursor(-1);

	set_new_position();
	make_transform();
	UpdateData(false);
}

afx_msg void ViewOrganismDialog::OnPaint()
{
	draw_organism();
}

afx_msg void ViewOrganismDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	double fx, fy;
	int x, y;
	CELL *curr;

	CDialog::OnLButtonDown(nFlags, point);

	if( point.x >= win_rect.left && point.x <= win_rect.right
			&& point.y >= win_rect.top && point.y <= win_rect.bottom ) {

		TF_WinToWorld(tf, point.x, point.y, &fx, &fy);
		x = (int) floor(fx+0.5);
		y = (int) floor(fy+0.5);

		for(curr=organism->cells; curr; curr=curr->next) {
			if( curr->x == x && curr->y == y ) {
				cell = curr;
				set_new_position();
				UpdateData(false);
			}
		}
	}
}

afx_msg BOOL ViewOrganismDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

#if 0
	CEdit *pED;
//
// way to ugly for my liking...
//
	CFont font1;
	font1.CreateFont(
	   14,                        // nHeight
	   0,                         // nWidth
	   0,                         // nEscapement
	   0,                         // nOrientation
	   FW_NORMAL,                 // nWeight
	   FALSE,                     // bItalic
	   FALSE,                     // bUnderline
	   0,                         // cStrikeOut
	   ANSI_CHARSET,              // nCharSet
	   OUT_DEFAULT_PRECIS,        // nOutPrecision
	   CLIP_DEFAULT_PRECIS,       // nClipPrecision
	   DEFAULT_QUALITY,           // nQuality
	   DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
	   "Courier New");                 // lpszFacename

	pED = (CEdit*) GetDlgItem(IDC_VO_INSTRUCTIONS);
	pED->SetFont(&font1);
	font1.Detach(); // prevent HFONT from being deleted
#endif

	return TRUE;
}

afx_msg BOOL ViewOrganismDialog::PreTranslateMessage(MSG* pMsg)
{
	if( pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN ) {
		if( pMsg->wParam == VK_F5 ) {
			PostMessage(WM_COMMAND, IDC_VO_RUN, 0L);
			return TRUE;

		} else if( pMsg->wParam == VK_F8 ) {
			PostMessage(WM_COMMAND, IDC_VO_WATCHPOINT, 0L);
			return TRUE;

		} else if( pMsg->wParam == VK_F9 ) {
			PostMessage(WM_COMMAND, IDC_VO_BREAKPOINT, 0L);
			return TRUE;

		} else if( pMsg->wParam == VK_F10 ) {
			PostMessage(WM_COMMAND, IDC_VO_STEP_OVER, 0L);
			return TRUE;

		} else if( pMsg->wParam == VK_F11 ) {
			PostMessage(WM_COMMAND, IDC_VO_SIMULATE, 0L);
			return TRUE;
		}
	} 

	return CDialog::PreTranslateMessage(pMsg);
}

bool ViewOrganismDialog::breakpoint_reached()
{
	KFORTH_MACHINE *kfm;
	BPLIST *curr;
	bool found;

	kfm = cell->kfm;

	found = FALSE;
	for(curr=m_breakpoints; curr; curr=curr->next) {
		if( kfm->cb == curr->cb && kfm->pc == curr->pc ) {
			found = TRUE;
			break;
		}
	}

	return found;
}

bool ViewOrganismDialog::is_call_instruction(KFORTH_MACHINE *kfm, int cb, int pc)
{
	unsigned char opcode;

	ASSERT( kfm != NULL );

	if( pc >= kfm->program->block_len[cb] ) {
		return FALSE;
	}

	opcode = kfm->program->opcode[cb][pc];

	/*
	 * 1 is opcode "call"
	 * 2 is opcode "if"
	 * 3 is opcode "ifelse"
	 */
	if( opcode == 1 || opcode == 2 || opcode == 3 )
		return TRUE;
	else
		return FALSE;
}

CString ViewOrganismDialog::find_instruction()
{
	CEdit *pED = (CEdit*) GetDlgItem(IDC_VO_INSTRUCTIONS);
	char buf[ 1000 ], *p;
	int schar, echar;
	bool found;
	int i, j;

	if( kfd == NULL )
		return "";

	pED->GetSel(schar, echar);

	if( schar < 0 )
		return "";

	found = FALSE;
	for(i=0; i < kfd->pos_len; i++) {
		if( kfd->pos[i].pc == -1 )
			continue;

		if( kfd->pos[i].end_pos >= schar ) {
			found = TRUE;
			break;
		}
	}

	if( ! found )
		return "";

	p = buf;
	for(j=kfd->pos[i].start_pos; j <= kfd->pos[i].end_pos; j++) {
		*p++ = kfd->program_text[j];
	}
	*p = '\0';

	return buf;
}

void ViewOrganismDialog::nice_select(int cpos1, int cpos2)
{
	CEdit *pED = (CEdit*) GetDlgItem(IDC_VO_INSTRUCTIONS);

	pED->SetSel(cpos1, cpos2, FALSE);
}

BEGIN_MESSAGE_MAP(ViewOrganismDialog, CDialog)
	ON_COMMAND(IDC_VO_HELP, Help)
	ON_COMMAND(IDC_VO_VISION, Vision)
	ON_COMMAND(IDC_VO_INSERT, Insert)
	ON_COMMAND(IDC_VO_SIMULATE, Simulate)
	ON_COMMAND(IDC_VO_STEP_OVER, StepOver)
	ON_COMMAND(IDC_VO_RUN, Run)
	ON_COMMAND(IDC_VO_BREAKPOINT, Breakpoint)
	ON_COMMAND(IDC_VO_WATCHPOINT, Watchpoint)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()


// ViewOrganismDialog message handlers
