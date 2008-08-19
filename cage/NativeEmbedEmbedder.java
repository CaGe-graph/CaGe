
package cage;


import lisken.systoolbox.*;


public class NativeEmbedEmbedder extends Embedder
{
  static final boolean debug = false;

  boolean isConstant = false;
  long nEmbed2DNew = 0, nReembed2D = 0;
  long nEmbed3DNew = 0, nEmbed3DEmbedded = 0;
  long nEmbedPID = 0;
  String[][] embed2DOrigCmd, embed2DNewCmd;
  String[][] embed3DOrigCmd, embed3DNewCmd, embed3DEmbeddedCmd;
  byte[] runDir, path;
  float intensityFactor = 1.0f;
  int embeddedMode = IGNORE_OLD_EMBEDDING;
  String[][] reembed2DCmd;
  int reembed2DArg;
  byte[] errFilenameBytes;

  native long nCompileCommands(Object[] cmds, byte[] runDir, byte[] path);
  native void nEmbed2D(long nGraph, long nEmbed2D)
   throws Exception;
  native void nEmbed3D(long nGraph, long nEmbed3D, long nEmbed3DEmbedded)
   throws Exception;
  native void nFinalize(long commands);
  native void nStop(long pid);

  public NativeEmbedEmbedder(String[][] embed2D, String[][] embed3D)
  {
    this.embed2DOrigCmd      = embed2D;
    this.embed3DOrigCmd      = embed3D;
    computeEmbedders();
  }

  public NativeEmbedEmbedder(boolean isConstant,
   String[][] embed2D, String[][] embed3D)
  {
    this.isConstant          = isConstant;
    this.embed2DOrigCmd      = embed2D;
    this.embed3DOrigCmd      = embed3D;
    computeEmbedders();
  }

  public NativeEmbedEmbedder
   (boolean isConstant,
    String[][] embed2D, String[][] embed3D,
    float intensityFactor, int embeddedMode)
  {
    this.isConstant          = isConstant;
    this.embed2DOrigCmd      = embed2D;
    this.embed3DOrigCmd      = embed3D;
    this.intensityFactor     = intensityFactor;
    this.embeddedMode        = embeddedMode;
    computeEmbedders();
  }

  public void setConstant(boolean isConstant)
  {
    this.isConstant          = isConstant;
  }

  public boolean isConstant()
  {
    return isConstant;
  }

  public void setEmbed2D(String[][] embed2D)
  {
    if (debug) System.err.println("{ setEmbed2D");
    this.embed2DOrigCmd = embed2D;
    isConstant = false;
    compute2DEmbedders();
    if (debug) System.err.println("} setEmbed2D");
  }

  public void setEmbed3D(String[][] embed3D)
  {
    if (debug) System.err.println("{ setEmbed3D");
    this.embed3DOrigCmd = embed3D;
    isConstant = false;
    compute3DEmbedders();
    if (debug) System.err.println("} setEmbed3D");
  }

  public void setRunDir(String runDir)
  {
    if (debug) System.err.println("{ setRunDir");
    this.runDir = runDir == null ? null : runDir.getBytes();
    computeEmbedders();
    if (debug) System.err.println("} setRunDir");
  }

  public void setPath(String path)
  {
    if (debug) System.err.println("{ setPath");
    this.path = path == null ? null : path.getBytes();
    computeEmbedders();
    if (debug) System.err.println("} setPath");
  }

  public void setIntensityFactor(float factor)
  {
    if (debug) System.err.println("{ setIntensity");
    this.intensityFactor = factor;
    isConstant = false;
    computeEmbedders();
    if (debug) System.err.println("} setIntensity");
  }

  public void setMode(int mode)
  {
    if (debug) System.err.println("{ setMode");
    this.embeddedMode = mode;
    isConstant = false;
    if (nEmbed3DEmbedded == nEmbed3DNew) {
      nEmbed3DEmbedded = 0;
    }
    computeEmbeddedEmbedders();
    if (debug) System.err.println("} setMode");
  }

  public int getMode()
  {
    return embeddedMode;
  }

  void computeEmbedders()
  {
    compute2DEmbedders();
    compute3DEmbedders();
  }

  void compute2DEmbedders()
  {
    embed2DNewCmd = setIntensity(embed2DOrigCmd, intensityFactor);
    nFinalize(nEmbed2DNew);
    nEmbed2DNew = nCompileCommands(Systoolbox.stringsToBytes(embed2DNewCmd),
     runDir, path);
    prepareReembed2D(embed2DNewCmd);
  }

  void compute3DEmbedders()
  {
    if (nEmbed3DEmbedded == nEmbed3DNew) {
      nEmbed3DEmbedded = 0;
    }
    embed3DNewCmd = setIntensity(embed3DOrigCmd, intensityFactor);
    nFinalize(nEmbed3DNew);
    nEmbed3DNew = nCompileCommands(Systoolbox.stringsToBytes(embed3DNewCmd),
     runDir, path);
    computeEmbeddedEmbedders();
  }

  void computeEmbeddedEmbedders()
  {
    nFinalize(nEmbed3DEmbedded);
    nEmbed3DEmbedded = nEmbed3DNew;
    embed3DEmbeddedCmd = embed3DNewCmd;
    switch (embeddedMode)
    {
      case KEEP_OLD_EMBEDDING:
	embed3DEmbeddedCmd = null;
	nEmbed3DEmbedded = 0;
	break;
      case REFINE_OLD_EMBEDDING:
	embed3DEmbeddedCmd = setRefine(embed3DNewCmd);
	if (debug) {
	  String sep = "refined embedder: ";
	  for (int i = 0; i < embed3DEmbeddedCmd.length; ++i)
	  {
	    System.err.print(sep);
	    sep = "";
	    for (int j = 0; j < embed3DEmbeddedCmd[i].length; ++j)
	    {
	      System.err.print(sep);
	      System.err.print(embed3DEmbeddedCmd[i][j]);
	      sep = " ";
	    }
	    sep = " | ";
	  }
	  System.err.print("\n");
	}
	nEmbed3DEmbedded = nCompileCommands(
	 Systoolbox.stringsToBytes(embed3DEmbeddedCmd), runDir, path);
        break;
    }
  }

  public String[][] getEmbed2DNew()
  {
    return embed2DNewCmd;
  }

  public String[][] getEmbed3DNew()
  {
    return embed3DNewCmd;
  }

  public String[][] getEmbed3DRefine()
  {
    return embed3DEmbeddedCmd;
  }


  static String[][] setIntensity(String[][] embed, float factor)
  {
    if (factor == 1.0f) return embed;
    String[] embedCmd = (String[]) embed[0].clone();
    boolean addIntensity = true;
    for (int i = 0; i < embedCmd.length; ++i)
    {
      String arg = embedCmd[i];
      if (arg.startsWith("-f")) {
	String intensities, prefix;
	float intensity[] = new float[] { 1.0f, 1.0f, 1.0f };
	if (arg.equals("-f") && i+1 < embedCmd.length) {
	  intensities = embedCmd[++i] + ",";
	  prefix = "";
	} else {
	  intensities = embedCmd[i].substring(2) + ",";
	  prefix = "-f";
	}
	try {
	  int pos;
	  for (int j = 0; j < 3; ++j)
	  {
	    pos = intensities.indexOf(',');
	    intensity[j] = Float.valueOf(intensities.substring(0, pos)).floatValue();
	    intensities = intensities.substring(pos+1);
	  }
	} catch (Exception ex) {
	}
	embedCmd[i] = prefix + intensity[0] + "," + intensity[1] + ","
	 + intensity[2] * factor;
	addIntensity = false;
      }
    }
    if (addIntensity) {
      String[] newEmbedCmd = new String[embedCmd.length + 1];
      System.arraycopy(embedCmd, 0, newEmbedCmd, 0, embedCmd.length);
      newEmbedCmd[embedCmd.length] = "-f1,1," + factor;
      embedCmd = newEmbedCmd;
    }
    return new String[][] { embedCmd };
  }

  static String[][] setRefine(String[][] embed)
  {
    if (debug) System.err.println("{ setRefine");
    String[] embedCmd = embed[0];
    boolean addInitial = true;
    for (int i = 0; i < embedCmd.length; ++i)
    {
      String arg = embedCmd[i];
      if (arg.startsWith("-i")) {
	if (arg.equals("-i") && i+1 < embedCmd.length) {
	  embedCmd[++i] = "k";
	  if (debug) System.err.println("-i k");
	} else {
	  embedCmd[i] = "-ik";
	  if (debug) System.err.println("-ik");
	}
	addInitial = false;
      }
    }
    if (addInitial) {
      if (debug) System.err.println("adding -ik");
      String[] newEmbedCmd = new String[embedCmd.length + 1];
      System.arraycopy(embedCmd, 0, newEmbedCmd, 0, embedCmd.length);
      newEmbedCmd[embedCmd.length] = "-ik";
      embedCmd = newEmbedCmd;
    }
    if (debug) System.err.println("} setRefine");
    return new String[][] { embedCmd };
  }

  public void embed2D(EmbeddableGraph graph)
   throws Exception
  {
    NativeEmbeddableGraph nGraph = (NativeEmbeddableGraph) graph;
    nEmbed2D(nGraph.nGraph, nEmbed2DNew);
  }

  public void embed3D(EmbeddableGraph graph)
   throws Exception
  {
    NativeEmbeddableGraph nGraph = (NativeEmbeddableGraph) graph;
    nEmbed3D(nGraph.nGraph, nEmbed3DNew, nEmbed3DEmbedded);
  }

  void prepareReembed2D(String[][] embed2D)
  {
    nFinalize(nReembed2D);
    nReembed2D = 0;
    if (embed2D.length == 1 && embed2D[0].length >= 1 && embed2D[0][0].equals("embed")) {
      reembed2DArg = embed2D[0].length;
      reembed2DCmd = new String[1][reembed2DArg + 1];
      System.arraycopy(embed2D[0], 0, reembed2DCmd[0], 0, reembed2DArg);
    } else {
      reembed2DArg = 1;
      reembed2DCmd = new String[1][reembed2DArg + 1];
      reembed2DCmd[0][0] = "embed";
    }
  }

  public void reembed2D(EmbeddableGraph graph)
   throws Exception
  {
    reembed2DCmd[0][reembed2DArg] = "-b" + e1 + "," + e2;
    nReembed2D = nCompileCommands(Systoolbox.stringsToBytes(reembed2DCmd),
     runDir, path);
    NativeEmbeddableGraph nGraph = (NativeEmbeddableGraph) graph;
    nEmbed2D(nGraph.nGraph, nReembed2D);
  }

  public String getDiagnosticOutput()
  {
    if (errFilenameBytes != null) {
      return Systoolbox.getFileContent(new String(errFilenameBytes), true);
    } else {
      return null;
    }
  }

  public synchronized void abort()
  {
    if (nEmbedPID != 0) {
      nStop(nEmbedPID);
    }
  }

  public void finalize()
  {
    nFinalize(nEmbed2DNew);
    nFinalize(nReembed2D);
    nFinalize(nEmbed3DNew);
    if (nEmbed3DEmbedded != nEmbed3DNew) {
      nFinalize(nEmbed3DEmbedded);
    }
    nEmbed2DNew = nReembed2D = nEmbed3DNew = nEmbed3DEmbedded = 0;
  }
}
