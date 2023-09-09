#include "EvalStackPanel.h"
#include "RunThread.h"

wxIMPLEMENT_DYNAMIC_CLASS(EvalStackPanel, Panel);

EvalStackPanel::EvalStackPanel()
{
	this->evalStackCtrl = nullptr;
}

/*virtual*/ EvalStackPanel::~EvalStackPanel()
{
}

/*virtual*/ bool EvalStackPanel::GetPaneInfo(wxAuiPaneInfo& paneInfo)
{
	paneInfo.Caption("Eval Stack");
	paneInfo.Dockable();
	paneInfo.BestSize(300, 300);
	paneInfo.Name("Eval Stack");
	paneInfo.Dock().Right();
	return true;
}

/*virtual*/ bool EvalStackPanel::MakeControls(void)
{
	this->evalStackCtrl = new wxListView(this);
	this->evalStackCtrl->AppendColumn("Address");
	this->evalStackCtrl->AppendColumn("Value");

	wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
	boxSizer->Add(this->evalStackCtrl, 1, wxGROW | wxALL, 0);
	this->SetSizer(boxSizer);

	return true;
}

/*virtual*/ void EvalStackPanel::OnNotified(Notification notification, void* notifyData)
{
	switch (notification)
	{
		case RUNTHREAD_STARTED:
		{
			break;
		}
		case RUNTHREAD_SUSPENDED:
		{
			RunThreadSuspendedEvent* event = (RunThreadSuspendedEvent*)notifyData;
			this->RebuildStackList(event->executor);
			break;
		}
	}
}

void EvalStackPanel::RebuildStackList(Powder::Executor* executor)
{
	this->evalStackCtrl->DeleteAllItems();
	for (uint32_t i = 0; i < executor->StackSize(); i++)
	{
		Powder::Value* value = executor->StackValue(i, nullptr);
		this->evalStackCtrl->InsertItem(i, wxString::Format("0x%08x", uint32_t(value)));
		this->evalStackCtrl->SetItem(i, 1, wxString(value->ToString().c_str()));
	}
}