TEMPLATE = subdirs
SUBDIRS = neurolab neurolib asyncLife automata thirdparty
asyncLife.depends = automata
neurolib.depends = automata
neurolab.depends = neurolib thirdparty
