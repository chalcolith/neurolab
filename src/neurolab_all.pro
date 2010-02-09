TEMPLATE = subdirs
SUBDIRS = automata neurolib neurolab 
#asyncLife.depends = automata
neurolib.depends = automata
neurolab.depends = neurolib
