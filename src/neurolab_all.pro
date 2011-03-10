TEMPLATE = subdirs
SUBDIRS = common automata neurolib thirdparty neurogui neurolab griditems

automata.depends = common
neurolib.depends = common automata
neurogui.depends = common neurolib thirdparty
neurolab.depends = common neurogui
griditems.depends = common neurogui
