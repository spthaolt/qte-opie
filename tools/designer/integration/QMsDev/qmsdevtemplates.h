const char main_cpp[] = {
    "#include <qapplication.h>\n"
    "#include \"$QMSDEVCLASSHEADER.h\"\n"
    "\n\n"    
    "int main( int argc, char** argv )\n"
    "{\n"
    "\tQApplication app( argc, argv );\n"
    "\n"
    "\t$QMSDEVCLASSNAME $QMSDEVINSTANCE$QMSDEVINSTCALL;\n"
    "\tapp.setMainWidget(&$QMSDEVINSTANCE);\n"
    "\n"
    "\t$QMSDEVRUNAPP\n"
    "}\n"
    "\n"
};


const char dialogbase_ui[] = {
"<!DOCTYPE UI><UI>\n"
"<class>$QMSDEVCLASSNAMEBase</class>\n"
"<widget>\n"
"\t<class>QDialog</class>\n"
"\t<property>\n"
"\t\t<name>name</name>\n"
"\t\t<cstring>$QMSDEVCLASSNAMEBase</cstring>\n"
"\t</property>\n"
"\t<property>\n"
"\t\t<name>geometry</name>\n"
"\t\t<rect>\n"
"\t\t\t<x>0</x>\n"
"\t\t\t<y>0</y>\n"
"\t\t\t<width>519</width>\n"
"\t\t\t<height>285</height>\n"
"\t\t</rect>\n"
"\t</property>\n"
"\t<property>\n"
"\t\t<name>caption</name>\n"
"\t\t<string>$QMSDEVPROJECTNAME</string>\n"
"\t</property>\n"
"\t<property>\n"
"\t\t<name>sizeGripEnabled</name>\n"
"\t\t<bool>true</bool>\n"
"\t</property>\n"
"\t<widget>\n"
"\t\t<class>QLayoutWidget</class>\n"
"\t\t<property>\n"
"\t\t\t<name>name</name>\n"
"\t\t\t<cstring>Layout1</cstring>\n"
"\t\t</property>\n"
"\t\t<property>\n"
"\t\t\t<name>geometry</name>\n"
"\t\t\t<rect>\n"
"\t\t\t\t<x>20</x>\n"
"\t\t\t\t<y>240</y>\n"
"\t\t\t\t<width>476</width>\n"
"\t\t\t\t<height>33</height>\n"
"\t\t\t</rect>\n"
"\t\t</property>\n"
"\t\t<hbox>\n"
"\t\t<property>\n"
"\t\t\t<name>margin</name>\n"
"\t\t\t<number>0</number>\n"
"\t\t</property>\n"
"\t\t<property>\n"
"\t\t\t<name>spacing</name>\n"
"\t\t\t<number>6</number>\n"
"\t\t</property>\n"
"\t\t<widget>\n"
"\t\t<class>QPushButton</class>\n"
"\t\t<property>\n"
"\t\t\t<name>name</name>\n"
"\t\t\t<cstring>buttonHelp</cstring>\n"
"\t\t</property>\n"
"\t\t<property>\n"
"\t\t\t<name>text</name>\n"
"\t\t\t<string>&amp;Help</string>\n"
"\t\t</property>\n"
"\t\t<property>\n"
"\t\t\t<name>autoDefault</name>\n"
"\t\t\t<bool>true</bool>\n"
"\t\t</property>\n"
"\t\t</widget>\n"
"\t\t<spacer>\n"
"\t\t<property>\n"
"\t\t\t<name>name</name>\n"
"\t\t\t<cstring>Horizontal Spacing2</cstring>\n"
"\t\t</property>\n"
"\t\t<property>\n"
"\t\t<name>geometry</name>\n"
"\t\t<rect>\n"
"\t\t<x>87</x>\n"
"\t\t<y>6</y>\n"
"\t\t<width>130</width>\n"
"\t\t<height>20</height>\n"
"\t\t</rect>\n"
"\t\t</property>\n"
"\t\t<property>\n"
"\t\t<name>orientation</name>\n"
"\t\t<enum>Horizontal</enum>\n"
"\t\t</property>\n"
"\t\t<property>\n"
"\t\t<name>sizeType</name>\n"
"\t\t<enum>Expanding</enum>\n"
"\t\t</property>\n"
"\t\t</spacer>\n"
"\t\t<widget>\n"
"\t\t<class>QPushButton</class>\n"
"\t\t<property>\n"
"\t\t<name>name</name>\n"
"\t\t<cstring>buttonApply</cstring>\n"
"\t\t</property>\n"
"\t\t<property>\n"
"\t\t<name>text</name>\n"
"\t\t<string>&amp;Apply</string>\n"
"\t\t</property>\n"
"\t\t<property>\n"
"\t\t<name>autoDefault</name>\n"
"\t\t<bool>true</bool>\n"
"\t\t</property>\n"
"\t\t</widget>\n"
"\t\t<widget>\n"
"\t\t<class>QPushButton</class>\n"
"\t\t<property>\n"
"\t\t<name>name</name>\n"
"\t\t<cstring>buttonOk</cstring>\n"
"\t\t</property>\n"
"\t\t<property>\n"
"\t\t<name>caption</name>\n"
"\t\t<string></string>\n"
"\t\t</property>\n"
"\t\t<property>\n"
"\t\t<name>text</name>\n"
"\t\t<string>&amp;OK</string>\n"
"\t\t</property>\n"
"\t\t<property>\n"
"\t\t<name>autoDefault</name>\n"
"\t\t<bool>true</bool>\n"
"\t\t</property>\n"
"\t\t<property>\n"
"\t\t<name>default</name>\n"
"\t\t<bool>true</bool>\n"
"\t\t</property>\n"
"\t\t</widget>\n"
"\t\t<widget>\n"
"\t\t<class>QPushButton</class>\n"
"\t\t<property>\n"
"\t\t<name>name</name>\n"
"\t\t<cstring>buttonCancel</cstring>\n"
"\t\t</property>\n"
"\t\t<property>\n"
"\t\t<name>text</name>\n"
"\t\t<string>&amp;Cancel</string>\n"
"\t\t</property>\n"
"\t\t<property>\n"
"\t\t<name>autoDefault</name>\n"
"\t\t<bool>true</bool>\n"
"\t\t</property>\n"
"\t\t</widget>\n"
"\t\t</hbox>\n"
"\t\t</widget>\n"
"\t\t<widget>\n"
"\t\t<class>QLabel</class>\n"
"\t\t<property>\n"
"\t\t<name>name</name>\n"
"\t\t<cstring>TextLabel1</cstring>\n"
"\t\t</property>\n"
"\t\t<property>\n"
"\t\t<name>geometry</name>\n"
"\t\t<rect>\n"
"\t\t<x>200</x>\n"
"\t\t<y>120</y>\n"
"\t\t<width>116</width>\n"
"\t\t<height>13</height>\n"
"\t\t</rect>\n"
"\t\t</property>\n"
"\t\t<property>\n"
"\t\t<name>text</name>\n"
"\t\t<string>Place your widgets here!</string>\n"
"\t\t</property>\n"
"\t</widget>\n"
"</widget>\n"
"<connections>\n"
"\t<connection>\n"
"\t\t<sender>buttonOk</sender>\n"
"\t\t<signal>clicked()</signal>\n"
"\t\t<receiver>$QMSDEVCLASSNAMEBase</receiver>\n"
"\t\t<slot>accept()</slot>\n"
"\t</connection>\n"
"\t<connection>\n"
"\t\t<sender>buttonCancel</sender>\n"
"\t\t<signal>clicked()</signal>\n"
"\t\t<receiver>$QMSDEVCLASSNAMEBase</receiver>\n"
"\t<slot>reject()</slot>\n"
"\t</connection>\n"
"</connections>\n"
"</UI>\n"
"\n"
};

const char dialog_h[] = {
"#include \"$QMSDEVCLASSHEADERbase.h\"\n"
"\n\n"
"class $QMSDEVCLASSNAME : public $QMSDEVCLASSNAMEBase\n"
"{\n"
"\tQ_OBJECT\n"
"public:\n"
"\t$QMSDEVCLASSNAME( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags f = 0 );\n"
"};\n"
"\n"
};

const char dialog_cpp[] = {
"#include \"$QMSDEVCLASSHEADER.h\"\n"
"\n\n"
"$QMSDEVCLASSNAME::$QMSDEVCLASSNAME( QWidget* parent, const char* name, bool modal, WFlags f )\n"
"\t: $QMSDEVCLASSNAMEBase( parent, name, modal, f )\n"
"{\n"
"\n"
"\t// Add your code\n"
"\n"
"}\n"
};

const char window_h[] = {
    "#include <qmainwindow.h>\n"
    "\n\n"
    "class $QMSDEVCLASSNAME : public QMainWindow\n"
    "{\n"
    "\tQ_OBJECT\n"
    "public:\n"
    "\t$QMSDEVCLASSNAME( QWidget* parent = 0, const char* name = 0, WFlags f = WType_TopLevel );\n"
    "};\n"
    "\n"
};

const char window_cpp[] = {
    "#include \"$QMSDEVCLASSHEADER.h\"\n"
    "\n\n"
    "$QMSDEVCLASSNAME::$QMSDEVCLASSNAME( QWidget* parent, const char* name, WFlags f )\n"
    "\t: QMainWindow( parent, name, f )\n"
    "{\n"
    "\tsetCaption(\"$QMSDEVPROJECTNAME\");\n"
    "\n"
    "\t// Add your code\n"
    "\n"
    "}\n"
};

const char mdi_h[] = {
    "#include <qmainwindow.h>\n"
    "\n\n"
    "class QWorkspace;\n"
    "\n"
    "class $QMSDEVCLASSNAME : public QMainWindow\n"
    "{\n"
    "\tQ_OBJECT\n"
    "public:\n"
    "\t$QMSDEVCLASSNAME( QWidget* parent = 0, const char* name = 0, WFlags f = WType_TopLevel );\n"
    "\n"
    "protected:\n"
    "\tQWorkspace *mdi;\n"
    "\n"
    "protected slots:\n"
    "\tvoid fileNew();\n"
    "};\n"
    "\n"
};

const char mdi_cpp[] = {
    "#include \"$QMSDEVCLASSHEADER.h\"\n"
    "\n"
    "#include <qapplication.h>\n"
    "#include <qworkspace.h>\n"
    "#include <qmenubar.h>\n"
    "#include <qtoolbar.h>\n"
    "#include <qpopupmenu.h>\n"
    "#include <qaction.h>\n"
    "\n\n"
    "$QMSDEVCLASSNAME::$QMSDEVCLASSNAME( QWidget* parent, const char* name, WFlags f )\n"
    "\t: QMainWindow( parent, name, f )\n"
    "{\n"
    "\tsetCaption(\"$QMSDEVPROJECTNAME\");\n"
    "\n"
    "\tmdi = new QWorkspace( this, \"workspace\" );\n"
    "\tmdi->setBackgroundMode( PaletteDark );\n"
    "\n"
    "\tQPopupMenu *fileMenu = new QPopupMenu( this );\n"
    "\tmenuBar()->insertItem( tr(\"&File\"), fileMenu );\n"
    "\tQToolBar *fileToolBar = new QToolBar( this );\n"
    "\taddToolBar( fileToolBar, tr(\"File\") );"
    "\n"
    "\tQAction *action = 0;\n"
    "\n"
    "\taction = new QAction( tr(\"New\"), tr(\"&New\"), CTRL+Key_N, this );\n"
    "\tconnect( action, SIGNAL( activated() ), SLOT( fileNew() ) );\n"
    "\taction->addTo( fileMenu );\n"
    "\taction->addTo( fileToolBar );\n"
    "\n"
    "\taction = new QAction( tr(\"Exit\"), tr(\"E&xit\"), 0, this );\n"
    "\tfileMenu->insertSeparator();\n"
    "\taction->addTo( fileMenu );\n"
    "\tconnect( action, SIGNAL( activated() ), qApp, SLOT( closeAllWindows() ) );\n"
    "\n"
    "\t// Insert more actions\n"
    "\n"
    "\tsetCentralWidget( mdi );\n"
    "\n"
    "}\n"
    "\n"
    "\n"
    "void $QMSDEVCLASSNAME::fileNew()\n"
    "{\n"
    "\t// Use your own documentclass\n"
    "\n"
    "\tQWidget *doc = new QWidget( mdi, \"$MSDEVPROJECTNAME document\" );\n"
    "\tdoc->setCaption( tr(\"Document%1\").arg( mdi->windowList().count()+1 ) );\n"
    "\tdoc->show();\n"
    "}\n"
    "\n"
};

const char readme_txt[] = {
    "========================================================================\n"
    "        Qt Application \"$QMSDEVPROJECTNAME\"\n"
    "========================================================================\n"
    "\n"
    "\n"
    "This Developer Studio Project file has been created by the QMsDev\n"
    "plugin and is a basic implementation of a Qt application based on\n"
    "a $QMSDEVUITYPE.\n"
    "\n"
    "The project contains the following files in which you should add the\n"
    "code for your project:\n"
    "\n"
    "main.cpp\n"
    "\tThe starting point for your application. Add startup routines, e.g.\n"
    "\tcommandline processing here.\n"
    "\n"
    "$QMSDEVCLASSHEADER.h\n"
    "\tA basic declaration of your application window. Add declarations of\n"
    "\tmethods here.\n"
    "$QMSDEVCLASSHEADER.cpp\n"
    "\tA basic implementation of the methods declared in\n"
    "\t$QMSDEVCLASSHEADER.h. Add implementation code here.\n"
    "$QMSDEVFILELIST"
    "\n"
    "The following files will be generated during the Qt specific build\n"
    "steps, and you shouldn't modify them:\n"
    "\n"
    "$QMSDEVQTFILELIST"
    "\n"
};

