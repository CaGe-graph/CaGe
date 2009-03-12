package lisken.systoolbox;

import java.io.EOFException;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;

public class Pipe {

    protected native void startPipe(Object[] cmds,
            int i_fd, int o_fd,
            byte[] i_name, byte[] o_name, boolean o_append, byte[] e_name);

    public native int checkForExit();

    public native int waitForExit();

    public native void stop();

    protected native void finalizePipe();
    protected int pipe_pid;
    protected int i_fd,  o_fd,  writer_fd,  reader_fd;
    protected byte[] i_name,  o_name,  e_name;
    protected boolean o_append;
    protected Object[] cmds;
    protected byte[] runDir,  path;

    /*
    static {
    System.loadLibrary("Pipe");
    }
     */
    public Pipe(String[][] commands)
            throws Exception {
        this();
        cmds = Systoolbox.stringsToBytes(commands);
    }

    public Pipe(String[][] commands,
            int inFildes, int outFildes,
            String inFilename, String outFilename, String errFilename)
            throws Exception {
        this(commands, inFildes, outFildes,
                inFilename, outFilename, false, errFilename);
    }

    public Pipe(String[][] commands,
            int inFildes, int outFildes,
            String inFilename, String outFilename, boolean append, String errFilename)
            throws Exception {
        this(commands);
        i_fd = inFildes;
        o_fd = outFildes;
        setInFile(inFilename);
        setOutFile(outFilename, append);
        setErrFile(errFilename);
    }

    public Pipe(String[][] commands,
            String inFilename, String outFilename, String errFilename)
            throws Exception {
        this(commands, inFilename, outFilename, false, errFilename);
    }

    public Pipe(String[][] commands,
            String inFilename, String outFilename, boolean append, String errFilename)
            throws Exception {
        this(commands);
        setInFile(inFilename);
        setOutFile(outFilename, append);
        setErrFile(errFilename);
    }

    public Pipe(String[][] commands, int inFildes, int outFildes)
            throws Exception {
        this(commands);
        i_fd = inFildes;
        o_fd = outFildes;
    }

    protected Pipe() {
        i_fd = -1;
        o_fd = -1;
        writer_fd = -1;
        reader_fd = -1;
        pipe_pid = -1;
        cmds = null;
    }

    public void start()
            throws Exception {
        startPipe(cmds, i_fd, o_fd, i_name, o_name, o_append, e_name);
    }

    void setInFile(String inFilename)
            throws Exception {
        if (inFilename != null) {
            if (!new File(inFilename).canRead()) {
                throw new IOException("can't read '" + inFilename + "'");
            }
            i_name = inFilename.getBytes();
            i_fd = -1;
        } else {
            i_name = null;
        }
    }

    void setOutFile(String outFilename, boolean append)
            throws Exception {
        if (outFilename != null) {
            File outFile = new File(outFilename);
            if (!Systoolbox.canCreat(outFile)) {
                throw new IOException("can't write '" + outFilename + "'");
            }
            o_name = outFilename.getBytes();
            o_append = append;
            o_fd = -1;
        } else {
            o_name = null;
            o_append = false;
        }
    }

    void setErrFile(String errFilename)
            throws Exception {
        if (errFilename != null) {
            if (!Systoolbox.canCreat(new File(errFilename))) {
                throw new IOException("can't write '" + errFilename + "'");
            }
            e_name = errFilename.getBytes();
        } else {
            e_name = null;
        }
    }

    public void setRunDir(String dir) {
        runDir = dir == null ? null : dir.getBytes();
    }

    public void setPath(String p) {
        path = p == null ? null : p.getBytes();
    }

    public BufferedFDOutputStream getOutputStream()
            throws Exception {
        if (writer_fd < 0) {
            throw new IOException("Pipe input not redirected from this process");
        }
        BufferedFDOutputStream result = new BufferedFDOutputStream(writer_fd);
        writer_fd = -1;
        return result;
    }

    public BufferedFDInputStream getInputStream()
            throws Exception {
        if (reader_fd < 0) {
            throw new IOException("Pipe output not redirected into this process");
        }
        BufferedFDInputStream result = new BufferedFDInputStream(reader_fd);
        reader_fd = -1;
        return result;
    }

    public int yieldUntilExit() {
        int status;
        while ((status = checkForExit()) == -1) {
            Thread.currentThread().yield();
        }
        return status;
    }

    public static void main(String[] argv)
            throws Exception {
        System.load("Native/Linux/Pipe");
        System.load("Native/Linux/BufferedFDInputStream");
        System.load("Native/Linux/BufferedFDOutputStream");
        System.err.println("\nrunning a pipe: ls -F | rev | sort\n");
        String[][] cmd = {{"ls", "-F"}, {"rev"}, {"sort"}};
        Pipe pipe = new Pipe(cmd, 0, -1,
                null, null, null);
        pipe.start();
        try {
            InputStream i = pipe.getInputStream();
            int c;
            while ((c = i.read()) > 0) {
                System.out.write(c);
            }
        } catch (EOFException e) {
        } catch (IOException e) {
            System.err.println("Exception: " + e.getMessage());
        }
        System.err.println("\nexit status: " + pipe.waitForExit() + "\n");

        System.err.println("\nlinking two pipes: ( ls -F ) and ( rev | sort )\n");
        BufferedFDOutputStream linkStream;
        Pipe pipe1, pipe2;
        pipe2 = new Pipe(new String[][]{{"rev"}, {"sort"}},
                null, null, null);
        pipe2.start();
        linkStream = pipe2.getOutputStream();
        pipe1 = new Pipe(new String[][]{{"ls", "-F"}},
                -1, linkStream.getFD(),
                "/dev/null", null, null);
        pipe1.start();
        linkStream.close();
        try {
            InputStream i = pipe2.getInputStream();
            int c;
            while ((c = i.read()) > 0) {
                System.out.write(c);
            }
        } catch (EOFException e) {
        } catch (IOException e) {
            System.err.println("Exception: " + e.getMessage());
        }
        System.err.println("\nls exit status: " + pipe1.waitForExit());
        System.err.println("rev exit status: " + pipe2.waitForExit() + "\n");
    }

    protected void finalize() throws Throwable {
        finalizePipe();
        super.finalize();
    }
}

