// KforthDialog.cpp : implementation file
//

#include "stdafx.h"
#include "evolve.h"
#include "KforthDialog.h"

//
// Dummy EVOLVE operations, which don't perform
// any actual operations.
//
static void kfd_OMOVE(KFORTH_MACHINE *kfm)
{
	if( kfm->data_stack_size < 2 )
		return;

	kforth_data_stack_pop(kfm);
	kforth_data_stack_pop(kfm);
	kforth_data_stack_push(kfm, 0);
}

static void kfd_CMOVE(KFORTH_MACHINE *kfm)
{
	if( kfm->data_stack_size < 2 )
		return;

	kforth_data_stack_pop(kfm);
	kforth_data_stack_pop(kfm);
	kforth_data_stack_push(kfm, 0);

}

static void kfd_ROTATE(KFORTH_MACHINE *kfm)
{
	if( kfm->data_stack_size < 1 )
		return;

	kforth_data_stack_pop(kfm);
	kforth_data_stack_push(kfm, 0);

}

static void kfd_EAT(KFORTH_MACHINE *kfm)
{
	if( kfm->data_stack_size < 2 )
		return;

	kforth_data_stack_pop(kfm);
	kforth_data_stack_pop(kfm);
	kforth_data_stack_push(kfm, 0);
}

static void kfd_MAKE_SPORE(KFORTH_MACHINE *kfm)
{
	if( kfm->data_stack_size < 3 )
		return;

	kforth_data_stack_pop(kfm);
	kforth_data_stack_pop(kfm);
	kforth_data_stack_pop(kfm);
	kforth_data_stack_push(kfm, 0);
}

static void kfd_GROW(KFORTH_MACHINE *kfm)
{
	if( kfm->data_stack_size < 2 )
		return;

	kforth_data_stack_pop(kfm);
	kforth_data_stack_pop(kfm);
	kforth_data_stack_push(kfm, 0);
}

static void kfd_LOOK(KFORTH_MACHINE *kfm)
{
	if( kfm->data_stack_size < 2 )
		return;

	kforth_data_stack_pop(kfm);
	kforth_data_stack_pop(kfm);

	kforth_data_stack_push(kfm, 0);
	kforth_data_stack_push(kfm, 0);
}

static void kfd_WHAT(KFORTH_MACHINE *kfm)
{
	if( kfm->data_stack_size < 2 )
		return;

	kforth_data_stack_pop(kfm);
	kforth_data_stack_pop(kfm);
	kforth_data_stack_push(kfm, 0);
}

static void kfd_WHERE(KFORTH_MACHINE *kfm)
{
	if( kfm->data_stack_size < 2 )
		return;

	kforth_data_stack_pop(kfm);
	kforth_data_stack_pop(kfm);
	kforth_data_stack_push(kfm, 0);
}

static void kfd_NEAREST(KFORTH_MACHINE *kfm)
{
	if( kfm->data_stack_size < 1 )
		return;

	kforth_data_stack_pop(kfm);
	kforth_data_stack_push(kfm, 0);
	kforth_data_stack_push(kfm, 0);
}

static void kfd_FARTHEST(KFORTH_MACHINE *kfm)
{
	if( kfm->data_stack_size < 1 )
		return;

	kforth_data_stack_pop(kfm);
	kforth_data_stack_push(kfm, 0);
	kforth_data_stack_push(kfm, 0);
}

static void kfd_NEAREST2(KFORTH_MACHINE *kfm)
{
	if( kfm->data_stack_size < 2 )
		return;

	kforth_data_stack_pop(kfm);
	kforth_data_stack_pop(kfm);
	kforth_data_stack_push(kfm, 0);
	kforth_data_stack_push(kfm, 0);
}

static void kfd_FARTHEST2(KFORTH_MACHINE *kfm)
{
	if( kfm->data_stack_size < 2 )
		return;

	kforth_data_stack_pop(kfm);
	kforth_data_stack_pop(kfm);
	kforth_data_stack_push(kfm, 0);
	kforth_data_stack_push(kfm, 0);
}

static void kfd_MOOD(KFORTH_MACHINE *kfm)
{
	if( kfm->data_stack_size < 2 )
		return;

	kforth_data_stack_pop(kfm);
	kforth_data_stack_pop(kfm);
	kforth_data_stack_push(kfm, 0);
}

static void kfd_SET_MOOD(KFORTH_MACHINE *kfm)
{
	if( kfm->data_stack_size < 1 )
		return;

	kforth_data_stack_pop(kfm);
}

static void kfd_BROADCAST(KFORTH_MACHINE *kfm)
{
	if( kfm->data_stack_size < 1 )
		return;

	kforth_data_stack_pop(kfm);
}

static void kfd_SEND(KFORTH_MACHINE *kfm)
{
	if( kfm->data_stack_size < 3 )
		return;

	kforth_data_stack_pop(kfm);
	kforth_data_stack_pop(kfm);
	kforth_data_stack_pop(kfm);
}

static void kfd_RECV(KFORTH_MACHINE *kfm)
{
	kforth_data_stack_push(kfm, 0);
}

static void kfd_ENERGY(KFORTH_MACHINE *kfm)
{
	kforth_data_stack_push(kfm, 0);
}

static void kfd_AGE(KFORTH_MACHINE *kfm)
{
	kforth_data_stack_push(kfm, 0);
}

static void kfd_NUM_CELLS(KFORTH_MACHINE *kfm)
{
	kforth_data_stack_push(kfm, 0);
}

static void kfd_HAS_NEIGHBOR(KFORTH_MACHINE *kfm)
{
	if( kfm->data_stack_size < 2 )
		return;

	kforth_data_stack_pop(kfm);
	kforth_data_stack_pop(kfm);
	kforth_data_stack_push(kfm, 0);
}


IMPLEMENT_DYNAMIC(KforthDialog, CDialog)
KforthDialog::KforthDialog(CWnd* pParent /*=NULL*/)
	: CDialog(KforthDialog::IDD, pParent)
{
	kfops = kforth_ops_make();
	kforth_ops_add(kfops,	"CMOVE",	kfd_CMOVE);
	kforth_ops_add(kfops,	"OMOVE",	kfd_OMOVE);
	kforth_ops_add(kfops,	"ROTATE",	kfd_ROTATE);
	kforth_ops_add(kfops,	"EAT",		kfd_EAT);
	kforth_ops_add(kfops,	"MAKE-SPORE",	kfd_MAKE_SPORE);
	kforth_ops_add(kfops,	"GROW",		kfd_GROW);
	kforth_ops_add(kfops,	"LOOK",		kfd_LOOK);
	kforth_ops_add(kfops,	"WHAT",		kfd_WHAT);
	kforth_ops_add(kfops,	"WHERE",	kfd_WHERE);
	kforth_ops_add(kfops,	"NEAREST",	kfd_NEAREST);
	kforth_ops_add(kfops,	"FARTHEST",	kfd_FARTHEST);
	kforth_ops_add(kfops,	"NEAREST2",	kfd_NEAREST2);
	kforth_ops_add(kfops,	"FARTHEST2",	kfd_FARTHEST2);
	kforth_ops_add(kfops,	"MOOD",		kfd_MOOD);
	kforth_ops_add(kfops,	"MOOD!",	kfd_SET_MOOD);
	kforth_ops_add(kfops,	"BROADCAST",	kfd_BROADCAST);
	kforth_ops_add(kfops,	"SEND",		kfd_SEND);
	kforth_ops_add(kfops,	"RECV",		kfd_RECV);
	kforth_ops_add(kfops,	"ENERGY",	kfd_ENERGY);
	kforth_ops_add(kfops,	"AGE",		kfd_AGE);
	kforth_ops_add(kfops,	"NUM-CELLS",	kfd_NUM_CELLS);
	kforth_ops_add(kfops,	"HAS-NEIGHBOR",	kfd_HAS_NEIGHBOR);

	kfm = NULL;
	kfp = NULL;
	kfd = NULL;

	m_filename = "*.kf";
	m_breakpoints = NULL;

	disassembly_updated = FALSE;
}

KforthDialog::~KforthDialog()
{
	if( kfops != NULL ) {
		kforth_ops_delete(kfops);
	}

	if( kfm != NULL ) {
		kforth_machine_delete(kfm);
		kfm = NULL;
	}

	if( kfp != NULL ) {
		kforth_delete(kfp);
		kfp = NULL;
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

/*
 * Return the instruction name that
 * is closest to the caret position in the
 * instruction pane.
 *
 * If nothing can be found, return "".
 *
 */
CString KforthDialog::find_instruction()
{
	CEdit *pED = (CEdit*) GetDlgItem(IDC_KI_INSTRUCTIONS);
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

void KforthDialog::nice_select(int cpos1, int cpos2)
{
	CEdit *pED = (CEdit*) GetDlgItem(IDC_KI_INSTRUCTIONS);

	pED->SetSel(cpos1, cpos2, FALSE);
}

void KforthDialog::DDX_machine(CDataExchange* pDX)
{
	if( pDX->m_bSaveAndValidate )
		return;
		
	if( kfm != NULL ) {
		DDX_machine_registers(pDX);
		DDX_call_stack(pDX);
		DDX_data_stack(pDX);

		nice_select(cursor_start, cursor_end);
	}
}

void KforthDialog::DDX_machine_registers(CDataExchange* pDX)
{
	char buf[1000];
	CString str;

	sprintf(buf, "%I64d", kfm->R[0]);
	str = buf;
	DDX_Text(pDX, IDC_KI_R0, str);

	sprintf(buf, "%I64d", kfm->R[1]);
	str = buf;
	DDX_Text(pDX, IDC_KI_R1, str);

	sprintf(buf, "%I64d", kfm->R[2]);
	str = buf;
	DDX_Text(pDX, IDC_KI_R2, str);

	sprintf(buf, "%I64d", kfm->R[3]);
	str = buf;
	DDX_Text(pDX, IDC_KI_R3, str);

	sprintf(buf, "%I64d", kfm->R[4]);
	str = buf;
	DDX_Text(pDX, IDC_KI_R4, str);

	sprintf(buf, "%I64d", kfm->R[5]);
	str = buf;
	DDX_Text(pDX, IDC_KI_R5, str);

	sprintf(buf, "%I64d", kfm->R[6]);
	str = buf;
	DDX_Text(pDX, IDC_KI_R6, str);

	sprintf(buf, "%I64d", kfm->R[7]);
	str = buf;
	DDX_Text(pDX, IDC_KI_R7, str);

	sprintf(buf, "%I64d", kfm->R[8]);
	str = buf;
	DDX_Text(pDX, IDC_KI_R8, str);

	sprintf(buf, "%I64d", kfm->R[9]);
	str = buf;
	DDX_Text(pDX, IDC_KI_R9, str);

	if( kfm->terminated ) {
		str = "TERMINATED";
		DDX_Text(pDX, IDC_KI_PC, str);
		DDX_Text(pDX, IDC_KI_CB, str);
	} else {
		sprintf(buf, "%d", kfm->pc);
		str = buf;
		DDX_Text(pDX, IDC_KI_PC, str);

		sprintf(buf, "%d", kfm->cb);
		str = buf;
		DDX_Text(pDX, IDC_KI_CB, str);
	}
}

void KforthDialog::DDX_data_stack(CDataExchange* pDX)
{
	KFORTH_STACK_NODE *curr;
	int i;
	CString str;
	char buf[1000];

	CListBox *pLB = (CListBox*) GetDlgItem(IDC_KI_DATA_STACK);

	pLB->ResetContent();

	i = 0;
	for(curr=kfm->data_sp; curr; curr=curr->prev) {
		sprintf(buf, "%I64d", curr->u.data.value);

		str = buf;

		pLB->InsertString(i, str);

		i++;
	}
}

void KforthDialog::DDX_call_stack(CDataExchange* pDX)
{
	KFORTH_STACK_NODE *curr;
	int i;
	CString str;
	char buf[1000];

	CListBox *pLB = (CListBox*) GetDlgItem(IDC_KI_CALL_STACK);

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

void KforthDialog::DoDataExchange(CDataExchange* pDX)
{
	CEdit *pED;
	BOOL flag;

	/*
	 * Rememeber modified state of source pane.
	 */
	pED = (CEdit*) GetDlgItem(IDC_KI_SOURCE);
	flag = pED->GetModify();

	CDialog::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_KI_SOURCE, source_code);
	DDX_Text(pDX, IDC_KI_ERROR, error_message);

	if( ! disassembly_updated ) {
		DDX_Text(pDX, IDC_KI_INSTRUCTIONS, disassembly);
		disassembly_updated = TRUE;
	}

	DDX_machine(pDX);

	/*
	 * Restore modified flag for source pane.
	 */
	pED->SetModify(flag);
}

void KforthDialog::set_new_position()
{
	int i;

	cursor_start = -1;
	cursor_end = -1;

	if( kfd == NULL )
		return;

	for(i=0; i < kfd->pos_len; i++) {
		if( kfd->pos[i].cb == kfm->cb && kfd->pos[i].pc == kfm->pc ) {
			cursor_start = kfd->pos[i].start_pos;
			cursor_end = kfd->pos[i].end_pos + 1;
			break;
		}
	}
}

void KforthDialog::disassemble_program()
{
	if( kfd != NULL ) {
		kforth_disassembly_delete(kfd);
		kfd = NULL;
	}

	if( kfp == NULL ) {
		disassembly = "";
		return;
	}

	kfd = kforth_disassembly_make(kfp, 55, 1);

	disassembly = kfd->program_text;
	disassembly_updated = FALSE;
}

void KforthDialog::Compile()
{
	char errbuf[ 1000 ];

	/*
	 * Get contents of Edit control
	 */
	UpdateData(true);

	Watchpoint();

	if( kfp != NULL ) {
		kforth_delete(kfp);
		kfp = NULL;
	}

	if( kfm != NULL ) {
		kforth_machine_delete(kfm);
		kfm = NULL;
	}

	kfp = kforth_compile(source_code, kfops, errbuf);
	if( kfp == NULL ) {
		error_message = errbuf;
		disassembly = "";
	} else {
		kfm = kforth_machine_make(kfp, NULL);
		error_message = "";
		disassemble_program();
		set_new_position();
	}

	Reset();

	UpdateData(false);
}

/*
 * This checks the Source pane to see if it has been modified,
 * if it has then we ask the user if they wish to save.
 *
 * If they say YES, then save. If they say NO, then DON'T save.
 *
 * If they hit cancel, then return TRUE.
 *
 */
BOOL KforthDialog::CheckAndSave()
{
	char buf[ 1000 ];
	CString str;
	int x;

	CEdit *pED = (CEdit*) GetDlgItem(IDC_KI_SOURCE);
	if( pED->GetModify() ) {
		sprintf(buf, "Source has been modified. Do you wish to save to:\n%s?", m_filename);
		str = buf;
		x = AfxMessageBox(str, MB_YESNOCANCEL, 0);
		if( x == IDYES ) {
			// save file
			Save();
		}

		if( x == IDNO ) {
			// do nothing
			pED->SetModify(FALSE);
		}

		if( x == IDCANCEL ) {
			return true;
		}
	}

	return false;
}

void KforthDialog::Load()
{
	char filters[] = "KFORTH Files (*.kf)|*.kf|All Files (*.*)|*.*||";
	char buf[ 1000 ];
	FILE *fp;
	BOOL cancel;
	size_t len;

	cancel = CheckAndSave();
	if( cancel )
		return;

	// Create an Open dialog; the default file name extension is ".my".
	CFileDialog fileDlg(true, "kf", m_filename,
		OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, filters, this);
   
	// Display the file dialog. When user clicks OK, fileDlg.DoModal() 
	// returns IDOK.
	if( fileDlg.DoModal()== IDOK ) {
		CString filename = fileDlg.GetPathName();
		m_filename = filename;

		fp = fopen(filename, "r");
		if( fp == NULL ) {
			sprintf(buf, "%s: %s", (const char*) filename, strerror(errno));
			error_message = buf;
		} else {
			source_code = "";
			while( fgets(buf, sizeof(buf), fp) != NULL ) {
				len = strlen(buf);

				if( buf[len-1] == '\n' )
					buf[ len-1 ] = '\0';

				strcat(buf, "\r\n");
				source_code += buf;
			}
			fclose(fp);
			disassembly = "";
			error_message = "";

			Watchpoint(); // clear any breakpoints
		}
		UpdateData(false);
	}

}

void KforthDialog::Save()
{
	char filters[] = "KFORTH Files (*.kf)|*.kf|All Files (*.*)|*.*||";
	char buf[ 1000 ];
	const char *p;
	FILE *fp;

	UpdateData(true);

	// Create an Open dialog; the default file name extension is ".my".
	CFileDialog fileDlg(false, "kf", m_filename, 0, filters, this);
   
	// Display the file dialog. When user clicks OK, fileDlg.DoModal() 
	// returns IDOK.
	if( fileDlg.DoModal()== IDOK ) {
		CString filename = fileDlg.GetPathName();
		m_filename = filename;

		fp = fopen(filename, "w");
		if( fp == NULL ) {
			sprintf(buf, "%s: %s", (const char*) filename, strerror(errno));
			error_message = buf;
		} else {
			for(p = source_code; *p; p++) {
				if( *p == '\r' )
					continue;

				fputc(*p, fp);
			}
			fclose(fp);
			error_message = "";
		}
		UpdateData(false);

		CEdit *pED = (CEdit*) GetDlgItem(IDC_KI_SOURCE);
		pED->SetModify(FALSE);
	}
}

//
// Is the kforth machine 'kfm' pointing to a breakpoint?
//
bool KforthDialog::breakpoint_reached()
{
	BPLIST *curr;
	bool found;

	found = FALSE;
	for(curr=m_breakpoints; curr; curr=curr->next) {
		if( kfm->cb == curr->cb && kfm->pc == curr->pc ) {
			found = TRUE;
			break;
		}
	}

	return found;
}

void KforthDialog::Run()
{
	CevolveApp *app;
	int size, size_limit;
	int step, step_limit;

	if( kfm ) {
		app = (CevolveApp *) AfxGetApp();
		app->DoWaitCursor(1);

		size_limit = 10 * (1000);
		step_limit = 10 * (1000 * 1000);

		step = 0;
		while( ! kfm->terminated ) {
			if( step > 0 && breakpoint_reached() ) {
				error_message = "Breakpoint reached.";
				break;
			}

			size = kfm->call_stack_size + kfm->data_stack_size;
			if( size > size_limit ) {
				error_message = "Stack Size Limit Exceeded.";
				break;
			}

			step++;
			if( step > step_limit ) {
				error_message = "Step Limit Exceeded.";
				break;
			}

			kforth_machine_execute(kfm);
		}
		app->DoWaitCursor(-1);

	}

	set_new_position();

	UpdateData(false);
}

void KforthDialog::Reset()
{
	if( kfm ) {
		kforth_machine_reset(kfm);
		set_new_position();
	}

	UpdateData(false);
}

void KforthDialog::SingleStep()
{
	error_message = "";

	if( kfm != NULL ) {
		if( ! kfm->terminated ) {
			kforth_machine_execute(kfm);
			set_new_position();
		}
	}

	UpdateData(false);
}

bool KforthDialog::is_call_instruction(KFORTH_MACHINE *kfm, int cb, int pc)
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

void KforthDialog::StepOver()
{
	int size, size_limit;
	int step, step_limit;
	int css, pc, cb;
	CevolveApp *app;

	if( kfm == NULL ) {
		return;
	}

	if( kfm->terminated ) {
		return;
	}

	error_message = "";

	app = (CevolveApp *) AfxGetApp();

	css = kfm->call_stack_size;
	cb = kfm->cb;
	pc = kfm->pc;

	/*
	 * If it isn't a CALL, then just do single step.
	 */
	if( ! is_call_instruction(kfm, cb, pc) ) {
		SingleStep();
		return;
	}

	size_limit = 10 * (1000);
	step_limit = 10 * (1000 * 1000);

	app->DoWaitCursor(1);

	step = 0;
	while( ! kfm->terminated ) {
		if( step > 0 && breakpoint_reached() ) {
			error_message = "Breakpoint reached.";
			break;
		}

		size = kfm->call_stack_size + kfm->data_stack_size;
		if( size > size_limit ) {
			error_message = "Stack Size Limit Exceeded.";
			break;
		}

		step++;
		if( step > step_limit ) {
			error_message = "Step Limit Exceeded.";
			break;
		}

		kforth_machine_execute(kfm);

		if( kfm->call_stack_size == css
				&& kfm->cb == cb && kfm->pc == pc+1 ) {
			break;
		}
	}

	app->DoWaitCursor(-1);

	set_new_position();
	UpdateData(false);
}

void KforthDialog::Breakpoint()
{
	CEdit *pED = (CEdit*) GetDlgItem(IDC_KI_INSTRUCTIONS);
	bool found;
	int schar, echar;
	int i;
	int dot_pos, cb, pc;
	BPLIST *curr, *prev;

	if( kfd == NULL )
		return;

	if( kfp == NULL )
		return;

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

	if( pc == kfp->block_len[cb] ) {
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
// For now, this button is the "clear breakpoint" button.
//
void KforthDialog::Watchpoint()
{
#if 0
	WatchpointDialog wdlg;
	INT_PTR nRet;

	nRet = wdlg.DoModal();
	if( nRet == IDCANCEL )
		return;
#endif
	CEdit *pED = (CEdit*) GetDlgItem(IDC_KI_INSTRUCTIONS);
	BPLIST *curr, *nxt;
	BOOL found;
	int dot_pos, i, count;

	if( kfd == NULL )
		return;

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

void KforthDialog::Insert()
{
	CEdit *pED = (CEdit*) GetDlgItem(IDC_KI_SOURCE);
	KforthInstructionDialog instructionDlg;
	CString str;
	INT_PTR nRet;

	//
	// This feature is gross, better to
	// just not enable it. I doubt
	// anybody who writes serious kforth is
	// using this tool, and if they are
	// this feature is very limited.
	// Better to be less confusing, than
	// offer a feature that never gets used.
	//
	instructionDlg.insert_button(FALSE);

	str = find_instruction();

	instructionDlg.go_to(str);

	nRet = instructionDlg.DoModal();

	if( nRet == IDCANCEL )
		return;

	str = instructionDlg.GetInstruction();

	if( str.GetLength() == 0 )
		return;

	str = str + CString(" ");

	pED->ReplaceSel(str);
	error_message = "";
	UpdateData(true);
	UpdateData(false);
}

void KforthDialog::Help()
{
	CString str;
	CWinApp *app;

	app = AfxGetApp();
	str = app->GetProfileString("help", "path");
	str = str + "\\kforth_interpreter_dialog.html";
	ShellExecute(m_hWnd, "open", str, NULL, NULL, SW_SHOWNORMAL);
}

BOOL KforthDialog::OnInitDialog()
{
	CEdit *pED;

	CDialog::OnInitDialog();

	CFont font1;
#if 1
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
#else
	// too fucking big!
	font1.CreateStockObject(ANSI_FIXED_FONT); 
#endif

	pED = (CEdit*) GetDlgItem(IDC_KI_INSTRUCTIONS);
	pED->SetFont(&font1);
	font1.Detach(); // prevent HFONT from being deleted

	CFont font2;
#if 1
	font2.CreateFont(
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
#else
	// too fucking big!
	font2.CreateStockObject(ANSI_FIXED_FONT); 
#endif

	pED = (CEdit*) GetDlgItem(IDC_KI_SOURCE);
	pED->SetFont(&font2);
	font2.Detach(); // prevent HFONT from being deleted

	return TRUE;
}

BOOL KforthDialog::PreTranslateMessage(MSG* pMsg)
{
	if( pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN ) {
		if( pMsg->wParam == VK_F5 ) {
			PostMessage(WM_COMMAND, IDC_KI_RUN, 0L);
			return TRUE;

		} else if( pMsg->wParam == VK_F8 ) {
			PostMessage(WM_COMMAND, IDC_KI_WATCHPOINT, 0L);
			return TRUE;

		} else if( pMsg->wParam == VK_F9 ) {
			PostMessage(WM_COMMAND, IDC_KI_BREAKPOINT, 0L);
			return TRUE;

		} else if( pMsg->wParam == VK_F10 ) {
			PostMessage(WM_COMMAND, IDC_KI_STEP_OVER, 0L);
			return TRUE;

		} else if( pMsg->wParam == VK_F11 ) {
			PostMessage(WM_COMMAND, IDC_KI_STEP, 0L);
			return TRUE;
		}
	} 

	return CDialog::PreTranslateMessage(pMsg);
}

void KforthDialog::OnOK()
{
	BOOL cancel;

	//
	// Did the user modify the source window?
	//
	CEdit *pED = (CEdit*) GetDlgItem(IDC_KI_SOURCE);
	if( pED->GetModify() ) {
		cancel = CheckAndSave();
		if( cancel )
			return;
	}

	CDialog::OnOK();
	
}


BEGIN_MESSAGE_MAP(KforthDialog, CDialog)
	ON_COMMAND(IDC_KI_COMPILE, Compile)
	ON_COMMAND(IDC_KI_LOAD, Load)
	ON_COMMAND(IDC_KI_SAVE, Save)
	ON_COMMAND(IDC_KI_RUN, Run)
	ON_COMMAND(IDC_KI_RESET, Reset)
	ON_COMMAND(IDC_KI_STEP, SingleStep)
	ON_COMMAND(IDC_KI_INSERT, Insert)
	ON_COMMAND(IDC_KI_STEP_OVER, StepOver)
	ON_COMMAND(IDC_KI_BREAKPOINT, Breakpoint)
	ON_COMMAND(IDC_KI_WATCHPOINT, Watchpoint)
	ON_COMMAND(IDC_KI_HELP, Help)
END_MESSAGE_MAP()

// KforthDialog message handlers

