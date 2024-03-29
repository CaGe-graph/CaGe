package cage;

/**
 * Title:        CaGe
 * Description:
 * Copyright:    Copyright (c) 2001
 * Company:      FSP Mathematisierung, Bielefeld University
 * @author Sebastian Lisken
 * @version
 */
import cage.background.BackgroundWindow;
import cage.utility.Debug;
import java.awt.Color;

import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.GridLayout;
import java.awt.Insets;
import java.awt.Toolkit;
import java.awt.Window;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.io.File;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Properties;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.regex.PatternSyntaxException;
import javax.swing.AbstractButton;
import javax.swing.BorderFactory;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JPanel;

import lisken.systoolbox.Systoolbox;
import lisken.uitoolbox.PushButtonDecoration;
import lisken.uitoolbox.Wizard;
import lisken.uitoolbox.WizardStage;

import util.SysInfo;

public class CaGe implements ActionListener {

    private static final String title = " CaGe - Chemical & abstract Graph environment ";
    private static final String configFile = "CaGe.ini";
    public static final boolean expertMode;
    public static final boolean debugMode;
    public static final boolean nativesAvailable;
    private static final String osName;
    public static int graphNoDigits = 6;
    private static final String installDirProperty = "CaGe.InstallDir";
    private static final Properties config = new Properties();
    private static Map<String, String> substitutedNames;
    private static final Properties systemProperties = new Properties();
    private static Wizard wizard;
    static CaGeListener listener = new CaGeListener();
    private static int generators;
    static public int lastGeneratorChoice = -1;
    static public String[] generator;
    static AbstractButton[] generatorButton;
    static boolean rememberPanels;
    static Map<String, GeneratorPanel> generatorPanels;
    static Map<String, OutputPanel> outputPanels;
    private static FoldnetThread foldnetThread;
    // private static CaGeFoldnetDialog foldnetDialog;
    private static BackgroundWindow backgroundWindow;
    private static final List<EmbeddingTypeFactory> embeddingTypeFactories = new ArrayList<>();


    static {
        boolean nativesAvailableValue = false, expertModeValue = false, debugModeValue = false;
        String osNameValue = null;
        try {
            osNameValue = SysInfo.get("os.name");

            // Get configuration
            try {
                InputStream configInput =
                        ClassLoader.getSystemResourceAsStream(configFile);
                if (configInput == null) {
                    System.err.println("CaGe: can't find configuration file '" + configFile + "'");
                    System.exit(1);
                }
                config.load(configInput);
            } catch (Exception e) {
                Logger.getLogger(CaGe.class.getName()).log(Level.SEVERE,
                        "Error while loading configuration file", e);
                System.exit(1);
            }

            // prepare config substitution mechanism
            substitutedNames = new HashMap<>();
            String installDir = System.getProperty(installDirProperty);
            if (installDir == null) {
                installDir = System.getProperty("user.dir");
            } else {
                installDir = Systoolbox.makeAbsolutePath(installDir);
            }
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
            while (names.hasMoreElements()) {
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

            debugModeValue = Systoolbox.parseBoolean(config.getProperty("CaGe.DebugMode"), false);

            graphNoDigits =
                    Integer.parseInt(config.getProperty("CaGe.GraphNoDigits"));

        } catch (Throwable t) {
            Logger.getLogger(CaGe.class.getName()).log(Level.SEVERE,
                    "Error while initialising CaGe", t);
            System.exit(1);
        }
        nativesAvailable = nativesAvailableValue;
        expertMode = expertModeValue;
        debugMode = debugModeValue;
        osName = osNameValue;

        // construct the list with embedding type factories
        List<String> embeddingTypeFactoryVector = Systoolbox.stringToVector(config.getProperty("CaGe.EmbeddingTypeFactory"));
        for (int i = 0; i < embeddingTypeFactoryVector.size(); i++) {
            try {
                Object o = Class.forName(embeddingTypeFactoryVector.get(i).toString()).newInstance();
                if(o instanceof EmbeddingTypeFactory)
                    embeddingTypeFactories.add((EmbeddingTypeFactory)o);
            } catch (ClassNotFoundException | InstantiationException | IllegalAccessException ex) {
                Logger.getLogger(CaGe.class.getName()).log(Level.SEVERE,
                        "Error while trying to load EmbeddingTypeFactory", ex);
            }
        }

    }

    // perform configuration substitution on a name from CaGe.ini
    // (substitution syntax is described in that file)
    static String substituteConfigValue(String name) {
        return substituteConfigValue(name, false);
    }

    static String substituteConfigValue(String name, boolean throwStackOverflow) {
        String value = config.getProperty(name);
        if (substitutedNames.containsKey(name)) {
            return value;
        }
        String[] substStartStrings = {"${", "$[", "$("};
        String[] substEndStrings = {"}", "]", ")"};
        int pos1, pos2 = -1;
        while ((pos1 = Systoolbox.firstIndexOf(value, substStartStrings, pos2 + 1)) >= 0) {
            int index = Systoolbox.foundSubstringIndex();
            // if there is another $ before the substitution marker,
            // strip one if the $s and leave everything else unchanged
            if (pos1 - 1 > pos2 && value.charAt(pos1 - 1) == '$') {
                value = value.substring(0, pos1) + value.substring(pos1 + 1);
                pos2 = pos1;
            // ${name} is substituted by a value
            } else if ((pos2 = value.indexOf(substEndStrings[index], pos1)) >= 0) {
                String substName = value.substring(pos1 + 2, pos2);
                String substValue = null;
                try {
                    substValue =
                            index == 0 ? Systoolbox.getenv(substName) : index == 1 ? getSystemProperty(substName) : index == 2 ? substituteConfigValue(substName, true) : null;
                } catch (StackOverflowError er) {
                    if (throwStackOverflow) {
                        throw er;
                    }
                    System.err.println("CaGe: recursive definition in " + configFile);
                    System.err.println("- problem found starting with '" + name + "'");
                    System.exit(1);
                }
                if (substValue == null) {
                    substValue = "";
                }
                value = value.substring(0, pos1) + substValue + value.substring(pos2 + 1);
                pos2 += substValue.length() - substName.length();
                pos2 -= substStartStrings[index].length() + substEndStrings[index].length();
            }
        }
        config.put(name, value);
        substitutedNames.put(name, "");
        return value;
    }

    public static String getCaGeProperty(String name) {
        return config.getProperty(name);
    }

    public static String getCaGeProperty(String name, String defaultValue) {
        return config.getProperty(name, defaultValue);
    }

    public static int getCaGePropertyAsInt(String name, int defaultValue) {
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

    public static float getCaGePropertyAsFloat(String name, float defaultValue) {
        float floatValue = defaultValue;
        try {
            String stringValue = config.getProperty(name);
            floatValue = Float.parseFloat(stringValue);
        } catch (NullPointerException e) {
            System.err.println("Property '" + name + "' not present in .ini file");
        } catch (NumberFormatException e) {
            System.err.println("Property '" + name + "' in .ini file is not a number");
        }
        return floatValue;
    }

    public static boolean getCaGePropertyAsBoolean(String name,
            boolean defaultValue) {
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

    public static Color getCaGePropertyAsColor(String name,
            Color defaultValue) {
        Color colorValue = defaultValue;
        try {
            String stringValue = config.getProperty(name);
            String sArray[] = stringValue.split(" ");
            if(sArray.length == 3){
                colorValue = new Color(Float.parseFloat(sArray[0]),
                        Float.parseFloat(sArray[1]),
                        Float.parseFloat(sArray[2]));
            } else {
                Debug.print("Property '" + name + "' is not formatted as a color");
            }
        } catch (NullPointerException e) {
            System.err.println("Property '" + name + "' not present in .ini file");
        } catch (Exception e) {
            Debug.print("Property '" + name + "' is not formatted as a color");
            Debug.reportException(e);
        }
        return colorValue;
    }

    public static String getSystemProperty(String name) {
        return systemProperties.getProperty(name, System.getProperty(name));
    }

    public static String installDirectory() {
        return getSystemProperty(installDirProperty);
    }

    public static Wizard wizard() {
        return wizard;
    }

    public static WizardStage getWizardStage() {
        return wizard().getStage();
    }

    public static Window getWizardWindow() {
        return wizard().getWindow();
    }

    public static FoldnetThread foldnetThread() {
        return foldnetThread;
    }

    public static BackgroundWindow backgroundWindow() {
        return backgroundWindow;
    }

    private CaGe() {
        // set a few static variables
        rememberPanels = Systoolbox.parseBoolean(config.getProperty("CaGe.RememberSettings"), false);
        if (rememberPanels) {
            generatorPanels = new HashMap<>();
            outputPanels = new HashMap<>();
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
        List<String> generatorVector = Systoolbox.stringToVector(config.getProperty("CaGe.Generators"));
        generators = generatorVector.size();
        generatorButton = new AbstractButton[generators];
        generator = new String[generators];
        for (int i = 0; i < generators; ++i) {
            generator[i] = generatorVector.get(i);
            generatorButton[i] = new JButton(config.getProperty(generator[i] + ".Title"));
            generatorButton[i].setBorder(BorderFactory.createEmptyBorder(5, 10, 5, 10));
            generatorButton[i].setActionCommand(Integer.toString(i));
            if (i < 10) {
                generatorButton[i].setMnemonic(KeyEvent.VK_0 + (i + 1) % 10);
            }
            generatorButton[i].addActionListener(generatorChoiceListener);
            PushButtonDecoration.decorate(generatorButton[i]);
            generatorsPanel.add(generatorButton[i]);
        }
        lc.anchor = GridBagConstraints.CENTER;
        lc.insets = new Insets(0, 20, 30, 20);
        lc.gridy = y++;
        titlePanel.add(generatorsPanel, lc);

        // start a wizard with this panel
        wizard = new Wizard(title, Toolkit.getDefaultToolkit().getImage("img/logo.png"));
        wizard.nextStage(titlePanel, this, null, null, null, null, Wizard.EXIT);
    }

    @Override
    public void actionPerformed(ActionEvent e) {
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

    /**
     * Returns the number of <code>EmbeddingTypeFactory</code> instances that
     * are registered.
     *
     * @return the number of <code>EmbeddingTypeFactory</code> instances that
     * are registered.
     */
    public static int getNumberOfEmbeddingTypeFactories(){
        return embeddingTypeFactories.size();
    }

    /**
     * Returns the <tt>i</tt>th <code>EmbeddingTypeFactory</code>
     * @param i The index of the requested <code>EmbeddingTypeFactory</code>
     * @return the <tt>i</tt>th <code>EmbeddingTypeFactory</code>
     * @throws IndexOutOfBoundsException when i $lt; 0 or i is larger than the
     * number of <code>EmbeddingTypeFactory</code> instances.
     */
    public static EmbeddingTypeFactory getEmbeddingTypeFactory(int i){
        return embeddingTypeFactories.get(i);
    }

    public static void exit() {
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
    public static void loadNativeLibrary(String libName) {
        System.load(
                (String) config.getProperty("CaGe.Native.FullLibDir") + libName);
    }

    public static void main(String[] args) {
        new CaGe();
    }
}
