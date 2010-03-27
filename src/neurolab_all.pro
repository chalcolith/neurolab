TEMPLATE = subdirs
SUBDIRS = griditems neurolab neurogui neurolib automata thirdparty # asyncLife
#asyncLife.depends = automata
neurolib.depends = automata
neurogui.depends = neurolib thirdparty
neurolab.depends = neurogui
griditems.depends = neurogui neurolib
