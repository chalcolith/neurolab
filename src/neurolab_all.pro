TEMPLATE = subdirs
SUBDIRS = griditems neurolab neurogui neurolib automata thirdparty 
neurolib.depends = automata
neurogui.depends = neurolib thirdparty
neurolab.depends = neurogui
griditems.depends = neurogui neurolib
