#include "EditorApp.h"

wxIMPLEMENT_APP(EditorApp);

EditorApp::EditorApp()
{
}

/*virtual*/ EditorApp::~EditorApp()
{
}

/*virtual*/ bool EditorApp::OnInit()
{
	if (!wxApp::OnInit())
		return false;

	// TODO: Make and show frame here.
	// TODO: App lets you open/close a directory.  Once opened, a tree of .pow files is shown on left.
	//       You can then double-click on files in the tree to open them.  Closing the files is done
	//       by just closing the tab.
	// TODO: This app embeds the Powder VM and attaches to it by default, but you could attach to some
	//       other VM embedded elsewhere if you want.

	return true;
}

/*virtual*/ int EditorApp::OnExit()
{
	return 0;
}