///-----------------------------------------------------------------
///
/// @file      rawprocFrm.cpp
/// @author    Glenn
/// Created:   11/18/2015 7:04:06 PM
/// @section   DESCRIPTION
///            rawprocFrm class implementation
///
///------------------------------------------------------------------

#include "rawprocFrm.h"
#include <wx/filedlg.h>
#include <wx/bitmap.h>
#include <wx/artprov.h>
#include <wx/image.h>
#include <wx/imaglist.h>
#include <wx/clipbrd.h>
#include <wx/aboutdlg.h> 
#include <wx/fileconf.h>
#include <wx/stdpaths.h>

#include "PicProcessorGamma.h"
#include "PicProcessorBright.h"
#include "PicProcessorContrast.h"
#include "PicProcessorSaturation.h"
#include "PicProcessorShadow.h"
#include "PicProcessorHighlight.h"
#include "PicProcessorCurve.h"
#include "PicProcessorGray.h"
#include "PicProcessorCrop.h"
#include "PicProcessorResize.h"
#include "PicProcessorBlackWhitePoint.h"
#include "PicProcessorSharpen.h"
#include "PicProcessorRotate.h"
#include "PicProcessorDenoise.h"
#include "myFileSelector.h"
#include "util.h"
#include "lcms2.h"
#include <omp.h>
#include <exception>

#include "unchecked.xpm"
#include "checked.xpm"

wxString version = "0.5Dev";

//Do not add custom headers between
//Header Include Start and Header Include End
//wxDev-C++ designer will remove them
////Header Include Start
////Header Include End

//----------------------------------------------------------------------------
// rawprocFrm
//----------------------------------------------------------------------------
//Add Custom Events only in the appropriate block.
//Code added in other places will be removed by wxDev-C++
////Event Table Start
BEGIN_EVENT_TABLE(rawprocFrm,wxFrame)
	////Manual Code Start
	EVT_TREE_SEL_CHANGED(ID_COMMANDTREE,rawprocFrm::CommandTreeSelChanged)
	////Manual Code End
	
	EVT_CLOSE(rawprocFrm::OnClose)
	EVT_MENU(ID_MNU_OPEN, rawprocFrm::Mnuopen1003Click)
	EVT_MENU(ID_MNU_OPENSOURCE, rawprocFrm::Mnuopensource1004Click)
	EVT_MENU(ID_MNU_SAVE, rawprocFrm::Mnusave1009Click)
	EVT_MENU(ID_MNU_EXIT, rawprocFrm::MnuexitClick)
	EVT_MENU(ID_MNU_GAMMA, rawprocFrm::Mnugamma1006Click)
	EVT_MENU(ID_MNU_BRIGHT, rawprocFrm::Mnubright1007Click)
	EVT_MENU(ID_MNU_CONTRAST, rawprocFrm::Mnucontrast1008Click)
	EVT_MENU(ID_MNU_SATURATION, rawprocFrm::MnusaturateClick)
	EVT_MENU(ID_MNU_SHADOW, rawprocFrm::MnuShadow1015Click)
	EVT_MENU(ID_MNU_HIGHLIGHT, rawprocFrm::MnuHighlightClick)
	EVT_MENU(ID_MNU_CURVE, rawprocFrm::Mnucurve1010Click)
	EVT_MENU(ID_MNU_GRAY, rawprocFrm::MnuGrayClick)
	EVT_MENU(ID_MNU_CROP, rawprocFrm::MnuCropClick)
	EVT_MENU(ID_MNU_RESIZE, rawprocFrm::MnuResizeClick)
	EVT_MENU(ID_MNU_BLACKWHITEPOINT, rawprocFrm::MnuBlackWhitePointClick)
	EVT_MENU(ID_MNU_SHARPEN, rawprocFrm::MnuSharpenClick)
	EVT_MENU(ID_MNU_ROTATE, rawprocFrm::MnuRotateClick)
	EVT_MENU(ID_MNU_DENOISE, rawprocFrm::MnuDenoiseClick)
	EVT_MENU(ID_MNU_Cut,rawprocFrm::MnuCut1201Click)
	EVT_MENU(ID_MNU_Copy,rawprocFrm::MnuCopy1202Click)
	EVT_MENU(ID_MNU_Paste,rawprocFrm::MnuPaste1203Click)
	EVT_MENU(ID_MNU_SHOWCOMMAND,rawprocFrm::MnuShowCommand1010Click)
	EVT_MENU(ID_MNU_ABOUT,rawprocFrm::MnuAbout1011Click)
	EVT_MENU(ID_MNU_VIEWHELP,rawprocFrm::MnuHelpClick)
	EVT_MENU(ID_MNU_PROPERTIES,rawprocFrm::MnuProperties)
	EVT_TREE_KEY_DOWN(ID_COMMANDTREE,rawprocFrm::CommandTreeKeyDown)
	//EVT_TREE_DELETE_ITEM(ID_COMMANDTREE, rawprocFrm::CommandTreeDeleteItem)
	EVT_TREE_BEGIN_DRAG(ID_COMMANDTREE, rawprocFrm::CommandTreeBeginDrag)
	EVT_TREE_END_DRAG(ID_COMMANDTREE, rawprocFrm::CommandTreeEndDrag)
	EVT_TREE_STATE_IMAGE_CLICK(ID_COMMANDTREE, rawprocFrm::CommandTreeStateClick)
	EVT_TREE_SEL_CHANGING(ID_COMMANDTREE, rawprocFrm::CommandTreeSelChanging)
	EVT_TREE_ITEM_MENU(ID_COMMANDTREE, rawprocFrm::CommandTreePopup)
END_EVENT_TABLE()
////Event Table End

rawprocFrm::rawprocFrm(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &position, const wxSize& size, long style)
: wxFrame(parent, id, title, position, size, style)
{

	CreateGUIControls();
#if defined(_OPENMP)
	omp_set_dynamic(0);
#endif
	deleting = false;

	wxImageList *states;
        wxIcon icons[2];
        icons[0] = wxIcon(unchecked_xpm);
        icons[1] = wxIcon(checked_xpm);

	//diag = NULL;

        int width  = icons[0].GetWidth(),
            height = icons[0].GetHeight();

        // Make a state image list containing small icons
        states = new wxImageList(width, height, true);
	for ( size_t i = 0; i < WXSIZEOF(icons); i++ )
            states->Add(icons[i]);
	commandtree->AssignStateImageList(states);

	configfile = "(none)";

	help.UseConfig(wxConfig::Get());
	bool ret;
	help.SetTempDir(wxStandardPaths::Get().GetTempDir());
	wxFileName helpfile( wxStandardPaths::Get().GetExecutablePath());
	helpfile.SetFullName("rawprocdoc.zip");
	ret = help.AddBook(wxFileName(helpfile));
	if (! ret)
		wxMessageBox(wxString::Format("Failed adding %s",helpfile.GetFullPath()));
}

rawprocFrm::~rawprocFrm()
{

}

void rawprocFrm::CreateGUIControls()
{
	//Do not add custom code between
	//GUI Items Creation Start and GUI Items Creation End
	//wxDev-C++ designer will remove them.
	//Add the custom code before or after the blocks
	////GUI Items Creation Start

	WxMenuBar1 = new wxMenuBar();
	wxMenu *ID_MNU_FILEMnu_Obj = new wxMenu();
	ID_MNU_FILEMnu_Obj->Append(ID_MNU_OPEN, _("Open..."), _(""), wxITEM_NORMAL);
	ID_MNU_FILEMnu_Obj->Append(ID_MNU_OPENSOURCE, _("Open Source..."), _(""), wxITEM_NORMAL);
	ID_MNU_FILEMnu_Obj->Append(ID_MNU_SAVE, _("Save..."), _(""), wxITEM_NORMAL);
	ID_MNU_FILEMnu_Obj->AppendSeparator();
	ID_MNU_FILEMnu_Obj->Append(ID_MNU_EXIT, _("Exit"), _(""), wxITEM_NORMAL);
	WxMenuBar1->Append(ID_MNU_FILEMnu_Obj, _("File"));

	wxMenu *ID_MNU_EDITMnu_Obj = new wxMenu();
	ID_MNU_EDITMnu_Obj->Append(ID_MNU_Cut,_("Cut"), _(""), wxITEM_NORMAL);
	ID_MNU_EDITMnu_Obj->Append(ID_MNU_Copy,_("Copy"), _(""), wxITEM_NORMAL);
	ID_MNU_EDITMnu_Obj->Append(ID_MNU_Paste,_("Paste"), _(""), wxITEM_NORMAL);
	ID_MNU_EDITMnu_Obj->AppendSeparator();
	ID_MNU_EDITMnu_Obj->Append(ID_MNU_PROPERTIES,_("Properties..."), _(""), wxITEM_NORMAL);
	WxMenuBar1->Append(ID_MNU_EDITMnu_Obj, _("Edit"));

	
	wxMenu *ID_MNU_ADDMnu_Obj = new wxMenu();
	
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_BLACKWHITEPOINT,	_("Black/White Point"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_BRIGHT,	_("Bright"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_CONTRAST,	_("Contrast"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_CROP,		_("Crop"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_CURVE,		_("Curve"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_DENOISE,	_("Denoise"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_GAMMA,		_("Gamma"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_GRAY,		_("Gray"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_HIGHLIGHT,	_("Highlight"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_RESIZE,	_("Resize"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_ROTATE,	_("Rotate"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_SATURATION,	_("Saturation"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_SHADOW,	_("Shadow"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_SHARPEN,	_("Sharpen"), _(""), wxITEM_NORMAL);
	
	
	WxMenuBar1->Append(ID_MNU_ADDMnu_Obj, _("Add"));
	
	wxMenu *ID_MNU_HELPMnu_Obj = new wxMenu();
	ID_MNU_HELPMnu_Obj->Append(ID_MNU_VIEWHELP, _("View Help..."), _(""), wxITEM_NORMAL);
	ID_MNU_HELPMnu_Obj->Append(ID_MNU_SHOWCOMMAND, _("Show Command..."), _(""), wxITEM_NORMAL);
	ID_MNU_HELPMnu_Obj->Append(ID_MNU_ABOUT, _("About..."), _(""), wxITEM_NORMAL);
	WxMenuBar1->Append(ID_MNU_HELPMnu_Obj, _("Help"));
	SetMenuBar(WxMenuBar1);

	WxStatusBar1 = new wxStatusBar(this, ID_WXSTATUSBAR1);
	int widths[2] = {-1, 100};
	WxStatusBar1->SetFieldsCount (2, widths);

	SetStatusBar(WxStatusBar1);
	SetTitle(_("rawproc"));
	SetIcon(wxNullIcon);
	SetSize(0,0,1200,750);
	Center();
	
	////GUI Items Creation End
	
	//wxAuiPaneInfo pinfo = wxAuiPaneInfo().Left().CloseButton(false).Dockable(false).Floatable(false);   //.MinSize(280,200)
	wxAuiPaneInfo pinfo = wxAuiPaneInfo().Left().CloseButton(false);
	mgr.SetManagedWindow(this);
	commandtree = new wxTreeCtrl(this, ID_COMMANDTREE, wxDefaultPosition, wxSize(280,200), wxTR_DEFAULT_STYLE);
	pic = new PicPanel(this);

	//wxPanel parms = new wxPanel(this, -1, wxDefaultPosition, wxSize(285,200),wxVSCROLL);
	//parameters = new wxScrolled<wxPanel>(parms);
	parameters = new myParameters(this, -1, wxDefaultPosition, wxSize(285,200));
	//parameters = new wxPanel(this, -1, wxDefaultPosition, wxSize(285,200));
	parameters->SetMinSize(wxSize(285,200));
	parameters->SetMaxSize(wxSize(1200,750));
	parameters->SetAutoLayout(true); 
	//parameters->SetBackgroundColour(*wxBLUE);
	//preview  = new wxPanel(this, -1, wxDefaultPosition, wxSize(285,200));

	mgr.AddPane(pic, wxCENTER);
	mgr.AddPane(commandtree, pinfo.Caption(wxT("Commands")).Position(0));
	mgr.AddPane(parameters, pinfo.Caption(wxT("Parameters")).Position(1).GripperTop());
	//mgr.AddPane(preview, pinfo.Caption(wxT("Preview")).Position(2).GripperTop());

	mgr.Update();
}

void rawprocFrm::OnClose(wxCloseEvent& event)
{
	if ( help.GetFrame() ) // returns NULL if no help frame active
		help.GetFrame()->Close(true);
	// now we can safely delete the config pointer
	event.Skip();
	delete wxConfig::Set(NULL);
	mgr.UnInit();
	Destroy();
}

void rawprocFrm::MnuexitClick(wxCommandEvent& event)
{

	mgr.UnInit();
	Destroy();
}

void rawprocFrm::SetThumbMode(int mode)
{
	pic->SetThumbMode(mode);
}

PicProcessor * rawprocFrm::GetItemProcessor(wxTreeItemId item)
{
	if (item.IsOk())
		return (PicProcessor *) commandtree->GetItemData(item);
	else
		wxMessageBox("bad item");
}



//ToDo: Get rid of MoveBefore, MoveAfter...

/*
bool rawprocFrm::MoveBefore(wxTreeItemId item)
{
    bool result = false;

	wxString n,c;
	PicProcessor * prevpic = (PicProcessor *) commandtree->GetItemData(item);
	wxString name = prevpic->getName();
	wxString params = prevpic->getParams();
    wxTreeItemId moved, prev, before;
    prev = commandtree->GetPrevSibling(item);
    if (prev.IsOk()) before = commandtree->GetPrevSibling(prev);
    if (before.IsOk()) {          
        moved = commandtree->InsertItem(commandtree->GetItemParent(item), before, name, -1, -1, AddItem(name,params));
    }
    else {
        if (prev.IsOk() && prev != commandtree->GetItemParent(item)) {
            moved = commandtree->InsertItem(commandtree->GetItemParent(item), 0, name, -1, -1, AddItem(name,params));
        }
    }
    commandtree->Delete(item);
    if (moved.IsOk()) {
        commandtree->SelectItem(moved);
	commandtree->SetItemState(moved,0);
        
        result = true;
    }
    return result;
}

bool rawprocFrm::MoveAfter(wxTreeItemId item)
{
    bool result = false;
	PicProcessor * prevpic = (PicProcessor *) commandtree->GetItemData(item);
	wxString name = prevpic->getName();
	wxString params = prevpic->getParams();
        wxTreeItemId after = commandtree->GetNextSibling(item);
	   if(after.IsOk()) {
            wxTreeItemId moved = commandtree->InsertItem(commandtree->GetItemParent(after), after, name, -1, -1, AddItem(name,params));
            commandtree->SelectItem(moved);
            commandtree->Delete(item);
            result = true;
        }
    return result;
}
*/

void rawprocFrm::EXIFDialog(wxTreeItemId item)
{
	wxString exif="";
	gImage dib = ((PicProcessor *) commandtree->GetItemData(item))->getProcessedPic();

	exif.Append(wxString::Format("Width: %d Height: %d\n\n",dib.getWidth(), dib.getHeight()));

	std::map<std::string,std::string> e =  dib.getInfo();
	for (std::map<std::string,std::string>::iterator it=e.begin(); it!=e.end(); ++it)
		exif.Append(wxString::Format("%s: %s\n",it->first.c_str(),it->second.c_str()));
	char buff[4096];
	exif.Append("\n");
	exif.Append(dib.Stats().c_str());


	char *profile = dib.getProfile();
	unsigned profile_length = dib.getProfileLength();
	if (profile) {
		cmsHPROFILE icc = cmsOpenProfileFromMem(profile,profile_length);
		cmsUInt32Number n =  cmsGetProfileInfoASCII(icc, cmsInfoDescription, "en", "us", buff, 4096);
		exif.Append(wxString::Format("\nICC Profile: %s\n", wxString(buff)));
		cmsCloseProfile(icc);
	}
	else exif.Append("\nICC Profile: None\n");

	wxMessageBox(exif,"Image Information");
	
}


PicProcessor * rawprocFrm::AddItem(wxString name, wxString command)
{
	SetStatusText("");
	bool result = true;
	PicProcessor *p;

	if      (name == "gamma")      		p = new PicProcessorGamma("gamma",command, commandtree,  pic,  parameters);
	else if (name == "bright")     		p = new PicProcessorBright("bright",command, commandtree, pic, parameters);
	else if (name == "contrast")   		p = new PicProcessorContrast("contrast",command, commandtree, pic, parameters);
	else if (name == "shadow")     		p = new PicProcessorShadow("shadow",command, commandtree, pic, parameters);
	else if (name == "highlight")  		p = new PicProcessorHighlight("highlight",command, commandtree, pic, parameters);
	else if (name == "saturation") 		p = new PicProcessorSaturation("saturation",command, commandtree, pic, parameters);
	else if (name == "curve")		p = new PicProcessorCurve("curve",command, commandtree, pic, parameters);
	else if (name == "gray")       		p = new PicProcessorGray("gray",command, commandtree, pic, parameters);
	else if (name == "crop")       		p = new PicProcessorCrop("crop",command, commandtree, pic, parameters);
	else if (name == "resize")     		p = new PicProcessorResize("resize",command, commandtree, pic, parameters);
	else if (name == "blackwhitepoint")     p = new PicProcessorBlackWhitePoint("blackwhitepoint",command, commandtree, pic, parameters);
	else if (name == "sharpen")     	p = new PicProcessorSharpen("sharpen",command, commandtree, pic, parameters);
	else if (name == "rotate")		p = new PicProcessorRotate("rotate",command, commandtree, pic, parameters);
	else if (name == "denoise")		p = new PicProcessorDenoise("denoise",command, commandtree, pic, parameters);
	else result = NULL;
	p->processPic();
	if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId());
	Refresh();
	Update();

	return p;
}


void rawprocFrm::CommandTreeSetDisplay(wxTreeItemId item)
{
	SetStatusText("");
	if (!item.IsOk()) return;
	wxTreeItemIdValue cookie;
	wxTreeItemId root = commandtree->GetRootItem();
	if (root.IsOk()) commandtree->SetItemState(root,0);
	wxTreeItemId iter = commandtree->GetFirstChild(root, cookie);
	if (iter.IsOk()) {
		commandtree->SetItemState(iter,0);
		iter = commandtree->GetNextChild(root, cookie);
		while (iter.IsOk()) {
			commandtree->SetItemState(iter,0);
			iter = commandtree->GetNextChild(root, cookie);
		}
	}
	commandtree->SetItemState(item,1);
	((PicProcessor *) commandtree->GetItemData(item))->displayProcessedPic();
	Refresh();
	Update();
}

wxString rawprocFrm::AssembleCommand()
{
	SetStatusText("");
	wxString cmd = "rawproc-";
	cmd.Append(version);
	cmd.Append(" ");
	wxTreeItemIdValue cookie;
	wxTreeItemId root = commandtree->GetRootItem();
	wxString rootcmd = ((PicProcessor *)commandtree->GetItemData(root))->getCommand();
		cmd.Append(wxString::Format("%s ",rootcmd));
	wxTreeItemId iter = commandtree->GetFirstChild(root, cookie);
	if (iter.IsOk()) {
		cmd.Append(wxString::Format("%s ",((PicProcessor *)commandtree->GetItemData(iter))->getCommand()));
		iter = commandtree->GetNextChild(root, cookie);
		while (iter.IsOk()) {
			cmd.Append(wxString::Format("%s ",((PicProcessor *)commandtree->GetItemData(iter))->getCommand()));
			iter = commandtree->GetNextChild(root, cookie);
		}
	}

	return cmd;
}

void rawprocFrm::OpenFile(wxString fname, wxString params)
{
	filename.Assign(fname);
	sourcefilename.Clear();
	gImage *dib;
	GIMAGE_FILETYPE fif;
	fif = gImage::getFileType(fname.c_str());
	if (fif != FILETYPE_UNKNOWN) {

		commandtree->DeleteAllItems();

		wxString configparams;
		if (fif == FILETYPE_RAW) configparams = wxConfigBase::Get()->Read("input.raw.parameters","");
		if (fif == FILETYPE_JPEG) configparams = wxConfigBase::Get()->Read("input.jpeg.parameters","");
		if (fif == FILETYPE_TIFF) configparams = wxConfigBase::Get()->Read("input.tiff.parameters","");

		SetStatusText(wxString::Format("Loading file:%s params:%s",filename.GetFullName(), configparams));

		mark();
		//dib = new gImage(gImage::loadImageFile(fname.c_str(), (std::string) params.c_str()));
		dib = new gImage(gImage::loadImageFile(fname.c_str(), (std::string) configparams.c_str()));
		wxString loadtime = duration();
		if (dib->getWidth() == 0) {
			wxMessageBox(wxString::Format("Error: File %s not loaded successfully", filename.GetFullName()));
			SetStatusText("");
			return;
		}
		wxString flagstring(params.c_str());
		if ((wxConfigBase::Get()->Read("input.log","0") == "1") || (wxConfigBase::Get()->Read("input.load.log","0") == "1"))
			log(wxString::Format("tool=load,filename=%s,imagesize=%dx%d,time=%s",filename.GetFullName(),dib->getWidth(), dib->getHeight(),loadtime));

		PicProcessor *picdata = new PicProcessor(filename.GetFullName(), configparams, commandtree, pic, parameters, dib);
		picdata->showParams();
		picdata->processPic();
		CommandTreeSetDisplay(picdata->GetId());
		SetTitle(wxString::Format("rawproc: %s",filename.GetFullName()));
		SetStatusText("");
		SetStatusText("scale: fit",1);
		pic->SetScaleToWidth();
		pic->FitMode(true);
		wxString raw_default = wxConfigBase::Get()->Read("input.raw.default","");
		if ((fif == FILETYPE_RAW) & (raw_default != "")) {
			if (wxMessageBox(wxString::Format("Apply %s to raw file?",raw_default), "Confirm", wxYES_NO, this) == wxYES) {
				wxArrayString token = split(raw_default, " ");
				try {
					for (int i=0; i<token.GetCount(); i++) {
						wxArrayString cmd = split(token[i], ":");
						if (cmd.GetCount() == 2)
							AddItem(cmd[0], cmd[1]);
						else
							AddItem(cmd[0], "");
						wxSafeYield(this);
					}
				}
				catch (std::exception& e) {
					wxMessageBox(wxString::Format("Error: Adding gamma tool failed: %s",e.what()));
				}
			}
		}

		Refresh();
		Update();
		
		//wxSafeYield(this);
	}
	else {
		SetStatusText(wxString::Format("Loading %s failed.",filename.GetFullName() ));
	}
}

void rawprocFrm::OpenFileSource(wxString fname)
{

	filename.Assign(fname);
	sourcefilename.Clear();
	gImage *dib;
	wxString ofilename;
	wxString oparams = "";

	GIMAGE_FILETYPE fif;
	fif = gImage::getFileType(fname.c_str());

	if (fif != FILETYPE_UNKNOWN) {
		SetStatusText("Retrieving source script...");
		std::map<std::string,std::string> info =  gImage::getInfo(fname.c_str());

		if(info.find("ImageDescription") != info.end() && info["ImageDescription"].find("rawproc") != std::string::npos ) {
			wxString script = info["ImageDescription"];
			wxArrayString token = split(script, " ");
			
			if (token[1].Contains(":")) {
				wxArrayString fparams = split(token[1],":");
				if (fparams.GetCount() >1) {
					oparams = fparams[1];
				}
				ofilename = fparams[0];
			}
			else ofilename = token[1];
				
			if (token[0].Find("rawproc") == wxNOT_FOUND) {
				wxMessageBox(wxString::Format("Source script not found in %s, aborting Open Source.", filename.GetFullName().c_str()) );
			}
			else {
				SetStatusText(wxString::Format("Source script found, loading source file %s...",ofilename) );
				commandtree->DeleteAllItems();
				filename.Assign(ofilename);
				sourcefilename.Assign(fname);
				dib = new gImage(gImage::loadImageFile(ofilename.c_str(), (std::string) oparams.c_str()));
				if (dib->getWidth() == 0) {
					wxMessageBox(wxString::Format("Error: File %s load failed", ofilename));
					SetStatusText("");
					return;
				}

				PicProcessor *picdata = new PicProcessor(filename.GetFullName(), oparams, commandtree, pic, parameters, dib);
				picdata->processPic();
				CommandTreeSetDisplay(picdata->GetId());
				SetTitle(wxString::Format("rawproc: %s (%s)",filename.GetFullName().c_str(), sourcefilename.GetFullName().c_str()));
				SetStatusText("");
				SetStatusText("scale: fit",1);
				pic->SetScaleToWidth();
				pic->FitMode(true);
				for (int i=2; i<token.GetCount(); i++) {
					//SetStatusText(wxString::Format("Applying %s...",token[i]) );
					wxArrayString cmd = split(token[i], ":");					
					AddItem(cmd[0], cmd[1]);
					wxSafeYield(this);
				}
				SetStatusText("");
				pic->SetScaleToWidth();
				Refresh();
				Update();
			}
			
		}
		else {
			wxMessageBox(wxString::Format("No source script found in %s, aborting Open Source.",filename.GetFullName() ));
		}
	}
	else {
		wxMessageBox(wxString::Format("Loading %s failed, unknown file format.",filename.GetFullName() ));
	}
	SetStatusText("");
}


void rawprocFrm::CommandTreeStateClick(wxTreeEvent& event)
{
	CommandTreeSetDisplay(event.GetItem());
	Update();
	Refresh();
	event.Skip();
}


void rawprocFrm::CommandTreeSelChanging(wxTreeEvent& event)
{
	olditem = event.GetOldItem();
	//if ((PicProcessor *) commandtree->GetItemData(olditem))
		//((PicProcessor *) commandtree->GetItemData(olditem))->showParams(false);
}

void rawprocFrm::CommandTreeSelChanged(wxTreeEvent& event)
{
	SetStatusText("");
	wxTreeItemId item = event.GetItem();
	//wxTreeItemId item = commandtree->GetSelection();
	if (item.IsOk()) { 
		if ((PicProcessor *) commandtree->GetItemData(item))
			((PicProcessor *) commandtree->GetItemData(item))->showParams();
	}
	Update();
	Refresh();
	event.Skip();
	wxSafeYield(this);
}

void rawprocFrm::CommandTreeDeleteItem(wxTreeItemId item)
{
	wxTreeItemId prev, next, newitem;
	if (commandtree->GetItemParent(item).IsOk()) {  //not root
		prev = commandtree->GetPrevSibling(item);
		next = commandtree->GetNextSibling(item);
		if (!prev.IsOk()) prev = commandtree->GetRootItem();
		if (commandtree->GetItemState(item) == 1) CommandTreeSetDisplay(prev);
		if (next.IsOk())
			newitem = next;
		else
			newitem = prev;
		commandtree->SelectItem(newitem);
       		commandtree->Delete(item);
		if (newitem.IsOk()) {
			((PicProcessor *) commandtree->GetItemData(newitem))->showParams();
			((PicProcessor *) commandtree->GetItemData(newitem))->processPic();
		}
		deleting = true;
	}
}

void rawprocFrm::CommandTreeKeyDown(wxTreeEvent& event)
{
	SetStatusText("");
	//wxTreeItemId item, prev, next, newitem;
	switch (event.GetKeyCode()) {
        case 127:  //Delete
	case 8: //Backspace
		CommandTreeDeleteItem(commandtree->GetSelection());
        	break;
        //case 315: //Up Arrow
        //	MoveBefore(commandtree->GetSelection());
        //	break;
        //case 317: //Down Arrow
        //	MoveAfter(commandtree->GetSelection());
        //	break;
	case 102: //f
	case 70: //F - fit image to window
		pic->SetScaleToWidth();
		pic->FitMode(true);
		SetStatusText("scale: fit",1);
		break;
	case 67: //c - test cropmode
		//pic->ToggleCropMode();
		break;
	case 80: //p - process command
		WxStatusBar1->SetStatusText("processing...");
		((PicProcessor *) commandtree->GetItemData(commandtree->GetSelection()))->processPic();
		WxStatusBar1->SetStatusText("");
		break;
	case 116: //t
	case 84: //T - toggle display thumbnail
		pic->ToggleThumb();
		break;
    	}
	//wxMessageBox(wxString::Format("keycode: %d", event.GetKeyCode()));
	event.Skip();
}

void rawprocFrm::CommandTreeDeleteItem(wxTreeEvent& event)
{
	wxTreeItemId s = commandtree->GetSelection();
	if (s)
		((PicProcessor *) commandtree->GetItemData(s))->processPic();
	((PicProcessor *) commandtree->GetItemData(event.GetItem()))->processPic();
}

void rawprocFrm::CommandTreeBeginDrag(wxTreeEvent& event)
{
	event.Allow();
}

void rawprocFrm::CommandTreeEndDrag(wxTreeEvent& event)
{

}


//Menu Items (keep last in file)

/*
 * Mnuopen1003Click
 */
void rawprocFrm::Mnuopen1003Click(wxCommandEvent& event)

{
/*
	myFileSelector *filediag = new myFileSelector(NULL, wxID_ANY, filename.GetPath(), "Open Image");

	if(filediag->ShowModal() == wxID_OK)    {
		wxFileName f(filediag->GetFileSelected());
		wxSetWorkingDirectory (f.GetPath());
        	OpenFile(filediag->GetFileSelected(), filediag->GetFlags());
	}
	filediag->~myFileSelector();
*/
	wxString fname = wxFileSelector("Open Image...", filename.GetPath());	
	if ( !fname.empty() ) { 
		wxFileName f(fname);
		wxSetWorkingDirectory (f.GetPath());
		OpenFile(fname, "");
	}

}

void rawprocFrm::Mnuopensource1004Click(wxCommandEvent& event)
{
	wxString fname = wxFileSelector("Open Image source...", filename.GetPath());	
	if ( !fname.empty() ) { 
		wxFileName f(fname);
		wxSetWorkingDirectory (f.GetPath());
		OpenFileSource(fname);
	}

}

void rawprocFrm::MnuProperties(wxCommandEvent& event)
{
	//if (!diag) diag = new PropertyDialog(this, wxID_ANY, "Properties", (wxFileConfig *) wxConfigBase::Get());
	//diag->Show();

	PropertyDialog diag(this, wxID_ANY, "Properties", (wxFileConfig *) wxConfigBase::Get());

	Bind(wxEVT_PG_CHANGED,&rawprocFrm::UpdateConfig,this);
	diag.ShowModal();
	Unbind(wxEVT_PG_CHANGED,&rawprocFrm::UpdateConfig,this);
	SetStatusText("");
}

void rawprocFrm::UpdateConfig(wxPropertyGridEvent& event)
{
	SetStatusText(wxString::Format("Changed %s to %s.", event.GetPropertyName(), event.GetPropertyValue().GetString()));
	//wxMessageBox(wxString::Format("%s: %s", event.GetPropertyName(), event.GetPropertyValue().GetString()));
	wxConfigBase::Get()->Write(event.GetPropertyName(), event.GetPropertyValue().GetString());
	wxConfigBase::Get()->Flush();
}



/*
 * Mnuadd1005Click
 */
void rawprocFrm::Mnuadd1005Click(wxCommandEvent& event)
{
	// insert your code here
}

/*
 * Mnugamma1006Click
*/
void rawprocFrm::Mnugamma1006Click(wxCommandEvent& event)
{
	SetStatusText("");
	try {
		wxString val = wxConfigBase::Get()->Read("tool.gamma.initialvalue","2.2");
		PicProcessorGamma *g = new PicProcessorGamma("gamma",val, commandtree, pic, parameters);	
		g->processPic();
		wxSafeYield(this);
		if (!commandtree->GetNextSibling(g->GetId()).IsOk()) CommandTreeSetDisplay(g->GetId());
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format("Error: Adding gamma tool failed: %s",e.what()));
	}
}


/*
 * Mnubright1007Click
*/
void rawprocFrm::Mnubright1007Click(wxCommandEvent& event)
{
	SetStatusText("");
	try {
		wxString val = wxConfigBase::Get()->Read("tool.bright.initialvalue","0");
		PicProcessorBright *g = new PicProcessorBright("bright",val, commandtree, pic, parameters);
		g->processPic();
		wxSafeYield(this);
		if (!commandtree->GetNextSibling(g->GetId()).IsOk()) CommandTreeSetDisplay(g->GetId());
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format("Error: Adding bright tool failed: %s",e.what()));
	}
}


/*
 * Mnucontrast1008Click
 */
void rawprocFrm::Mnucontrast1008Click(wxCommandEvent& event)
{
	SetStatusText("");
	try {
		wxString val = wxConfigBase::Get()->Read("tool.contrast.initialvalue","0");
		PicProcessorContrast *c = new PicProcessorContrast("contrast",val, commandtree, pic, parameters);
		c->processPic();
		wxSafeYield(this);
		if (!commandtree->GetNextSibling(c->GetId()).IsOk()) CommandTreeSetDisplay(c->GetId());
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format("Error: Adding contrast tool failed: %s",e.what()));
	}
}



void rawprocFrm::MnusaturateClick(wxCommandEvent& event)
{
	SetStatusText("");
	try {
		wxString val = wxConfigBase::Get()->Read("tool.saturate.initialvalue","1.0");
		PicProcessorSaturation *c = new PicProcessorSaturation("saturation",val, commandtree, pic, parameters);
		c->processPic();
		wxSafeYield(this);
		if (!commandtree->GetNextSibling(c->GetId()).IsOk()) CommandTreeSetDisplay(c->GetId());
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format("Error: Adding saturation tool failed: %s",e.what()));
	}
}



void rawprocFrm::Mnucurve1010Click(wxCommandEvent& event)
{
	SetStatusText("");
	try {
		PicProcessorCurve *crv = new PicProcessorCurve("curve","0.0,0.0,255.0,255.0", commandtree, pic, parameters);
		crv->processPic();
		wxSafeYield(this);
		if (!commandtree->GetNextSibling(crv->GetId()).IsOk()) CommandTreeSetDisplay(crv->GetId());
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format("Error: Adding curve tool failed: %s",e.what()));
	}
}



void rawprocFrm::MnuShadow1015Click(wxCommandEvent& event)
{
	SetStatusText("");
	try {
		wxString level = wxConfigBase::Get()->Read("tool.shadow.level","0");
		wxString threshold = wxConfigBase::Get()->Read("tool.shadow.threshold","64");
		wxString cmd= wxString::Format("%s,%s",level,threshold);
		PicProcessorShadow *shd = new PicProcessorShadow("shadow",cmd, commandtree, pic, parameters);
		shd->processPic();
		wxSafeYield(this);
		if (!commandtree->GetNextSibling(shd->GetId()).IsOk()) CommandTreeSetDisplay(shd->GetId());
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format("Error: Adding shadow tool failed: %s",e.what()));
	}
}

void rawprocFrm::MnuHighlightClick(wxCommandEvent& event)
{
	SetStatusText("");
	try {
		wxString level = wxConfigBase::Get()->Read("tool.highlight.level","0");
		wxString threshold = wxConfigBase::Get()->Read("tool.highlight.threshold","192");
		wxString cmd= wxString::Format("%s,%s",level,threshold);
		PicProcessorHighlight *s = new PicProcessorHighlight("highlight",cmd, commandtree, pic, parameters);
		s->processPic();
		wxSafeYield(this);
		if (!commandtree->GetNextSibling(s->GetId()).IsOk()) CommandTreeSetDisplay(s->GetId());
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format("Error: Adding highlight tool failed: %s",e.what()));
	}
}

void rawprocFrm::MnuGrayClick(wxCommandEvent& event)
{
	SetStatusText("");
	try {
		wxString r = wxConfigBase::Get()->Read("tool.gray.r","0.21");
		wxString g = wxConfigBase::Get()->Read("tool.gray.g","0.72");
		wxString b = wxConfigBase::Get()->Read("tool.gray.b","0.07");
		wxString cmd= wxString::Format("%s,%s,%s",r,g,b);
		PicProcessorGray *gr = new PicProcessorGray("gray",cmd, commandtree, pic, parameters);
		gr->processPic();
		wxSafeYield(this);
		if (!commandtree->GetNextSibling(gr->GetId()).IsOk()) CommandTreeSetDisplay(gr->GetId());
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format("Error: Adding grayscale tool failed: %s",e.what()));
	}
}

void rawprocFrm::MnuCropClick(wxCommandEvent& event)
{
	SetStatusText("");
	try {
		PicProcessorCrop *c = new PicProcessorCrop("crop", commandtree, pic, parameters);
		c->processPic();
		wxSafeYield(this);
		if (!commandtree->GetNextSibling(c->GetId()).IsOk()) CommandTreeSetDisplay(c->GetId());
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format("Error: Adding crop tool failed: %s",e.what()));
	}
}

void rawprocFrm::MnuResizeClick(wxCommandEvent& event)
{
	SetStatusText("");
	try {
		wxString x = wxConfigBase::Get()->Read("tool.resize.x","640");
		wxString y = wxConfigBase::Get()->Read("tool.resize.y","0");
		wxString algo = wxConfigBase::Get()->Read("tool.resize.algorithm","catmullrom");
		wxString cmd= wxString::Format("%s,%s,%s",x,y,algo);
		PicProcessorResize *c = new PicProcessorResize("resize", cmd, commandtree, pic, parameters);
		c->processPic();
		pic->SetScale(1.0);
		wxSafeYield(this);
		if (!commandtree->GetNextSibling(c->GetId()).IsOk()) CommandTreeSetDisplay(c->GetId());
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format("Error: Adding resize tool failed: %s",e.what()));
	}
}

void rawprocFrm::MnuBlackWhitePointClick(wxCommandEvent& event)
{
	SetStatusText("");
	try {
		PicProcessorBlackWhitePoint *c;
		if (wxConfigBase::Get()->Read("tool.blackwhitepoint.auto","0") =="1")
			c = new PicProcessorBlackWhitePoint("blackwhitepoint", "", commandtree, pic, parameters);
		else
			c = new PicProcessorBlackWhitePoint("blackwhitepoint", "0,255", commandtree, pic, parameters);
		//c->processPic();
		wxSafeYield(this);
		if (!commandtree->GetNextSibling(c->GetId()).IsOk()) CommandTreeSetDisplay(c->GetId());
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format("Error: Adding blackwhitepoint tool failed: %s",e.what()));
	}
}

void rawprocFrm::MnuSharpenClick(wxCommandEvent& event)
{
	SetStatusText("");
	try {
		wxString defval = wxConfigBase::Get()->Read("tool.sharpen.initialvalue","0");
		PicProcessorSharpen *c = new PicProcessorSharpen("sharpen", defval, commandtree, pic, parameters);
		if (defval != "0") c->processPic();
		wxSafeYield(this);
		if (!commandtree->GetNextSibling(c->GetId()).IsOk()) CommandTreeSetDisplay(c->GetId());
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format("Error: Adding sharpen tool failed: %s",e.what()));
	}
}

void rawprocFrm::MnuRotateClick(wxCommandEvent& event)
{
	SetStatusText("");
	try {
		wxString defval = wxConfigBase::Get()->Read("tool.rotate.initialvalue","0.0");
		PicProcessorRotate *c = new PicProcessorRotate("rotate", defval, commandtree, pic, parameters);
		if (defval != "0.0") c->processPic();
		wxSafeYield(this);
		if (!commandtree->GetNextSibling(c->GetId()).IsOk()) CommandTreeSetDisplay(c->GetId());
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format("Error: Adding rotate tool failed: %s",e.what()));
	}
}

void rawprocFrm::MnuDenoiseClick(wxCommandEvent& event)
{
	SetStatusText("");
	try {
		wxString sigma = wxConfigBase::Get()->Read("tool.denoise.initialvalue","0.0");
		wxString local = wxConfigBase::Get()->Read("tool.denoise.local","3");
		wxString patch = wxConfigBase::Get()->Read("tool.denoise.patch","1");
		wxString cmd = wxString::Format("%s,%s,%s",sigma,local,patch);
		PicProcessorDenoise *d = new PicProcessorDenoise("denoise", cmd, commandtree, pic, parameters);
		if (sigma != "0") d->processPic();
		wxSafeYield(this);
		if (!commandtree->GetNextSibling(d->GetId()).IsOk()) CommandTreeSetDisplay(d->GetId());
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format("Error: Adding denoise tool failed: %s",e.what()));
	}
}

/*
 * Mnusave1009Click
 */
void rawprocFrm::Mnusave1009Click(wxCommandEvent& event)
{
	wxString fname;
	gImage dib;

	if (!sourcefilename.IsOk()) 
		fname = wxFileSelector("Save image...",filename.GetPath(),filename.GetName(),filename.GetExt(),"JPEG files (*.jpg)|*.jpg|TIFF files (*.tif)|*.tif|PNG files (*.png)|*.png",wxFD_SAVE);
	else
		fname = wxFileSelector("Save image...",sourcefilename.GetPath(),sourcefilename.GetName(),sourcefilename.GetExt(),"JPEG files (*.jpg)|*.jpg|TIFF files (*.tif)|*.tif|PNG files (*.png)|*.png",wxFD_SAVE);

	if ( !fname.empty() )
	{
		if (wxFileName::FileExists(fname)) 
			if (wxMessageBox("File exists; overwrite?", "Confirm", wxYES_NO, this) == wxNO)
				return;
			if (gImage::getFileType(fname) == FILETYPE_UNKNOWN) {
				wxMessageBox("Error: invalid file type");
				return;
			}
			if (commandtree->ItemHasChildren(commandtree->GetRootItem()))
				dib = ((PicProcessor *) commandtree->GetItemData( commandtree->GetLastChild(commandtree->GetRootItem())))->getProcessedPic();
			else
				dib = ((PicProcessor *) commandtree->GetItemData( commandtree->GetRootItem()))->getProcessedPic();

			dib.setInfo("ImageDescription",(std::string) AssembleCommand().c_str());
			dib.setInfo("Software",(std::string) wxString::Format("rawproc %s",version).c_str());

			GIMAGE_FILETYPE filetype = gImage::getFileType(fname);

			wxString configparams;
			if (filetype == FILETYPE_RAW)  configparams = wxConfigBase::Get()->Read("output.raw.parameters", "");
			if (filetype == FILETYPE_JPEG) configparams = wxConfigBase::Get()->Read("output.jpeg.parameters","");
			if (filetype == FILETYPE_TIFF) configparams = wxConfigBase::Get()->Read("output.tiff.parameters","");


			WxStatusBar1->SetStatusText("Saving file...");
			dib.saveImageFile(fname, std::string(configparams.c_str()));
			wxFileName tmpname(fname);

			if (tmpname.GetFullName().compare(filename.GetFullName()) != 0) {
				sourcefilename.Assign(fname);
				SetTitle(wxString::Format("rawproc: %s (%s)",filename.GetFullName().c_str(), sourcefilename.GetFullName().c_str()));
			}
			
		WxStatusBar1->SetStatusText("");
	}
}

void rawprocFrm::MnuCut1201Click(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	if (commandtree->GetSelection() == commandtree->GetRootItem()) return;
	if (wxTheClipboard->Open())
	{
		wxTheClipboard->SetData( new wxTextDataObject(((PicProcessor *) commandtree->GetItemData(commandtree->GetSelection()))->getCommand() ) );
		wxTheClipboard->Close();
		commandtree->Delete(commandtree->GetSelection());
	}
}

void rawprocFrm::MnuCopy1202Click(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	if (wxTheClipboard->Open())
	{
		wxTheClipboard->SetData( new wxTextDataObject(((PicProcessor *) commandtree->GetItemData(commandtree->GetSelection()))->getCommand() ) );
		wxTheClipboard->Close();
	}
}

void rawprocFrm::MnuPaste1203Click(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	if (wxTheClipboard->Open())
	{
		if (wxTheClipboard->IsSupported( wxDF_TEXT ))
		{
			wxTextDataObject data;
			wxTheClipboard->GetData( data );
			wxArrayString s = split(data.GetText(), ":");
			AddItem(s[0], s[1]);
		}
		wxTheClipboard->Close();
	}
}

void rawprocFrm::MnuShowCommand1010Click(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	wxMessageBox(AssembleCommand());
	//wxMessageBox(wxString::Format("%d",(int) pic->getHistogramString().Length()));
	//wxMessageBox( pic->getHistogramString());
}

void rawprocFrm::MnuAbout1011Click(wxCommandEvent& event)
{
	wxAboutDialogInfo info;
	info.SetName(_("rawproc"));
	info.SetVersion(_(version));
	info.SetCopyright(wxT("(C) 2016 Glenn Butcher <glenn.butcher@gmail.com>"));

	wxString gImageVersion(gImage::Version().c_str());
	wxString WxWidgetsVersion = wxGetLibraryVersionInfo().GetVersionString();
	wxString LittleCMSVersion = wxString::Format("%d",cmsGetEncodedCMMversion());
	//wxString PixelFormat = wxString::Format("%d",gImage::getRGBSize());
	info.SetDescription(wxString::Format("Basic camera raw file and image editor.\n\n%s\ngImage %s\nLittleCMS %s\n\nConfiguration file: %s\nPixel Format: %s", WxWidgetsVersion, gImageVersion, LittleCMSVersion, configfile, gImage::getRGBCharacteristics().c_str()));
	wxAboutBox(info);

}

void rawprocFrm::MnuHelpClick(wxCommandEvent& event)
{
	help.DisplayContents();
}

#define ID_EXIF		2001
#define ID_HISTOGRAM	2002
#define ID_DELETE	2003
#define ID_ICC		2004

void rawprocFrm::CommandTreePopup(wxTreeEvent& event)
{
	wxMenu mnu;
 	mnu.Append(ID_EXIF, "Image Information...");
 	mnu.Append(ID_HISTOGRAM, "Full Histogram...");
	mnu.AppendSeparator();
	mnu.Append(ID_DELETE, "Delete");
	switch (GetPopupMenuSelectionFromUser(mnu)) {
		case ID_EXIF:
			EXIFDialog(event.GetItem());
			break;
		case ID_HISTOGRAM:
			wxMessageBox("Not there yet, press 't' to toggle the thumbnail histogram...");
			break;
		case ID_DELETE:
			CommandTreeDeleteItem(event.GetItem());
			break;
	}
}


void rawprocFrm::SetConfigFile(wxString cfile)
{
	configfile = cfile;
}


