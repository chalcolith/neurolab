TEMPLATE = subdirs
SUBDIRS = neurolab neurolib automata thirdparty
#asyncLife.depends = automata
neurolib.depends = automata
neurolab.depends = neurolib thirdparty
