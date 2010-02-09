TEMPLATE = subdirs
SUBDIRS = automata neurolib neurolab
neurolib.depends = automata
neurolab.depends = neurolib
