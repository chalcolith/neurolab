TEMPLATE = subdirs
SUBDIRS = automata asyncLife neurolib neurolab
asyncLife.depends = automata
neurolib.depends = automata
neurolab.depends = neurolib
