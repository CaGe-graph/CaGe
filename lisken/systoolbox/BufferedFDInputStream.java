
package lisken.systoolbox;


import java.io.*;


public class BufferedFDInputStream extends InputStream
{
  private long file;
  private int  fd = -1;

  private native long init(int fd);
  private native long openFile(byte[] filename);
  private native int nRead(long file);
  private native void nUnread(long file, int b);
  private native int nNextByte(long file);
  private native void nClose(long file);

/*
  static {
    System.loadLibrary("BufferedFDInputStream");
  }
*/

  public BufferedFDInputStream(int fildes)
   throws IOException
  {
    fd = fildes;
    file = init(fd);
  }

  public BufferedFDInputStream(String filename)
   throws IOException
  {
    file = openFile(filename.getBytes());
  }

  public int read()
   throws IOException
  {
    return nRead(file);
  }

  public void unread(int b)
   throws IOException
  {
    nUnread(file, b);
  }

  public int nextByte()
  {
    return nNextByte(file);
  }

  public int available()
  {
    return nextByte() < 0 ? 0 : 1;
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
    return "BufferedFDInputStream[" + fd + "]";
  }
}

