
package lisken.systoolbox;


import java.io.*;


public class BufferedFDOutputStream extends OutputStream
{
  private long file;
  private int  fd = -1;

  private native long init(int fd);
  private native long openFile(byte[] filename, boolean append);
  private native void nWrite(long file, int b);
  private native void nFlush(long file);
  private native void nClose(long file);

/*
  static {
    System.loadLibrary("BufferedFDOutputStream");
  }
*/

  public BufferedFDOutputStream(int fildes)
   throws IOException
  {
    fd = fildes;
    file = init(fd);
  }

  public BufferedFDOutputStream(String filename)
   throws IOException
  {
    this(filename, false);
  }

  public BufferedFDOutputStream(String filename, boolean append)
   throws IOException
  {
    file = openFile(filename.getBytes(), append);
  }

  public void write(int b)
   throws IOException
  {
    nWrite(file, b);
  }

  public void write(String s)
   throws IOException
  {
    write(s.getBytes());
  }

  public void flush()
  {
    nFlush(file);
  }

  public void close()
  {
    nClose(file);
    file = 0;
  }

  public int getFD()
  {
    return fd;
  }

  protected void finalize()
   throws Throwable
  {
    close();
    super.finalize();
  }

  public String toString()
  {
    return "BufferedFDOutputStream[" + fd + "]";
  }
}

