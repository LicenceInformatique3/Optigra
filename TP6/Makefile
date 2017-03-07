# Makefile general du TP - 21/02/2016

SHELL = /bin/bash
RM    = rm -f

# Mettre ici la liste des repertoires des differents programmes ou versions
# de programmes a compiler, separee par des espaces : 
DIRS = V*

help ::
	@ echo "Tapez: make depend    pour calculer les dependances"
	@ echo "       make all       pour tout compiler"
	@ echo "       make clean     pour tout nettoyer"
	@ echo "       make tar       pour tout sauvegarder"

depend all clean ::
	@ for d in $(DIRS) ; do \
	    test -f "$$d"/Makefile || continue ;\
	    (cd "$$d" && make $@) ;\
	done

clean ::
	$(RM) *~

tar :: clean
	@N=$$(pwd) ; N=$$(basename "$$N") ;\
	(cd .. && tar cvfz "$$N.tgz" "$$N" --exclude='svg*' --exclude='*.tgz' && \
	 echo "DONE ../$$N.tgz")

