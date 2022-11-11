
what?:

rebuild: clean
	javac -cp .:Jmol.jar -deprecation -source 1.8 -target 1.8 ` find cage lisken util -name "*.java" -print `

clean:
	rm -f ` find cage lisken util -name "*.class" -print `

backup:
	find . -type f -print | cut -c3- | tar -hT- -cf CaGe-backup.tar
	rm -f CaGe-backup.tar.gz
	gzip -9 CaGe-backup.tar

CaGe.jar: clear_distribution rebuild
	find cage lisken Images -print | \
	egrep "\.(class|gif|ps)$$" | \
	egrep -v "SysInfo|test|embed\.tar|Viewers|(^hall/)|HelloWorld|\.xvpics" | \
	./j jar c0f CaGe.jar ` sed -e 's:\./::' -e "s:.*:'&':" `

sysinfo.jar: clear_distribution rebuild
	./j jar c0f sysinfo.jar util/SysInfo.class

CaGe-C.zip: clear_distribution
	find Native Generators PreCompute -print | sed -e 's:\./::' | \
	egrep "(\.(c|h)$$)|((Makefile|Sysdir)$$)" | \
	egrep -v "SysInfo|test|embed\.tar|Viewers|(^hall/)|HelloWorld|\.xvpics" | \
	zip -oqX9@ CaGe-C.zip

source_distribution:
	( \
	 find cage lisken Images util Native Generators PreCompute img Makefile -print | \
	 egrep "(\.(java|gif|png|ps|c|h)$$)|((Makefile|Sysdir)$$)" | \
	 egrep -v "test|embed\.tar|Viewers|(^hall/)|HelloWorld|\.xvpics"; \
	 find INSTALL.sh cage.sh CaGe.ini java j .rasmolrc \
	  Jmol.jar -print \
	) | \
	tar -hT- -cf CaGe-sources.tar
	rm -f CaGe-sources.tar.gz
	gzip -9 CaGe-sources.tar

publish: distribution
	scp CaGe-dist.zip jupiter:WWW/CaGe/CaGe.zip && rm CaGe-dist.zip

distribution: CaGe.jar CaGe-C.zip sysinfo.jar
	zip -oqX9 CaGe-dist.zip INSTALL.sh cage.sh CaGe.ini img/logo.png java sysinfo.jar .rasmolrc Data CaGe-C.zip CaGe.jar Jmol.jar
	rm -f CaGe.jar CaGe-C.zip sysinfo.jar

clear_distribution:
	rm -f CaGe.jar CaGe-C.zip CaGe-dist.zip sysinfo.jar

javadoc:
	javadoc -d ../javadoc/ -link ../jmol/ -link http://download.oracle.com/javase/1.5.0/docs/api/ -sourcepath .:../libraries/Jmol/src/ -subpackages cage:lisken:util -use -group "CaGe Core" "cage*" -group "Utility" "lisken*:util*"  -header "<b>CaGe</b>" -windowtitle CaGe -doctitle CaGe
