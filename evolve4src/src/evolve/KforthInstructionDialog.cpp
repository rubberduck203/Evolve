//
// KforthInstructionDialog.cpp : implementation file
//

#include "stdafx.h"
#include "evolve.h"
#include "KforthInstructionDialog.h"

//
// This dialog is used in several contexts
// The core KFORTH instructions will always be displayed in
// this list (all masks use MASK_K).
//
// MASK_XXXX flags are or'd together and associated
// with each entry in the KFI table.
//
// The contexts are:
//	Showing instructions in view organism dialog
//	showing instructions for the kforth interpreter dialog
//	showing valid instruction for Find expressions (MASK_F)
//
//

#define MASK_K	0x01	/* kforth core instructions */
#define MASK_C	0x02	/* cell instruction */
#define MASK_F	0x04	/* find instructions */

static struct {
	int	mask;
	char	*instruction;
	char	*comment;
	char	*description;

} KFI[] = {
	MASK_K,
	"call",		"( cb -- )",
	"remove 'cb' from the top of the data stack. Add this to number to our current"
	"code block number and then call this code block."
	"If code block is invalid, the request "
	"is ignored. In disassembly row123 is code-block 123, and so on.",

	MASK_K,
	"if",		"( expr cb -- )",
	"remove two items from top of stack. If expr is "
	"non-zero then call code-block 'cb'. Code block numbers are relative "
	"to the current code block being executed.",

	MASK_K,
	"ifelse",	"( expr cb1 cb2 -- )",
	"remove three items from top of stack. If expr is "
	"non-zero then call 'cb1', else call 'cb2'. Code block number "
	"are relative to the current code block being executed.",

	MASK_K,
	"?loop",	"( n -- )",
	"Remove one item from the stack. If value is non-zero "
	"jump to the start of the current code block. Otherwise "
	"continue with the next instruction after '?loop'.",

	MASK_K,
	"?exit",	"( n -- )",
	"Remove one item from the stack. If non-zero then "
	"exit current code block.",

	MASK_K,
	"pop",		"( n -- )",
	"Remove item from stack and discard it.",

	MASK_K,
	"dup",		"( a -- a a )",
	"Duplicate item on top of the stack.",

	MASK_K,
	"swap",		"( a b -- b a )",
	"Swap top two elements on the stack.",

	MASK_K,
	"over",		"( a b -- a b a )",
	"Copy second item from the stack.",

	MASK_K,
	"rot",		"( a b c -- b c a )",
	"Rotate third item to top.",

	MASK_K,
	"pick",		"( ... v2 v1 v0 n -- ... v3 v2 v1 vn )",
	"Grab arbitrary item on the stack. n is the item to grab. "
	"n numbering begins with 0. If n is bogus, then n is removed "
	"from the stack, but no 'vn' is pushed on the stack ",

	MASK_K,
	"?dup",		"( n -- n n | 0 )",
	"Duplicate top element if non-zero.",

	MASK_K,
	"-rot",		"( a b c -- c a b )",
	"Rotate top to third position.",

	MASK_K,
	"2swap",	"( a b c d --  c d a b )",
	"Swap pairs.",

	MASK_K,
	"2over",	"( a b c d -- a b c d a b)",
	"Leapfrog pair.",

	MASK_K,
	"2dup",		"( a b -- a b a b )",
	"Dupicate pair.",

	MASK_K,
	"2pop",		"( a b  -- )",
	"Remove pair.",

	MASK_K,
	"nip",		"( a b -- b )",
	"Remove 2nd item from stack.",

	MASK_K,
	"tuck",		"( a b -- b a b)",
	"Copy top item to third position.",

	MASK_K,
	"1+",		"( n -- n+1 )",
	"Add 1 to the item on top of the stack.",

	MASK_K,
	"1-",		"( n -- n-1 )",
	"Subtract 1 from item on top of the stack.",

	MASK_K,
	"2+",		"( n -- n+2 )",
	"Add 2 to item on top of the stack",

	MASK_K,
	"2-",		"( n -- n-2 )",
	"Subtract 2 from the item on top of the stack.",

	MASK_K,
	"2/",		"( n -- n/2 )",
	"Half value.",

	MASK_K,
	"2*",		"( n -- n*2 )",
	"Double value.",

	MASK_K,
	"abs",		"( n -- abs(n) )",
	"Absolute value of n.",

	MASK_K,
	"sqrt",		"( n -- sqrt(n) )",
	"Square root. n must be positive. If n isn't positive then leave stack unchanged.",

	MASK_K,
	"+",		"( a b -- a+b )",
	"Add top two elements on stack.",

	MASK_K,
	"-",		"( a b -- a-b )",
	"Subtract first item on stack from the second item.",

	MASK_K,
	"*",		"( a b -- a*b )",
	"Multiply top two elements on the data stack.",

	MASK_K,
	"/",		"( a b -- a/b )",
	"Divide.",

	MASK_K,
	"mod",		"( a b -- a%b )",
	"Modulos.",

	MASK_K,
	"/mod",		"( a b -- a%b a/b )",
	"Divide and modulos.",

	MASK_K,
	"negate",	"( n -- -n )",
	"Negate item on stack",

	MASK_K,
	"2negate",	"( a b -- -a -b )",
	"negate top two items on stack.",

	MASK_K,
	"=",		"( a b -- EQ(a,b) )",
	"Equal to. 1 means 'a' and 'b' are the same, else 0.",

	MASK_K,
	"<>",		"( a b -- NE(a,b) )",
	"Not equal to. 1 means 'a' and 'b' are not equal, else 0.",

	MASK_K,
	"<",		"( a b -- LT(a,b) )",
	"Less than. 1 means 'a' is less than 'b', else 0.",

	MASK_K,
	">",		"( a b -- GT(a,b) )",
	"Greater than. 1 means 'a' is greater than 'b', else 0.",

	MASK_K,
	"<=",		"( a b -- LE(a,b) )",
	"Less than or equal to.",

	MASK_K,
	">=",		"( a b -- GE(a,b) )",
	"Greater than or equal to.",

	MASK_K,
	"0=",		"( n -- EQ(n,0) )",
	"Is element on top of the stack equal to 0?",

	MASK_K,
	"or",		"( a b -- a|b )",
	"Bitwise OR. Can be used as a logical OR operator too, because "
	"KFORTH boolean operators return 1 and 0.",

	MASK_K,
	"and",		"( a b -- a&b )",
	"Bitwise AND. Can be used a a logical AND operator too, because "
	"KFORTH boolean operators return 1 and 0.",

	MASK_K,
	"not",		"( n -- !n )",
	"Logical NOT. Any non-zero value will be converted to 0. 0 will be converted to 1.",

	MASK_K,
	"invert",	"( n -- ~n )",
	"Invert bits (Bitwise NOT).",

	MASK_K,
	"xor",		"( a b -- a^b )",
	"bitwise XOR function.",

	MASK_K,
	"min",		"( a b -- min(a,b) )",
	"Minimum value.",

	MASK_K,
	"max",		"( a b  -- max(a,b) )",
	"Remove two items from stack and replace with "
	"maximum value.",

	MASK_K,
	"CB",		"(  -- CB )",
	"Pushes the current code block number on the data stack.",

	MASK_K,
	"Rn",		"( n -- Rn )",
	"Pushes register Rn on the data stack. If "
	"n is not 0-9 the request is ignored.",

	MASK_K,
	"R0",		"(   -- R0 )",
	"Pushes register R0 on the data stack",

	MASK_K,
	"R1",		"(   -- R1 )",
	"Pushes register R1 on the data stack",

	MASK_K,
	"R2",		"(   -- R2 )",
	"Pushes register R2 on the data stack",

	MASK_K,
	"R3",		"(   -- R3 )",
	"Pushes register R3 on the data stack",

	MASK_K,
	"R4",		"(   -- R4 )",
	"Pushes register R4 on the data stack",

	MASK_K,
	"R5",		"(   -- R5 )",
	"Pushes register R5 on the data stack",

	MASK_K,
	"R6",		"(   -- R6 )",
	"Pushes register R6 on the data stack",

	MASK_K,
	"R7",		"(   -- R7 )",
	"Pushes register R7 on the data stack",

	MASK_K,
	"R8",		"(   -- R8 )",
	"Pushes register R8 on the data stack",

	MASK_K,
	"R9",		"(   -- R9 )",
	"Pushes register R9 on the data stack",

	MASK_K,
	"Rn!",		"( val n -- )",
	"Removes 2 items from the data stack "
	"'n' is the register and 'val' is the value to "
	"store into register Rn. "
	"If 'n' is not 0-9 then this instruction ignores the request.",

	MASK_K,
	"R0!",		"( val -- )",
	"Removes 1 item 'val' from the data stack "
	"and stores 'val' into register R0",

	MASK_K,
	"R1!",		"( val -- )",
	"Removes 1 item 'val' from the data stack "
	"and stores 'val' into register R1",

	MASK_K,
	"R2!",		"( val -- )",
	"Removes 1 item 'val' from the data stack "
	"and stores 'val' into register R2",

	MASK_K,
	"R3!",		"( val -- )",
	"Removes 1 item 'val' from the data stack "
	"and stores 'val' into register R3",

	MASK_K,
	"R4!",		"( val -- )",
	"Removes 1 item 'val' from the data stack "
	"and stores 'val' into register R4",

	MASK_K,
	"R5!",		"( val -- )",
	"Removes 1 item 'val' from the data stack "
	"and stores 'val' into register R5",

	MASK_K,
	"R6!",		"( val -- )",
	"Removes 1 item 'val' from the data stack "
	"and stores 'val' into register R6",

	MASK_K,
	"R7!",		"( val -- )",
	"Removes 1 item 'val' from the data stack "
	"and stores 'val' into register R7",

	MASK_K,
	"R8!",		"( val -- )",
	"Removes 1 item 'val' from the data stack "
	"and stores 'val' into register R8",

	MASK_K,
	"R9!",		"( val -- )",
	"Removes 1 item 'val' from the data stack "
	"and stores 'val' into register R9",

	MASK_K,
	"sign",		"( n -- SIGN(n) )",
	"return sign of 'n'. If n is negative this instruction "
	"returns -1. If n is positive this instruction returns 1."
	"If n is 0, returns 0.",

	MASK_K,
	"pack2",	"( a b -- n )",
	"combines 2 32-bit integers into a single value 'n'",

	MASK_K,
	"unpack2",	"( n -- a b )",
	"does the opposite of pack2.",

	MASK_K,
	"pack4",	"( a b c d -- n )",
	"combines 4 16-bit integers into a single value 'n'",

	MASK_K,
	"unpack4",	"( n -- a b c d )",
	"does the opposite of pack4.",

	MASK_C,
	"OMOVE",	"(x y -- r)",
	"Move organism. Organism will move in the direction "
	"specified by (x, y). These coordinates are normalized, "
	"so that (-20, 15) maps to a direction of (-1, 1). "
	"r is 1 on success, else r is 0.", 

	MASK_C,
	"CMOVE",	"(x y -- r)",
	"Move Cell. The cell executing this instruction "
	"will be moved relative to the organism. The coordinates "
	"are normalized to (8, -9) becomes (1, -1). "
	"Cell must ensure that the organism is still connected "
	"after the move. "
	"On success r is 1, else r is 0.",

	MASK_C,
	"ROTATE",	"(n -- r)",
	"rotate organism. "
	"rotate in 90 degree units. positive values are "
	"clockwise rotations, and negative values are "
	"counter-clockwise. On success r is 1, else r is 0.",

	MASK_C,
	"EAT",		"(x y -- e)",
	"The cell executing this instruction will eat stuff at "
	"the (x, y) offset. The three things that can be eaten "
	"are Organic material, Spores, and Cells from other organisms. "
	"'e' will the amount of energy we gained by eating.",

	MASK_C,
	"MAKE-SPORE",	"(x y e -- s)",
	"A spore is created at the normalized coordinates (x, y). The "
	"spore will be given energy 'e'. Normalized coordinates mean "
	"that (-3, 0) becomes (-1, 0). s is set to 0 on failure. "
	"s is set to 1 if we created a spore. s is set to -1 if we "
	"fertilized an existing spore.",

	MASK_C,
	"GROW",		"(x y -- r)",
	"A new cell is added to the organism. The coordinates (x, y) are "
	"normalized, meaning (123, -999) becomes (1, -1). The new cell "
	"inherits the all the execution context of the cell that executed "
	"this instruction. returns 1=parent cell, -1=new cell, 0=failure.",

	MASK_C,
	"LOOK",		"(x y -- what where)",
	"Look in direction (x, y) and return 'what' was seen, and how "
	"far away it was 'where'.",

	MASK_C,
	"WHAT",	"(x y -- what)",
	"what was seen in the (x, y) direction. 'what' will be: 0=nothing, 1=cell, 2=spore, "
	"4=organic, 8=barrier. Coordinates are normalized.",

	MASK_C,
	"WHERE",	"(x y -- distance)",
	"Returns distance to how far away something seen in the (x, y) direction. "
	"Coordinates are normalized. A distance of 1 means something is toching us. "
	"A distance of 0 means nothing was seen in this direction.",

	MASK_C,
	"NEAREST",	"(mask -- x y)",
	"Returns the (x,y) coordinates for the vision data with the SMALLEST distance "
	"that matches mask. 'mask' is an OR'ing of 'what' values."
	"If nothing matches (0,0) is returned.",

	MASK_C,
	"FARTHEST",	"(mask -- x y)",
	"Returns the (x,y) coordinates for the vision data with the LARGEST distance "
	"that matches mask. 'mask' is an OR'ing of 'what' values. "
	"If nothing matches (0,0) is returned.",

	MASK_C,
	"NEAREST2",	"(mask dist -- x y)",
	"Returns the (x,y) coordinates for the vision data with the SMALLEST distance "
	"that matches mask. 'mask' is an OR'ing of 'what' values. 'dist' limits the search "
	"radius. If nothing matches (0,0) is returned.",

	MASK_C,
	"FARTHEST2",	"(mask dist -- x y)",
	"Returns the (x,y) coordinates for the vision data with the LARGEST distance "
	"that matches mask. 'mask' is an OR'ing of 'what' values. 'dist' limits the search "
	"radius. If nothing matches (0,0) is returned.",

	MASK_C,
	"MOOD",		"(x y -- mood)",
	"We examine one of our own cells at offset (x, y) and return the MOOD "
	"of that cell. If (x, y) is invalid 0 is returned.",

	MASK_C,
	"MOOD!",	"(m -- )",
	"The cell executing this instruction will set its mood variable to 'm'.",

	MASK_C,
	"BROADCAST",	"(m -- )",
	"The message 'm' will be placed in the message buffer for every cell "
	"in our organism.",

	MASK_C,
	"SEND",		"(m x y -- )",
	"The message 'm' will be placed in the message buffer for the cell "
	"indicated by the vector (x, y). Any cell offset can be speficied.",

	MASK_C,
	"RECV",	"( -- m)",
	"Our message buffer 'm' is placed on top of the data stack.",

	MASK_C,
	"ENERGY",	"( -- e)",
	"Get our energy 'e' amount on top of the data stack.",

	MASK_C,
	"AGE",		"( -- a)",
	"Get our age 'a' on top of the data stack.",

	MASK_C,
	"NUM-CELLS",	"( -- n)",
	"Get the number of cells in our organism 'n' on top of the data stack.",

	MASK_C,
	"HAS-NEIGHBOR",	"(x y -- s)",
	"check if we have a neighboring cell at the normalized coordinates (x, y). "
	"s is set to 1 if there is a neighbor, else s is set to 0.",

	/*
	 * These are the special instructions
	 * for the FIND feature
	 */

	MASK_F,
	"ID", "( -- id)",
	"the organism ID.",

	MASK_F,
	"PARENT1", "( -- parent1)",
	"the parent1 ID.",

	MASK_F,
	"PARENT2", "( -- parent2)",
	"the parent2 ID.",

	MASK_F,
	"STRAIN", "( -- strain)",
	"the strain number.",

	MASK_F,
	"ENERGY", "( -- e)",
	"the energy.",

	MASK_F,
	"GENERATION", "( -- g)",
	"the generation.",

	MASK_F,
	"NUM-CELLS", "( -- n)",
	"the number of cells that the organism consists of.",

	MASK_F,
	"AGE", "( -- n)",
	"the age of the organism.",

	MASK_F,
	"NCHILDREN", "( -- n)",
	"the number of living children.",

	MASK_F,
	"EXECUTING", "(cb -- bool)",
	"Does organism have a cell currently executing inside code block 'cb'?",

	MASK_F,
	"NUM-CB", "( -- n)",
	"the total number of code blocks the organism has.",

	MASK_F,
	"NUM-DEAD", "( -- n)",
	"the number of cells that have just dies (colored red).",

	MASK_F,
	"MAX-ENERGY", "( -- e)",
	"Constant: for all organisms return the MAXIMUM energy amount",

	MASK_F,
	"MIN-ENERGY", "( -- e)",
	"Constant: for all organisms return the MINIMUM energy amount",

	MASK_F,
	"AVG-ENERGY", "( -- e)",
	"Constant: for all organisms return the AVERAGE energy amount",


	MASK_F,
	"MAX-AGE", "( -- n)",
	"Constant: for all organisms return the MAXIMUM age.",

	MASK_F,
	"MIN-AGE", "( -- n)",
	"Constant: for all organisms return the MINIMUM age.",

	MASK_F,
	"AVG-AGE", "( -- n)",
	"Constant: for all organisms return the AVERAGE age.",

	MASK_F,
	"MAX-NUM-CELLS", "( -- n)",
	"Constant: for all organisms return the MAXIMUM number of cells an organism has.",

};


// KforthInstructionDialog dialog

IMPLEMENT_DYNAMIC(KforthInstructionDialog, CDialog)

KforthInstructionDialog::KforthInstructionDialog(CWnd* pParent /*=NULL*/)
	: CDialog(KforthInstructionDialog::IDD, pParent)
{
	m_mask = MASK_K | MASK_C;
	m_insert_button = FALSE;
	m_goto = "";
}

KforthInstructionDialog::~KforthInstructionDialog()
{
}

CString KforthInstructionDialog::GetInstruction()
{
	if( m_item < 0 || m_item >= sizeof(KFI)/sizeof(KFI[0]) ) {
		return CString("");
	} else {
		return CString(KFI[m_item].instruction);
	}
}

void KforthInstructionDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	if( ! pDX->m_bSaveAndValidate ) {
		LoadList();
	}

	DDX_Text(pDX, IDC_IKI_DESC, m_description);
	DDX_LBIndex(pDX, IDC_IKI_LIST, m_lbitem);
}

//
// Populate list box with all the KFORTH
// instructions.
//
void KforthInstructionDialog::LoadList()
{
	CListBox *pLB = (CListBox*) GetDlgItem(IDC_IKI_LIST);
	char buf[1000];
	int i, j;

	if( pLB->GetCount() > 0 )
		return;

	pLB->ResetContent();

	m_item = -1;
	m_lbitem = -1;
	j = 0;
	for(i=0; i < sizeof(KFI)/sizeof(KFI[0]); i++) {
		if( KFI[i].mask & m_mask ) {
			sprintf(buf, "%-15s; %s", KFI[i].instruction, KFI[i].comment);

			pLB->InsertString(-1, buf);

			if( stricmp(KFI[i].instruction, m_goto) == 0 ) {
				m_item = i;
				m_lbitem = j;
			}

			j++;
		}
	}

	if( m_lbitem < 0 )  {
		return;
	}

	pLB->SetCurSel(m_lbitem);
	m_description = CString(KFI[m_item].instruction)
			+ CString("      ; ")
			+ CString(KFI[m_item].comment)
			+ CString("\r\n")
			+ CString("\r\n")
			+ CString(KFI[m_item].description);
}

void KforthInstructionDialog::InsertInstruction()
{
	if( m_insert_button ) {
		EndDialog(IDOK);
	}
}

void KforthInstructionDialog::SelectInstruction()
{
	CListBox *pLB = (CListBox*) GetDlgItem(IDC_IKI_LIST);
	int i, j;
	int n;

	n = pLB->GetCurSel();

	if( n == LB_ERR )
		return;

	if( n < 0 || n >= sizeof(KFI)/sizeof(KFI[0]) )
		return;

	m_lbitem = n;

	//
	// Map 'm_item' to the actual item in the KFI list, not
	// the reduced list shown to the user.
	//
	m_item = -1; // set to nothing
	j = 0;
	for(i=0; i < sizeof(KFI)/sizeof(KFI[0]); i++) {
		if( KFI[i].mask & m_mask ) {
			if( n == j ) {
				m_item = i;
				break;
			}
			j++;
		}
	}

	ASSERT( m_item != -1 );

	m_description = CString(KFI[m_item].instruction)
				+ CString("      ; ")
				+ CString(KFI[m_item].comment)
				+ CString("\r\n")
				+ CString("\r\n")
				+ CString(KFI[m_item].description);

	UpdateData(false);
}

void KforthInstructionDialog::show_instruction_set(int iset)
{
	switch(iset) {
	case IS_INTERPRETER:
		m_mask = MASK_K | MASK_C;
		break;

	case IS_CELL:
		m_mask = MASK_K | MASK_C;
		break;

	case IS_FIND:
		m_mask = MASK_K | MASK_F;
		break;

	default:
		ASSERT(0);
	}
}

void KforthInstructionDialog::insert_button(bool flag)
{
	m_insert_button = flag;
}

void KforthInstructionDialog::go_to(CString str)
{
	m_goto = str;
}

BOOL KforthInstructionDialog::OnInitDialog()
{
	CButton *pBT;

	CDialog::OnInitDialog();

	pBT = (CButton*) GetDlgItem(IDOK);

	if( m_insert_button ) {
		pBT->ShowWindow(SW_SHOW);
	} else {
		pBT->ShowWindow(SW_HIDE);
	}

	return TRUE;
}


BEGIN_MESSAGE_MAP(KforthInstructionDialog, CDialog)
	ON_LBN_DBLCLK(IDC_IKI_LIST, InsertInstruction)
	ON_LBN_SELCHANGE(IDC_IKI_LIST, SelectInstruction)
END_MESSAGE_MAP()


// KforthInstructionDialog message handlers
