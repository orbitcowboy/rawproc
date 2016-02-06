#include "PicProcessor.h"
#include "PicProcessorSharpen.h"
#include "PicProcPanel.h"
#include "FreeImage.h"
#include "FreeImage16.h"
#include "wxTouchSlider.h"
#include "util.h"

/*
class SharpenPanel: public PicProcPanel
{
	public:
		SharpenPanel(wxPanel *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT).Expand();
			wxArrayString algos;
			algos.Add("box");
			algos.Add("bilinear");
			algos.Add("bspline");
			algos.Add("bicubic");
			algos.Add("catmullrom");
			algos.Add("lanczos3");
			wxArrayString p = split(params,",");
			b->Add(new wxStaticText(this,-1, "width (0 for original aspect)", wxDefaultPosition, wxSize(200,20)),  flags);
			widthedit = new wxTextCtrl(this, wxID_ANY, p[0], wxDefaultPosition, wxSize(100,20),wxTE_PROCESS_ENTER);
			b->Add(widthedit, flags);
			b->Add(new wxStaticText(this,-1, "height (0 for original aspect)", wxDefaultPosition, wxSize(200,20)), flags);
			heightedit = new wxTextCtrl(this, wxID_ANY, p[1], wxDefaultPosition, wxSize(100,20),wxTE_PROCESS_ENTER);
			b->Add(heightedit, flags);		
			algoselect = new wxRadioBox (this, wxID_ANY, "Resize Algorithm", wxDefaultPosition, wxSize(100,200),  algos, 1, wxRA_SPECIFY_COLS);
			if (p.size() >=3) {
				for (int i=0; i<algos.size(); i++) {
					if (p[2] == algos[i]) algoselect->SetSelection(i);
				}
			}
			b->Add(algoselect, flags);	
			b->Add(new wxButton(this,-1, "Apply", wxDefaultPosition, wxSize(200,30)), flags);
			SetSizerAndFit(b);
			b->Layout();
			Refresh();
			Update();
			SetFocus();
			Bind(wxEVT_BUTTON,&ResizePanel::paramChanged, this);
	
		}

		~SharpenPanel()
		{
			widthedit->~wxTextCtrl();
			heightedit->~wxTextCtrl();
			algoselect->~wxRadioBox();
		}

		void paramChanged(wxCommandEvent& event)
		{
			q->setParams(wxString::Format("%s,%s,%s",widthedit->GetValue(),heightedit->GetValue(),algoselect->GetString(algoselect->GetSelection())));
			event.Skip();
		}


	private:

		wxTextCtrl *widthedit, *heightedit;
		wxRadioBox *algoselect;

};
*/

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
	r = new BlankPanel(m_parameters, this, c);
}


bool PicProcessorSharpen::processPic() {
	wxString algo = "";
	double kernel[3][3] =  
	{ 
		 0.0, -1.0,  0.0, 
		-1.0,  5.0, -1.0, 
		 0.0, -1.0,  0.0 
	}; 
	m_tree->SetItemBold(GetId(), true);
	((wxFrame*) m_parameters->GetParent())->SetStatusText("sharpen...");

	bool result = true;
	FIBITMAP *prev = dib;

	dib = FreeImage_3x3Convolve16(getPreviousPicProcessor()->getProcessedPic(), kernel); 

	if (prev) FreeImage_Unload(prev);

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
	return result;
}



