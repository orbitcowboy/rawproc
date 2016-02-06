#ifndef __PICPROCESSOR_H__
#define __PICPROCESSOR_H__

#ifndef WX_PRECOMP
	#include <wx/wx.h>
	#include <wx/frame.h>
#else
	#include <wx/wxprec.h>
#endif

#include <wx/image.h>
#include <wx/treectrl.h>
#include <wx/sizer.h>
#include <wx/event.h>

#include "PicPanel.h"
#include "PicProcPanel.h"
#include "FreeImage.h"

class PicProcPanel;

class PicPanel;

class PicProcessor: public wxTreeItemData
{

	public:
		PicProcessor(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters, FIBITMAP *startpic=NULL);
		//PicProcessor(PicProcessor * copyof);
		~PicProcessor();
		virtual bool processPic();
		wxString getCommand();
		wxString getParams();
		wxString getName();
		virtual void showParams();
		PicProcessor *getPreviousPicProcessor();
		FIBITMAP *getProcessedPic();
		PicPanel *getDisplay();
		wxTreeCtrl *getCommandTree();
		virtual void displayProcessedPic();
		virtual void setParams(wxString params);

//	private:
		FIBITMAP *dib;
		PicPanel *m_display;
		
		wxTreeCtrl *m_tree;
		wxString n, c;

		wxPanel *m_parameters;
		PicProcPanel *r;
		

};

class BlankPanel: public PicProcPanel
{
	public:
		BlankPanel(wxPanel *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			panel = new wxPanel(this);
			b->Add(panel, 1, wxALIGN_LEFT, 10);
			SetSizerAndFit(b);
		}

		~BlankPanel()
		{
			panel->~wxPanel();
		}

	private:
		wxPanel *panel;

};



#endif
