
package cage.viewer;

import cage.CaGe;
import cage.CaGeResult;
import cage.GeneratorInfo;
import cage.ResultPanel;
import cage.writer.AbstractChemicalWriter;
import cage.writer.WriterFactory;
import java.awt.Color;
import java.awt.Frame;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextField;
import javax.swing.Timer;
import lisken.systoolbox.BufferedFDInputStream;
import lisken.systoolbox.BufferedFDOutputStream;
import lisken.systoolbox.Pipe;
import lisken.uitoolbox.UItoolbox;


public class RasmolViewer implements CaGeViewer
{
  static private AbstractChemicalWriter pdbWriter;
  static private Boolean rasmolTrialResult = null;
  static private String runDir, initCmd;
  static private final boolean debug = true;
  static private final String rasmolFilename = ".rasmol.input";
  static private final File rasmolFile;

  static {
    pdbWriter = (AbstractChemicalWriter) WriterFactory.createCaGeWriter("PDB");
    pdbWriter.setDimension(3);
    runDir = CaGe.config.getProperty("CaGe.Generators.RunDir");
    initCmd = "source "
     + CaGe.installDirectory() + File.separator + ".rasmolrc"
     + "\n";
    rasmolFile = new File(runDir + File.separator + rasmolFilename);
  }

  public static boolean isAvailable(int dimension)
  {
    if (dimension == 3) {
      return tryRasmol();
    } else {
      return false;
    }
  }

  static boolean tryRasmol()
  {
    if (rasmolTrialResult == null) {
      try {
	Pipe trialPipe = new Pipe(
	 new String[][] { { "rasmol", "-nodisplay" } },
	 null, "/dev/null", "/dev/null");
	trialPipe.setRunDir(runDir);
	trialPipe.start();
	BufferedFDOutputStream rasmolInput = trialPipe.getOutputStream();
	rasmolInput.write("quit\n");
	rasmolInput.close();
	int status = trialPipe.waitForExit();
	rasmolTrialResult = new Boolean(status <= 0);
      } catch (Exception ex) {
	rasmolTrialResult = new Boolean(false);
      }
    }
    return rasmolTrialResult.booleanValue();
  }

  Pipe rasmolPipe = null;
  BufferedFDOutputStream rasmolInput;
  BufferedFDInputStream rasmolOutput;
  RasmolWatcher rasmolWatcher;
  ResultPanel resultPanel;

  public RasmolViewer()
  {
    if (CaGe.expertMode) return;
    UItoolbox.showTextInfo("Rasmol warning",
"Warning:\n" +
"The Rasmol viewer tries to auto-calculate connections between atoms.\n" +
"With certain 'non-chemical' graph classes, the displayed result may be\n" +
"different from the graph produced by the generator.\n" +
"\n"+
"Also, you might see no Rasmol window if your installed rasmol version\n" +
"refuses to work on your X display - we have no way of testing for that.");
  }

  public void setGeneratorInfo(GeneratorInfo generatorInfo)
  {
    pdbWriter.setGeneratorInfo(generatorInfo);
  }

  public void setDimension(int dimension)
  {
    if (dimension != 3) {
      throw new RuntimeException("RasmolViewer is for 3D only");
    }
  }

  public int getDimension()
  {
    return 3;
  }

  public void outputResult(CaGeResult result)
  {
    try {
      if (rasmolPipe == null || rasmolPipe.checkForExit() >= 0) {
        rasmolPipe = new Pipe(new String[][] { { "rasmol" } },
	 null, null, "/dev/null");
	rasmolPipe.setRunDir(runDir);
	rasmolPipe.start();
	rasmolInput = rasmolPipe.getOutputStream();
	rasmolOutput = rasmolPipe.getInputStream();
	rasmolWatcher = new RasmolWatcher(rasmolOutput, rasmolInput);
      }
      OutputStream rasmolData = new FileOutputStream(rasmolFile);
      pdbWriter.setOutputStream(rasmolData);
      pdbWriter.outputResult(result);
      rasmolData.close();
      rasmolWatcher.abort();
      rasmolInput.write("\nzap\n");
      rasmolInput.write(initCmd);
      rasmolInput.flush();
      rasmolInput.write("load " + rasmolFilename + "\n");
      rasmolInput.write(initCmd);
      rasmolInput.flush();
    } catch (Exception ex) {
      resultPanel.showException(ex, "exception with Rasmol communication");
    }
  }

  public void stop()
  {
    if (rasmolPipe != null) {
      try {
	rasmolInput.write("\nquit\n");
      } catch (IOException ex) {
      } finally {
	rasmolInput.close();
	rasmolInput = null;
	rasmolFile.delete();
	rasmolWatcher.end();
	rasmolOutput.close();
      }
      rasmolPipe = null;
    }
  }

  public void setResultPanel(ResultPanel resultPanel)
  {
    this.resultPanel = resultPanel;
  }

  public void setVisible(boolean isVisible)
  {
  }
}

class RasmolWatcher extends WindowAdapter implements ActionListener
{
  static private final int watchInterval =
   CaGe.getCaGePropertyAsInt("RasmolViewer.WatchInterval", 500);

  BufferedFDInputStream rasmolOutput;
  BufferedFDOutputStream rasmolInput;
  Timer rasmolTimer;
  StringBuffer lastLine;
  int lastByte;
  JLabel hint, question;
  JTextField responseField;
  JPanel content;
  JDialog rasmolDialog;

  public RasmolWatcher(
   BufferedFDInputStream rasmolOutput,
   BufferedFDOutputStream rasmolInput)
  {
    this.rasmolOutput = rasmolOutput;
    this.rasmolInput  = rasmolInput;
    lastByte = '\n';
    lastLine = new StringBuffer();
    hint = new JLabel("Rasmol is asking a question.");
    hint.setForeground(Color.gray);
    question = new JLabel();
    question.setForeground(Color.black);
    responseField = new JTextField();
    responseField.addActionListener(this);
    content = new JPanel();
    BoxLayout contentLayout = new BoxLayout(content, BoxLayout.Y_AXIS);
    content.setLayout(contentLayout);
    content.setAlignmentX(0.0f);
    content.add(hint);
    content.add(Box.createVerticalStrut(3));
    content.add(question);
    content.add(Box.createVerticalStrut(10));
    content.add(responseField);
    content.setBorder(BorderFactory.createEmptyBorder(20, 20, 20, 20));
    rasmolDialog = null;
    rasmolTimer = new Timer(watchInterval, this);
    rasmolTimer.start();
  }

  public void end()
  {
    rasmolTimer.stop();
    abort();
  }

  public void abort()
  {
    if (rasmolDialog != null) {
      unloadDialog();
    }
  }

  public void actionPerformed(ActionEvent e)
  {
    Object source = e.getSource();
    if (source == rasmolTimer) {
      checkRasmolOutput();
    } else if (source == responseField) {
      processDialog();
    }
  }

  void checkRasmolOutput()
  {
    if (rasmolOutput.available() <= 0) return;
    do {
      int currentByte;
      try {
	currentByte = rasmolOutput.read();
      } catch (IOException ex) {
	break;
      }
      if (currentByte == -1) {
	break;
      }
      if (currentByte == '\n') {
	lastLine.setLength(0);
      } else {
	lastLine.append((char) currentByte);
	if (! (Character.isSpaceChar((char) currentByte))
	 || Character.isWhitespace((char) currentByte)) {
	  lastByte = currentByte;
	}
      }
    } while (rasmolOutput.available() > 0);
    if (lastByte == ':') {
      showDialog();
    }
  }

  void showDialog()
  {
    responseField.setText("");
    question.setText(lastLine.toString());
    rasmolDialog = new JDialog((Frame)null, "Rasmol response required", false);
    rasmolDialog.setContentPane(content);
    rasmolDialog.pack();
    UItoolbox.centerOnScreen(rasmolDialog);
    responseField.requestFocus();
    rasmolDialog.addWindowListener(this);
    rasmolDialog.setVisible(true);
  }

  void processDialog()
  {
    unloadDialog();
    String response = responseField.getText();
    if (response == null) response = "";
    try {
      rasmolInput.write(response + "\n");
      rasmolInput.flush();
    } catch (IOException ex) {
    }
  }

  void unloadDialog()
  {
    rasmolDialog.setVisible(false);
    rasmolDialog.setContentPane(new JPanel());
    rasmolDialog.dispose();
    rasmolDialog = null;
  }

  public void windowClosing(WindowEvent e)
  {
    responseField.setText("");
    actionPerformed(new ActionEvent(responseField, 0, "", 0));
  }
}

