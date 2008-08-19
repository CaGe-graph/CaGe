
package cage;

/**
 * Title:        CaGe
 * Description:
 * Copyright:    Copyright (c) 2001
 * Company:      FSP Mathematisierung, Bielefeld University
 * @author Sebastian Lisken
 * @version
 */

import java.io.*;
import java.util.*;
import java.beans.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import cage.viewer.*;
import cage.writer.*;
import lisken.uitoolbox.*;
import lisken.systoolbox.*;
import org.SysInfo;


public class CaGe
 implements ActionListener
{
  public static final String title = " CaGe - Chemical & abstract Graph environment ";
  public static final String configFile = "CaGe.ini";
  public static final boolean expertMode;
  public static final boolean nativesAvailable;
  public static final String osName;
  public static int graphNoDigits = 6;
  static final String installDirProperty = "CaGe.InstallDir";

  public static Properties config;

  static Hashtable substitutedNames;
  static Properties systemProperties;
  static Wizard wizard;
  static CaGeListener listener = new CaGeListener();

  static public int generators;
  static public int lastGeneratorChoice = -1;
  static public String[] generator;
  static AbstractButton[] generatorButton;
  static boolean rememberPanels;
  static Hashtable generatorPanels, outputPanels;

  private static FoldnetThread foldnetThread;
  // private static CaGeFoldnetDialog foldnetDialog;

  private static BackgroundWindow backgroundWindow;

  static {
    boolean nativesAvailableValue = false, expertModeValue = false;
    String osNameValue = null;
    try {
      System.runFinalizersOnExit(true);
      osNameValue = SysInfo.get("os.name");

      // Get configuration
      config = new Properties();
      try {
	InputStream configInput =
	 ClassLoader.getSystemResourceAsStream(configFile);
	if (configInput == null) {
	  System.err.println("CaGe: can't find configuration file '" + configFile + "'");
	  System.exit(1);
	}
	config.load(configInput);
      } catch(Exception e) {
	e.printStackTrace();
	System.exit(1);
      }

      // prepare config substitution mechanism
      substitutedNames = new Hashtable();
      String installDir = System.getProperty(installDirProperty);
      if (installDir == null) {
        installDir = System.getProperty("user.dir");
      } else {
	installDir = Systoolbox.makeAbsolutePath(installDir);
      }
      systemProperties = new Properties();
      systemProperties.put(installDirProperty, installDir);

      // load CaGe's native library
      String libDir = null;
      try {
        libDir = substituteConfigValue("CaGe.Native.LibDir");
      } catch (UnsatisfiedLinkError er) {
        System.err.println("CaGe: config value 'CaGe.Native.LibDir' must not contain ${...} substitutions");
	System.exit(1);
      }
      config.put("CaGe.Native.FullLibDir",
       Systoolbox.makeAbsolutePath(libDir) + File.separator +
       osNameValue + File.separator);
      try {
/*
	if (osNameValue.equals("Windows")) {
          System.load(installDir + File.separator + "cygwin1.dll");
        }
*/
        loadNativeLibrary("CaGe");
	nativesAvailableValue = true;
      } catch (Throwable t) {
	System.err.println("Error loading native library: " + t);
	System.err.println("Error message: " + t.getMessage());
	nativesAvailableValue = false;
      }

      Enumeration names = config.propertyNames();
      while (names.hasMoreElements())
      {
	substituteConfigValue((String) names.nextElement());
      }

      // build system-specific path list
      String pathList;
      if ((pathList = config.getProperty("CaGe.Generators.Path")) != null) {
	String[] pathArray = Systoolbox.stringToArray(pathList);
	config.put("CaGe.Generators.Path",
	 Systoolbox.join(pathArray, File.pathSeparator));
      }

      expertModeValue = Systoolbox.parseBoolean(config.getProperty("CaGe.ExpertMode"), false);
/*
      if (expertModeValue) {
	config.save(System.out, "CaGe configuration");
      }
*/

      graphNoDigits =
       Integer.parseInt(config.getProperty("CaGe.GraphNoDigits"));

    } catch (Throwable t) {
      t.printStackTrace();
      System.exit(1);
    }
    nativesAvailable = nativesAvailableValue;
    expertMode = expertModeValue;
    osName = osNameValue;
  }

  // perform configuration substitution on a name from CaGe.ini
  // (substitution syntax is described in that file)
  static String substituteConfigValue(String name)
  {
    return substituteConfigValue(name, false);
  }
  static String substituteConfigValue(String name, boolean throwStackOverflow)
  {
    String value = config.getProperty(name);
    if (substitutedNames.containsKey(name)) {
      return value;
    }
    String[] substStartStrings = { "${", "$[", "$(" };
    String[] substEndStrings = { "}", "]", ")" };
    int pos1, pos2 = -1;
    while ((pos1 = Systoolbox.firstIndexOf(value, substStartStrings, pos2 + 1)) >= 0)
    {
      int index = Systoolbox.foundSubstringIndex();
      // if there is another $ before the substitution marker,
      // strip one if the $s and leave everything else unchanged
      if (pos1 - 1 > pos2 && value.charAt(pos1 - 1) == '$') {
	value = value.substring(0, pos1)
	 + value.substring(pos1 + 1);
	pos2 = pos1;
      // ${name} is substituted by a value
      } else if ((pos2 = value.indexOf(substEndStrings[index], pos1)) >= 0) {
	String substName = value.substring(pos1 + 2, pos2);
	String substValue = null;
	try {
	  substValue =
	   index == 0 ? Systoolbox.getenv(substName) :
	   index == 1 ? getSystemProperty(substName) :
	   index == 2 ? substituteConfigValue(substName, true) :
	   null;
	} catch (StackOverflowError er) {
	  if (throwStackOverflow) throw er;
	  System.err.println("CaGe: recursive definition in " + configFile);
	  System.err.println("- problem found starting with '" + name + "'");
	  System.exit(1);
	}
	if (substValue == null) substValue = "";
	value = value.substring(0, pos1)
	 + substValue + value.substring(pos2 + 1);
	pos2 += substValue.length() - substName.length();
	pos2 -= substStartStrings[index].length() + substEndStrings[index].length();
      }
    }
    config.put(name, value);
    substitutedNames.put(name, "");
    return value;
  }

  public static String getCaGeProperty(String name)
  {
    return config.getProperty(name);
  }

  public static int getCaGePropertyAsInt(String name, int defaultValue)
  {
    int intValue = defaultValue;
    try {
      String stringValue = config.getProperty(name);
      intValue = Integer.parseInt(stringValue);
    } catch (NullPointerException e) {
      System.err.println("Property '" + name + "' not present in .ini file");
    } catch (NumberFormatException e) {
      System.err.println("Property '" + name + "' in .ini file is not a number");
    }
    return intValue;
  }

  public static boolean getCaGePropertyAsBoolean(String name,
   boolean defaultValue)
  {
    boolean booleanValue = defaultValue;
    try {
      String stringValue = config.getProperty(name);
      if (stringValue.equalsIgnoreCase("true")) {
        booleanValue = true;
      } else if (stringValue.equalsIgnoreCase("false")) {
        booleanValue = false;
      } else if (stringValue.equalsIgnoreCase("yes")) {
        booleanValue = true;
      } else if (stringValue.equalsIgnoreCase("no")) {
        booleanValue = false;
      } else {
        booleanValue = Integer.parseInt(stringValue) != 0;
      }
    } catch (NullPointerException e) {
      System.err.println("Property '" + name + "' not present in .ini file");
    } catch (Exception e) {
      System.err.println("Property '" + name + "' in .ini file is not a boolean");
    }
    return booleanValue;
  }

  public static String getSystemProperty(String name)
  {
    return systemProperties.getProperty(name, System.getProperty(name));
  }

  public static String installDirectory()
  {
    return getSystemProperty(installDirProperty);
  }

  public static Wizard wizard()
  {
    return wizard;
  }

  public static WizardStage getWizardStage()
  {
    return wizard().getStage();
  }

  public static Window getWizardWindow()
  {
    return wizard().getWindow();
  }

  public static FoldnetThread foldnetThread()
  {
    return foldnetThread;
  }

  public static BackgroundWindow backgroundWindow()
  {
    return backgroundWindow;
  }

  private CaGe()
  {
    // set a few static variables
    rememberPanels = Systoolbox.parseBoolean(config.getProperty("CaGe.RememberSettings"), false);
    if (rememberPanels) {
      generatorPanels = new Hashtable();
      outputPanels = new Hashtable();
    }
    foldnetThread = new FoldnetThread();
    foldnetThread.setRunDir(config.getProperty("CaGe.Generators.RunDir"));
    foldnetThread.setPath(config.getProperty("CaGe.Generators.Path"));
    foldnetThread.start();
    // foldnetDialog = new CaGeFoldnetDialog(foldnetThread);
    backgroundWindow = new BackgroundWindow(foldnetThread);

    // build a panel with a buttons for each generator
    JPanel titlePanel = new JPanel();
    titlePanel.setLayout(new GridBagLayout());
    GridBagConstraints lc = new GridBagConstraints();
    int y = 0;
    lc.anchor = GridBagConstraints.WEST;
    lc.gridx = 0;
    lc.gridy = y++;
    lc.insets = new Insets(10, 30, 20, 250);
    JLabel choiceLabel = new JLabel("Choose a generator:");
    titlePanel.add(choiceLabel, lc);
    JPanel generatorsPanel = new JPanel();
    generatorsPanel.setLayout(new GridLayout(0, 1, 0, 20));
    GeneratorChoiceListener generatorChoiceListener = new GeneratorChoiceListener();
    Vector generatorVector = Systoolbox.stringToVector(config.getProperty("CaGe.Generators"));
    generators = generatorVector.size();
    generatorButton = new AbstractButton[generators];
    generator = new String[generators];
    for (int i = 0; i < generators; ++i)
    {
      generator[i] = (String) generatorVector.elementAt(i);
      generatorButton[i] = new JButton(config.getProperty(generator[i] + ".Title"));
      generatorButton[i].setBorder(BorderFactory.createEmptyBorder(5, 10, 5, 10));
      generatorButton[i].setActionCommand(Integer.toString(i));
      if (i < 10) generatorButton[i].setMnemonic(KeyEvent.VK_0 + (i+1) % 10);
      generatorButton[i].addActionListener(generatorChoiceListener);
      new PushButtonDecoration(generatorButton[i]);
      generatorsPanel.add(generatorButton[i]);
    }
    lc.anchor = GridBagConstraints.CENTER;
    lc.insets = new Insets(0, 20, 30, 20);
    lc.gridy = y++;
    titlePanel.add(generatorsPanel, lc);

    // start a wizard with this panel
    wizard = new Wizard(title);
    wizard.nextStage(titlePanel, this, null, null, null, null, Wizard.EXIT);
  }

  public void actionPerformed(ActionEvent e)
  {
    String cmd = e.getActionCommand();
    if (cmd.equals(Wizard.SHOWING)) {
      if (lastGeneratorChoice >= 0) {
        generatorButton[lastGeneratorChoice].requestFocus();
      }
    } else if (cmd.equals(Wizard.EXIT)) {
      exit();
      return;
    }
    wizard.actionPerformed(e);
  }

  public static void exit()
  {
    if (backgroundWindow.active()) {
      backgroundWindow.askForExit();
    } else {
      foldnetThread.exit();
      try {
	new File(CaGe.config.getProperty("CaGe.Generators.ErrFile")).delete();
      } catch (Exception ex) {
      }
      System.exit(0);
    }
  }

/*
  public static void exit()
  {
    if (! foldnetThread().isAlive()) {
      System.exit(0);
    } else if (foldnetThread().tasksLeft() > 0) {
      foldnetThread.addPropertyChangeListener(foldnetDialog);
      foldnetThread.fireTasksChanged();
      foldnetDialog.setVisible(true);
    } else {
      foldnetThread.last();
      try {
	foldnetThread.join();
      } catch (InterruptedException ex) {
      }
      System.exit(0);
    }
  }
*/

  public static void loadNativeLibrary(String libName)
  {
    System.load(
     (String) config.getProperty("CaGe.Native.FullLibDir") + libName);
  }

  public static void main(String[] args)
  {
    new CaGe();
  }
}


/*
class CaGeFoldnetDialog extends FlaggedJDialog
 implements PropertyChangeListener, Runnable
{
  FoldnetThread foldnetThread;
  boolean aborted = false;

  JPanel msgPanel = new JPanel();
  JLabel msg1 = new JLabel();
  JLabel msg2 = new JLabel();
  JButton cancelButton = new JButton();
  JButton exitButton = new JButton();
  JButton abortButton = new JButton();

  public CaGeFoldnetDialog(FoldnetThread foldnetThread)
  {
    super((Frame) null, "waiting for folding nets", true);
    setNearComponent(CaGe.getWizardWindow());
    this.foldnetThread = foldnetThread;
    cancelButton.setText("Cancel");
    setCancelButton(cancelButton);
    exitButton.setText("Exit");
    setDefaultButton(exitButton);
    abortButton.setText("Abort current");
    abortButton.addActionListener(this);
    Font font = msg1.getFont();
    font = new Font(
     font.getName(),
     font.getStyle() & ~ Font.BOLD,
     font.getSize());
    abortButton.setFont(font);
    msg1.setText("All folding nets have been made.");
    msg1.setFont(font);
    msg1.setAlignmentX(0.0f);
    msg2.setText("\u00a0");
    msg2.setFont(font);
    msg2.setAlignmentX(0.0f);
    JPanel buttonPanel = new JPanel();
    buttonPanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
    buttonPanel.add(exitButton);
    buttonPanel.add(Box.createHorizontalStrut(10));
    buttonPanel.add(cancelButton);
    buttonPanel.add(Box.createHorizontalStrut(10));
    buttonPanel.add(abortButton);
    buttonPanel.setAlignmentX(0.0f);
    JPanel content = (JPanel) getContentPane();
    content.setLayout(new BoxLayout(content, BoxLayout.Y_AXIS));
    content.add(msg1);
    content.add(msg2);
    content.add(Box.createVerticalStrut(20));
    content.add(buttonPanel);
    content.setBorder(BorderFactory.createEmptyBorder(20, 20, 20, 20));
    pack();
  }

  public void setVisible(boolean visible)
  {
    if (visible) {
      aborted = false;
      abortButton.setEnabled(true);
      exitButton.requestFocus();
    }
    super.setVisible(visible);
  }

  public void propertyChange(PropertyChangeEvent e)
  {
    SwingUtilities.invokeLater(this);
  }

  public void run()
  {
    int left = foldnetThread.tasksLeft();
    if (left > 0) {
      msg1.setText(left + " folding net" + (left == 1 ? "" : "s") + " left to make.");
      msg2.setText("Waiting ...");
    } else {
      msg1.setText("All folding nets have been made.");
      if (aborted) {
	abortButton.setEnabled(false);
	msg2.setText("Click to exit or cancel.");
	exitButton.requestFocus();
      } else {
	msg2.setText("CaGe will now exit.");
	foldnetThread.last();
	finishAndExit();
      }
    }
  }

  public void actionPerformed(ActionEvent e)
  {
    Object source = e.getSource();
    if (source == cancelButton) {
      foldnetThread.removePropertyChangeListener(this);
    } else if (source == exitButton) {
      cancelButton.setEnabled(false);
      foldnetThread.removePropertyChangeListener(this);
      foldnetThread.last();
      foldnetThread.abortCurrent();
      finishAndExit();
    } else if (source == abortButton) {
      aborted = true;
      foldnetThread.abortCurrent();
      return;
    }
    super.actionPerformed(e);
  }

  private void finishAndExit()
  {
    try {
      foldnetThread.join();
    } catch (InterruptedException ex) {
    }
    setVisible(false);
    CaGe.exit();
  }
}
*/


class CaGeListener implements ActionListener
{
  public void actionPerformed(ActionEvent e)
  {
    String cmd = e.getActionCommand();
    if (cmd.equals(Wizard.CANCEL)) {
      if (CaGe.wizard().getStageNo() > 1) {
        CaGe.wizard().toStage(1, true);
        return;
      }
    } else if (cmd.equals(Wizard.EXIT)) {
      CaGe.exit();
      return;
    }
    CaGe.wizard().actionPerformed(e);
  }
}


class GeneratorChoiceListener implements ActionListener
{
  public void actionPerformed(ActionEvent e)
  {
    CaGe.getWizardWindow().setVisible(false);
    CaGe.lastGeneratorChoice = Integer.parseInt(e.getActionCommand());
    String generator = CaGe.generator[CaGe.lastGeneratorChoice];
    String configPanelName = CaGe.config.getProperty(generator + ".ConfigPanel");
    try {
      GeneratorPanel configPanel;
      if (CaGe.rememberPanels) {
        configPanel = (GeneratorPanel) CaGe.generatorPanels.get(configPanelName);
      } else {
        configPanel = null;
      }
      if (configPanel == null) {
	configPanel = (GeneratorPanel) Class.forName(configPanelName).newInstance();
      }
      if (CaGe.rememberPanels) {
        CaGe.generatorPanels.put(configPanelName, configPanel);
      }
      configPanel.setBorder(BorderFactory.createTitledBorder(BorderFactory.createCompoundBorder(
       BorderFactory.createCompoundBorder(
       BorderFactory.createEmptyBorder(10, 10, 10, 10),
       BorderFactory.createEtchedBorder()),
       BorderFactory.createEmptyBorder(20, 20, 20, 20)
      ), " Generator Options:  " + ((JButton) e.getSource()).getText() + " "));
      CaGe.wizard().nextStage(configPanel,
       new GeneratorParamsListener(configPanel, generator),
       Wizard.PREVIOUS, Wizard.NEXT, null, null, null);
    } catch (Exception ex) {
      ex.printStackTrace();
    }
  }
}


class GeneratorParamsListener implements ActionListener
{
  GeneratorPanel generatorPanel;
  String generatorName;

  public GeneratorParamsListener
   (GeneratorPanel generatorPanel, String generatorName)
  {
    this.generatorPanel = generatorPanel;
    this.generatorName = generatorName;
  }

  public void actionPerformed(ActionEvent e)
  {
    String cmd = e.getActionCommand();
    if (cmd.equals(Wizard.SHOWING)) {
      generatorPanel.showing();
    } else if (cmd.equals(Wizard.CANCEL)) {
      CaGe.getWizardStage().previousButton.doClick();
    } else if (cmd.equals(Wizard.NEXT)) {
      CaGe.getWizardWindow().setVisible(false);
      GeneratorInfo generatorInfo = generatorPanel.getGeneratorInfo();
      generatorInfo.setGeneratorName(
       CaGe.generatorButton[CaGe.lastGeneratorChoice].getText());
      OutputPanel outputPanel;
      if (CaGe.rememberPanels) {
        outputPanel = (OutputPanel) CaGe.outputPanels.get(generatorName);
      } else {
        outputPanel = null;
      }
      if (outputPanel == null) {
	outputPanel = new OutputPanel(generatorName);
      }
      if (CaGe.rememberPanels) {
        CaGe.outputPanels.put(generatorName, outputPanel);
      }
      outputPanel.setGeneratorInfo(generatorInfo);
      CaGe.wizard().nextStage(outputPanel,
       new OutputParamsListener(outputPanel),
       Wizard.PREVIOUS, "Start", null, Wizard.CANCEL, null);
    } else {
      CaGe.listener.actionPerformed(e);
    }
  }
}


class OutputParamsListener implements ActionListener
{
  OutputPanel outputPanel;

  public OutputParamsListener(OutputPanel outputPanel)
  {
    this.outputPanel = outputPanel;
  }

  public void actionPerformed(ActionEvent e)
  {
    String cmd = e.getActionCommand();
    if (cmd.equals(Wizard.SHOWING)) {
      outputPanel.showing();
    } else if (cmd.equals(Wizard.NEXT)) {
      CaGeStarter starter = new CaGeStarter(outputPanel);
      starter.start();
    } else {
      CaGe.listener.actionPerformed(e);
    }
  }
}


class CaGeStarter implements ActionListener
{
  OutputPanel outputPanel;
  GeneratorInfo generatorInfo;
  Vector viewers, writers, writeDests;
  int nViewers, nWriters;
  CaGePipe generatorPipe = null;
  ResultPanel resultPanel = null;
  boolean stopped;

  public CaGeStarter(OutputPanel outputPanel)
  {
    this.outputPanel = outputPanel;
  }

  public void start()
  {
    if (checkOutputOptions()) return;
    CaGe.getWizardWindow().setVisible(false);
    prepareGeneratorAndEmbedder();
    if (nViewers > 0) {
      resultPanel = new ResultPanel(
       generatorPipe, generatorInfo,
       outputPanel.requests2D(), outputPanel.requests3D(),
       viewers, writers);
      CaGe.wizard().nextStage(resultPanel, this,
       CaGe.expertMode ? Wizard.PREVIOUS : null,
       null, "Stop", Wizard.CANCEL, Wizard.EXIT, false);
    } else if (nWriters > 0) {
      BackgroundRunner backgroundRunner = new BackgroundRunner(
       generatorPipe, generatorInfo,
       outputPanel.requests2D(), outputPanel.requests3D(),
       writers, writeDests);
      CaGe.wizard().toStage(1, true);
      CaGe.backgroundWindow().addRunner(backgroundRunner);
    }
  }

  boolean checkOutputOptions()
  {
    generatorInfo = outputPanel.getGeneratorInfo();
    viewers = outputPanel.getViewers();
    writers = outputPanel.getWriters();
    writeDests = outputPanel.getWriteDestinations();
    try {
      setWriterOutputStreams();
    } catch (Exception ex) {
      CaGe.getWizardWindow().setVisible(true);
      UItoolbox.showTextInfo
       ("File/Pipe output failure",
"Some of your output destinations are invalid.\n" +
"The following exceptions have occurred:\n\n" +
ex.toString() + "\n" +
"\nPlease change your choices in the output options window.",
       CaGe.getWizardStage().nextButton);
      return true;
    }
    nViewers = viewers == null ? 0 : viewers.size();
    nWriters = writers == null ? 0 : writers.size();
    if (nViewers <= 0 && nWriters <= 0) {
      String viewerErrors = outputPanel.getViewerErrors();
      UItoolbox.showTextInfo("output failure",
"Failed to instantiate any of the selected viewers or graph formatters.\n" +
"Please change your choices in the output options window.\n" +
(viewerErrors == null ?  "" :
 "\nErrors during attempted instantiation:\n\n" + viewerErrors),
       CaGe.getWizardStage().nextButton);
      return true;
    }
    return false;
  }

  void prepareGeneratorAndEmbedder()
  {
    String runDir, path;
    runDir = CaGe.config.getProperty("CaGe.Generators.RunDir");
    path = CaGe.config.getProperty("CaGe.Generators.Path");
    String[][] generator, preFilter;
    Embedder embedder = generatorInfo.getEmbedder();
    embedder.setRunDir(runDir);
    embedder.setPath(path);
    generator = generatorInfo.getGenerator();
    preFilter = outputPanel.getPreFilter();
    if (preFilter != null) {
      String[][] newGenerator = new String[generator.length + preFilter.length][];
      System.arraycopy(generator, 0, newGenerator, 0, generator.length);
      System.arraycopy(preFilter, 0, newGenerator, generator.length, preFilter.length);
      generator = newGenerator;
    }
    try {
      generatorPipe = new NativeCaGePipe(generator,
       CaGe.config.getProperty("CaGe.Generators.ErrFile"));
      generatorPipe.setRunDir(runDir);
      generatorPipe.setPath(path);
    } catch (Exception e) {
      e.printStackTrace();
      System.exit(1);
    }
  }

  public void actionPerformed(ActionEvent e)
  {
    AbstractButton stopButton = CaGe.getWizardStage().finishButton;
    String cmd = e.getActionCommand();
    if (e.getSource() == resultPanel) {
      stopped = true;
      stopButton.setText("Restart");
      stopButton.setEnabled(true);
    } else if (cmd.equals(Wizard.SHOWING)) {
      resultPanel.setStopListener(this);
      stopped = false;
      resultPanel.start();
    } else if (cmd.equals(Wizard.FINISH)) {
      if (stopped) {
	stopButton.setEnabled(false);
	resultPanel.reset();
        stopButton.setText("Stop");
	stopButton.setEnabled(true);
	resultPanel.setStopListener(this);
	stopped = false;
	resetWriterOutputStreams();
	resultPanel.start();
      } else {
	stopButton.setEnabled(false);
	resultPanel.stop();
      }
    } else if (cmd.equals(Wizard.PREVIOUS) || cmd.equals(Wizard.CANCEL) || cmd.equals(Wizard.EXIT)) {
      resultPanel.setStopListener(null);
      stopButton.setEnabled(false);
      resultPanel.stop();
      stopped = true;
      resultPanel.reset();
      stopButton.setText("Restart");
      stopButton.setEnabled(true);
      CaGe.listener.actionPerformed(e);
    }
  }

  void setWriterOutputStreams()
   throws Exception
  {
    ExceptionGroup exceptionGroup = new ExceptionGroup();
    int n = Math.min(writers.size(), writeDests.size());
    for (int i = 0; i < n; ++i)
    {
      setWriterOutputStream(
       (CaGeWriter) writers.elementAt(i), (String) writeDests.elementAt(i),
       exceptionGroup);
    }
    if (exceptionGroup.size() > 0) {
      throw exceptionGroup;
    }
  }

  void resetWriterOutputStreams()
  {
    try {
      setWriterOutputStreams();
    } catch (Exception ex) {
      UItoolbox.showTextInfo
       ("File/Pipe output exceptions", ex.toString(), resultPanel);
    }
  }

  void setWriterOutputStream
   (CaGeWriter writer, String dest, ExceptionGroup exceptionGroup)
  {
    try {
      writer.setOutputStream(Systoolbox.createOutputStream(dest,
       CaGe.config.getProperty("CaGe.Generators.RunDir")));
    } catch (Exception ex) {
      exceptionGroup.add(ex);
    }
  }
}
