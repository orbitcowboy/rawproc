#include "PicProcessor.h"
#include "PicProcessorSharpen.h"
#include "PicProcPanel.h"
#include "FreeImage.h"
#include "FreeImage16.h"
#include "wxTouchSlider.h"
#include "util.h"

#include <wx/time.h> 

class SharpenPanel: public PicProcPanel
{
	public:
		SharpenPanel(wxPanel *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT).Expand();
			wxArrayString mode;
			mode.Add("plain");
			mode.Add("edgemask");
			wxArrayString p = split(params,",");
			b->Add(new wxStaticText(this,-1, "edge threshold", wxDefaultPosition, wxSize(200,20)),  flags);
			thresholdedit = new wxTextCtrl(this, wxID_ANY, p[1], wxDefaultPosition, wxSize(100,20),wxTE_PROCESS_ENTER);
			b->Add(thresholdedit, flags);
					
			modeselect = new wxRadioBox (this, wxID_ANY, "Sharpen Mode", wxDefaultPosition, wxSize(100,100),  mode, 1, wxRA_SPECIFY_COLS);
			if (p.size() >=2) {
				for (int i=0; i<mode.size(); i++) {
					if (p[0] == mode[i]) modeselect->SetSelection(i);
				}
			}
			b->Add(modeselect, flags);	
			b->Add(new wxButton(this,-1, "Apply", wxDefaultPosition, wxSize(200,30)), flags);
			SetSizerAndFit(b);
			b->Layout();
			Refresh();
			Update();
			SetFocus();
			Bind(wxEVT_BUTTON,&SharpenPanel::paramChanged, this);
	
		}

		~SharpenPanel()
		{
			thresholdedit->~wxTextCtrl();
			modeselect->~wxRadioBox();
		}

		void paramChanged(wxCommandEvent& event)
		{
			q->setParams(wxString::Format("%s,%s",modeselect->GetString(modeselect->GetSelection()),thresholdedit->GetValue()));
			event.Skip();
		}


	private:

		wxTextCtrl *thresholdedit;
		wxRadioBox *modeselect;

};


PicProcessorSharpen::PicProcessorSharpen(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters): PicProcessor(name, command,  tree, display, parameters) 
{
	//p->DestroyChildren();
	//r = new BlankPanel(p,this,c);
	showParams();
}

void PicProcessorSharpen::showParams()
{
	if (!m_parameters) return;
	m_parameters->DestroyChildren();
	//r = new BlankPanel(m_parameters, this, c);
	r = new SharpenPanel(m_parameters, this, c);
}


bool PicProcessorSharpen::processPic() {
	wxString msg = "";
	long t;	

	double sharpen[3][3] =  
	{ 
		 0.0, -1.0,  0.0, 
		-1.0,  5.0, -1.0, 
		 0.0, -1.0,  0.0 
	}; 
	double edge[3][3] =  
	{ 
		 0.0,  1.0, 0.0, 
		 1.0, -4.0, 1.0, 
		 0.0,  1.0, 0.0 
	}; 
	double blur[3][3] =  
	{ 
		 1.0, 1.0,  1.0, 
		 1.0, 1.0,  1.0, 
		 1.0, 1.0,  1.0 
	}; 
	double betterblur[3][3] =  
	{ 
		 0.0, 0.2, 0.0, 
		 0.2, 0.2, 0.2, 
		 0.0, 0.2, 0.0 
	}; 
	m_tree->SetItemBold(GetId(), true);
	((wxFrame*) m_parameters->GetParent())->SetStatusText("sharpen...");

	bool result = true;

	wxArrayString cp = split(getParams(),",");
	int threshold =  atoi(cp[1]);

	if (cp[0] == "plain") {
		((wxFrame*) m_parameters->GetParent())->SetStatusText("sharpen...");
		t = wxGetLocalTime();
		FIBITMAP *prev = dib;
		dib = FreeImage_3x3Convolve16(getPreviousPicProcessor()->getProcessedPic(), sharpen); 
		if (prev) FreeImage_Unload(prev);
		msg.Append(wxString::Format("sharpen: %ld sec\n", wxGetLocalTime()-t));
	}
	else {

		FIBITMAP *prev = dib;
		//FIBITMAP *orig = getPreviousPicProcessor()->getProcessedPic();
		((wxFrame*) m_parameters->GetParent())->SetStatusText("find edges...");
		FIBITMAP *edgedib = FreeImage_3x3Convolve16(getPreviousPicProcessor()->getProcessedPic(), edge);
		((wxFrame*) m_parameters->GetParent())->SetStatusText("make mask...");
		FIBITMAP *maskdib = FreeImage_ConvertTo8Bits(edgedib);
		FreeImage_Unload(edgedib);
		((wxFrame*) m_parameters->GetParent())->SetStatusText(wxString::Format("sharpen, threshold %d...", threshold));
		dib = FreeImage_3x3Convolve16(getPreviousPicProcessor()->getProcessedPic(), sharpen, maskdib, threshold);
		FreeImage_Unload(maskdib);
		if (prev) FreeImage_Unload(prev);
		//orig = NULL;
	}


	//put in every processPic()...
	if (m_tree->GetItemState(GetId()) == 1) m_display->SetPic(dib);
	m_tree->SetItemBold(GetId(), false);
	wxTreeItemId next = m_tree->GetNextSibling(GetId());
	if (next.IsOk()) {
		PicProcessor * nextitem = (PicProcessor *) m_tree->GetItemData(next);
		nextitem->processPic();
	}
	m_tree->SetItemBold(GetId(), false);
	((wxFrame*) m_parameters->GetParent())->SetStatusText("");
	//wxMessageBox(msg);
	return result;
}



