
# Makefile for CaGe's native libraries

LIBRARIES=	libCaGe.so \
		libNativeEmbeddableGraph.so libNativeEmbedEmbedder.so \
		libPipe.so libNativeCaGePipe.so \
		libBufferedFDInputStream.so libBufferedFDOutputStream.so \
		libNativePlanarWriter.so \
		libNativeWritegraphWriter.so libNativeCMLWriter.so \
		libNativePDBWriter.so \
		libSystoolbox.so

MACLIBRARIES=	libCaGe.jnilib \
		libNativeEmbeddableGraph.jnilib libNativeEmbedEmbedder.jnilib \
		libPipe.jnilib libNativeCaGePipe.jnilib \
		libBufferedFDInputStream.jnilib libBufferedFDOutputStream.jnilib \
		libNativePlanarWriter.jnilib \
		libNativeWritegraphWriter.jnilib libNativeCMLWriter.jnilib \
		libNativePDBWriter.jnilib \
		libSystoolbox.jnilib

DEMOS=		t_dstring t_graph t_pipe t_cmdv

CFLAGS= -fPIC

cage_library:	libCaGe.so clean

# Windows:	CPPFLAGS=$(CPPFLAGS) -I//D/Programme/Java/jdk1.1.8/include -I//D/Programme/Java/jdk1.1.8/include/win32
Windows:	CaGe.dll clean

Linux:		$(LIBRARIES) clean

Mac:		$(MACLIBRARIES) clean

demos:		$(DEMOS)

clean:
	rm -f *.o

reset:
	rm -f *.o $(LIBRARIES) $(DEMOS) CaGe.def

all:		mac clean

libCaGe.so:	NativeCaGePipe.c NativeEmbedEmbedder.c \
		NativeEmbeddableGraph.c NativeEdgeIterator.c \
		NativePlanarWriter.c \
		NativeWritegraphWriter.c NativeCMLWriter.c \
		NativePDBWriter.c \
		Pipe.c BufferedFDInputStream.c BufferedFDOutputStream.c \
		get_element.c check_graph.c \
		graph.c read_graphs.c dstring.c cmd_vector.c \
		pipe_tools.c file_tools.c \
		malloc.c jbytes.c j_debug.c j_error_exit.c \
		Systoolbox.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -shared $^ -o $@
	cp $@ ../` sh ./Sysdir `/$(patsubst lib%.so,%,$@)

CaGe.dll:	NativeCaGePipe.c NativeEmbedEmbedder.c \
		NativeEmbeddableGraph.c NativeEdgeIterator.c \
		NativePlanarWriter.c \
		NativeWritegraphWriter.c NativeCMLWriter.c \
		NativePDBWriter.c \
		Pipe.c BufferedFDInputStream.c BufferedFDOutputStream.c \
		get_element.c check_graph.c \
		graph.c read_graphs.c dstring.c cmd_vector.c \
		pipe_tools.c file_tools.c \
		malloc.c jbytes.c j_debug.c j_error_exit.c \
		Systoolbox.c
	$(CC) $(CFLAGS) -c $(CPPFLAGS) $^
	dllwrap --output-def CaGe.def --add-stdcall-alias --driver-name $(CC) -o $@ -Wl,-e,__cygwin_noncygwin_dll_entry@12 -s $(^:.c=.o)
	cp $@ ../Windows/$@

libPipe.so:	Pipe.c pipe_tools.c cmd_vector.c \
		jbytes.c malloc.c j_debug.c j_error_exit.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -shared $^ -o $@
	cp $@ ../` sh ./Sysdir `/$(patsubst lib%.so,%,$@)

libNativeCaGePipe.so:	NativeCaGePipe.c jbytes.c read_graphs.c \
			pipe_tools.c file_tools.c \
			graph.c dstring.c malloc.c j_debug.c j_error_exit.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -shared $^ -o $@
	cp $@ ../` sh ./Sysdir `/$(patsubst lib%.so,%,$@)

libNativeEmbedEmbedder.so:	NativeEmbedEmbedder.c \
				pipe_tools.c read_graphs.c \
				graph.c cmd_vector.c \
				jbytes.c dstring.c malloc.c \
				j_debug.c j_error_exit.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -shared $^ -o $@
	cp $@ ../` sh ./Sysdir `/$(patsubst lib%.so,%,$@)

libNativeEmbeddableGraph.so:	NativeEmbeddableGraph.c NativeEdgeIterator.c \
				graph.c dstring.c malloc.c \
				jbytes.c j_debug.c j_error_exit.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -shared $^ -o $@
	cp $@ ../` sh ./Sysdir `/$(patsubst lib%.so,%,$@)

libSystoolbox.so: Systoolbox.c jbytes.c malloc.c j_error_exit.c j_debug.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -shared $^ -o $@
	cp $@ ../` sh ./Sysdir `/$(patsubst lib%.so,%,$@)

libBufferedFDInputStream.so:	BufferedFDInputStream.c file_tools.c \
				jbytes.c j_debug.c j_error_exit.c malloc.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -shared $^ -o $@
	cp $@ ../` sh ./Sysdir `/$(patsubst lib%.so,%,$@)

libBufferedFDOutputStream.so:	BufferedFDOutputStream.c \
				jbytes.c j_debug.c j_error_exit.c malloc.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -shared $^ -o $@
	cp $@ ../` sh ./Sysdir `/$(patsubst lib%.so,%,$@)

libNativePlanarWriter.so:	NativePlanarWriter.c \
				dstring.c graph.c check_graph.c \
				malloc.c jbytes.c j_debug.c j_error_exit.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -shared $^ -o $@
	cp $@ ../` sh ./Sysdir `/$(patsubst lib%.so,%,$@)

libNativeWritegraphWriter.so:	NativeWritegraphWriter.c \
				dstring.c graph.c read_graphs.c check_graph.c \
				malloc.c jbytes.c j_debug.c j_error_exit.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -shared $^ -o $@
	cp $@ ../` sh ./Sysdir `/$(patsubst lib%.so,%,$@)

libNativeCMLWriter.so:		NativeCMLWriter.c get_element.c \
				dstring.c graph.c jbytes.c check_graph.c \
				malloc.c j_debug.c j_error_exit.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -shared $^ -o $@
	cp $@ ../` sh ./Sysdir `/$(patsubst lib%.so,%,$@)

libNativePDBWriter.so:		NativePDBWriter.c get_element.c \
				dstring.c graph.c jbytes.c check_graph.c \
				malloc.c j_debug.c j_error_exit.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -shared $^ -o $@
	cp $@ ../` sh ./Sysdir `/$(patsubst lib%.so,%,$@)

libCaGe.jnilib:	NativeCaGePipe.c NativeEmbedEmbedder.c \
		NativeEmbeddableGraph.c NativeEdgeIterator.c \
		NativePlanarWriter.c \
		NativeWritegraphWriter.c NativeCMLWriter.c \
		NativePDBWriter.c \
		Pipe.c BufferedFDInputStream.c BufferedFDOutputStream.c \
		get_element.c check_graph.c \
		graph.c read_graphs.c dstring.c cmd_vector.c \
		pipe_tools.c file_tools.c \
		malloc.c jbytes.c j_debug.c j_error_exit.c \
		Systoolbox.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -dynamiclib $^ -o $@ -framework JavaVM -single_module
	cp $@ ../` sh ./Sysdir `/$(patsubst lib%.jnilib,%,$@)

libPipe.jnilib:	Pipe.c pipe_tools.c cmd_vector.c \
		jbytes.c malloc.c j_debug.c j_error_exit.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -dynamiclib $^ -o $@ -framework JavaVM -single_module
	cp $@ ../` sh ./Sysdir `/$(patsubst lib%.jnilib,%,$@)

libNativeCaGePipe.jnilib:	NativeCaGePipe.c jbytes.c read_graphs.c \
			pipe_tools.c file_tools.c \
			graph.c dstring.c malloc.c j_debug.c j_error_exit.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -dynamiclib $^ -o $@ -framework JavaVM -single_module
	cp $@ ../` sh ./Sysdir `/$(patsubst lib%.jnilib,%,$@)

libNativeEmbedEmbedder.jnilib:	NativeEmbedEmbedder.c \
				pipe_tools.c read_graphs.c \
				graph.c cmd_vector.c \
				jbytes.c dstring.c malloc.c \
				j_debug.c j_error_exit.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -dynamiclib $^ -o $@ -framework JavaVM -single_module
	cp $@ ../` sh ./Sysdir `/$(patsubst lib%.jnilib,%,$@)

libNativeEmbeddableGraph.jnilib:	NativeEmbeddableGraph.c NativeEdgeIterator.c \
				graph.c dstring.c malloc.c \
				jbytes.c j_debug.c j_error_exit.c read_graphs.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -dynamiclib $^ -o $@ -framework JavaVM -single_module
	cp $@ ../` sh ./Sysdir `/$(patsubst lib%.jnilib,%,$@)

libSystoolbox.jnilib: Systoolbox.c jbytes.c malloc.c j_error_exit.c j_debug.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -dynamiclib $^ -o $@ -framework JavaVM -single_module
	cp $@ ../` sh ./Sysdir `/$(patsubst lib%.jnilib,%,$@)

libBufferedFDInputStream.jnilib:	BufferedFDInputStream.c file_tools.c \
				jbytes.c j_debug.c j_error_exit.c malloc.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -dynamiclib $^ -o $@ -framework JavaVM -single_module
	cp $@ ../` sh ./Sysdir `/$(patsubst lib%.jnilib,%,$@)

libBufferedFDOutputStream.jnilib:	BufferedFDOutputStream.c \
				jbytes.c j_debug.c j_error_exit.c malloc.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -dynamiclib $^ -o $@ -framework JavaVM -single_module
	cp $@ ../` sh ./Sysdir `/$(patsubst lib%.jnilib,%,$@)

libNativePlanarWriter.jnilib:	NativePlanarWriter.c \
				dstring.c graph.c check_graph.c \
				malloc.c jbytes.c j_debug.c j_error_exit.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -dynamiclib $^ -o $@ -framework JavaVM -single_module
	cp $@ ../` sh ./Sysdir `/$(patsubst lib%.jnilib,%,$@)

libNativeWritegraphWriter.jnilib:	NativeWritegraphWriter.c \
				dstring.c graph.c read_graphs.c check_graph.c \
				malloc.c jbytes.c j_debug.c j_error_exit.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -dynamiclib $^ -o $@ -framework JavaVM -single_module
	cp $@ ../` sh ./Sysdir `/$(patsubst lib%.jnilib,%,$@)

libNativeCMLWriter.jnilib:		NativeCMLWriter.c get_element.c \
				dstring.c graph.c jbytes.c check_graph.c \
				malloc.c j_debug.c j_error_exit.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -dynamiclib $^ -o $@ -framework JavaVM -single_module
	cp $@ ../` sh ./Sysdir `/$(patsubst lib%.jnilib,%,$@)

libNativePDBWriter.jnilib:		NativePDBWriter.c get_element.c \
				dstring.c graph.c jbytes.c check_graph.c \
				malloc.c j_debug.c j_error_exit.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -dynamiclib $^ -o $@ -framework JavaVM -single_module
	cp $@ ../` sh ./Sysdir `/$(patsubst lib%.jnilib,%,$@)

lib%.so: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -shared $(patsubst lib%.so,%,$@).c -o $@

%.class: %.java
	javac $<

t_dstring: t_dstring.c error_exit.c dstring.c malloc.c
	$(CC) -g $(CFLAGS) $(CPPFLAGS) $^ -o $@

t_pipe:		t_pipe.c NativeCaGePipe.c \
		NativeEmbeddableGraph.c NativeEmbedEmbedder.c \
		cmd_vector.c jbytes.c pipe_tools.c file_tools.c \
		read_graphs.c graph.c error_exit.c dstring.c malloc.c
	$(CC) -g $(CFLAGS) $(CPPFLAGS) $^ -o $@

t_graph: t_graph.c graph.c error_exit.c malloc.c
	$(CC) -g $(CFLAGS) $(CPPFLAGS) $^ -o $@

t_cmdv: t_cmdv.c error_exit.c cmd_vector.c jbytes.c malloc.c
	$(CC) -g $(CFLAGS) $(CPPFLAGS) $^ -o $@
