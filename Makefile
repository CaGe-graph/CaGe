
what?:

rebuild:
	rm -f ` find cage lisken org -name "*.class" -print `
	javac -cp .:swing.jar:collections.jar:JmolApplet.jar -deprecation ` find cage lisken org -name "*.java" -print `

backup:
	find . -type f -print | cut -c3- | tar -hT- -cf CaGe-backup.tar
	rm -f CaGe-backup.tar.gz
	gzip -9 CaGe-backup.tar

CaGe.jar: clear_distribution
	find cage lisken Images -print | \
	egrep "\.(class|gif|ps)$$" | \
	egrep -v "SysInfo|test|embed\.tar|Viewers|(^hall/)|HelloWorld|\.xvpics" | \
	./j jar c0f CaGe.jar ` sed -e 's:\./::' -e "s:.*:'&':" `

sysinfo.jar: clear_distribution
	./j jar c0f sysinfo.jar org/SysInfo.class

CaGe-C.zip: clear_distribution
	find Native Generators -print | sed -e 's:\./::' | \
	egrep "(\.(c|h)$$)|((Makefile|Sysdir)$$)" | \
	egrep -v "SysInfo|test|embed\.tar|Viewers|(^hall/)|HelloWorld|\.xvpics" | \
	zip -oqX9@ CaGe-C.zip

source_distribution:
	( \
	 find cage lisken Images org Native Generators Makefile -print | \
	 egrep "(\.(java|gif|ps|c|h)$$)|((Makefile|Sysdir)$$)" | \
	 egrep -v "test|embed\.tar|Viewers|(^hall/)|HelloWorld|\.xvpics"; \
	 find INSTALL.sh cage.sh CaGe.ini java j .rasmolrc \
	  JmolApplet.jar collections.jar swing.jar -print \
	) | \
	tar -hT- -cf CaGe-sources.tar
	rm -f CaGe-sources.tar.gz
	gzip -9 CaGe-sources.tar

publish: distribution
	scp CaGe-dist.zip jupiter:WWW/CaGe/CaGe.zip && rm CaGe-dist.zip

distribution: CaGe.jar CaGe-C.zip sysinfo.jar
	zip -oqX9 CaGe-dist.zip INSTALL.sh cage.sh CaGe.ini java sysinfo.jar .rasmolrc Data CaGe-C.zip CaGe.jar JmolApplet.jar collections.jar swing.jar
	rm -f CaGe.jar CaGe-C.zip sysinfo.jar

clear_distribution:
	rm -f CaGe.jar CaGe-C.zip CaGe-dist.zip sysinfo.jar

