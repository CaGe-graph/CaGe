package lisken.systoolbox;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.PrintWriter;
import java.io.StreamTokenizer;
import java.io.StringReader;
import java.io.StringWriter;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.List;

import util.SysInfo;

/**
 * Utility class that combines several static methods for system operations
 * such as getting the environment, file operations and other minor utility
 * methods.
 */
public class Systoolbox {

    private Systoolbox(){
        //private constructor
    }

    private static native byte[] nGetEnv(byte[] name);

    public static String getenv(String name) {
        byte[] value = null;
        try {
            value = nGetEnv(name.getBytes());
        } catch (Throwable t) {
        }
        return value == null ? null : new String(value);
    }

    public static String getenv(String name, String encoding)
            throws UnsupportedEncodingException {
        byte[] value = null;
        try {
            value = nGetEnv(name.getBytes(encoding));
        } catch (UnsupportedEncodingException ex) {
            throw ex;
        } catch (Throwable t) {
        }
        return value == null ? null : new String(value, encoding);
    }
    static private String osName = SysInfo.get("os.name");

    public static String makeAbsolutePath(String path) {
        if (osName.equals("Windows")) {
            if (path.length() >= 2 && path.charAt(1) == ':' && Character.toUpperCase(path.charAt(0)) >= 'A' && Character.toUpperCase(path.charAt(0)) <= 'Z') {
                return path;
            } else if (path.startsWith(File.separator)) {
                return System.getProperty("user.dir").substring(0, 2) + path;
            }
        } else if (path.startsWith(File.separator)) {
            return path;
        }
        return System.getProperty("user.dir") + File.separator + path;
    }

    public static String join(String[] stringArray, String separator) {
        int i;
        String result = "", sep = "";
        for (i = 0; i < stringArray.length; ++i) {
            if (stringArray[i] == null) {
                continue;
            }
            result = result + sep + stringArray[i];
            sep = separator;
        }
        return result;
    }

    public static String multiply(String s, int n) {
        String r = "", a = s;
        for (int k = n; k > 0; k >>= 1) {
            if ((k & 1) != 0) {
                r += a;
            }
            a += a;
        }
        return r;
    }

    public static String replace(String string, String oldString, String newString) {
        int o = oldString.length();
        if (o == 0) {
            return string;
        }
        String result = string;
        int i = 0, j = 0, s = string.length(), d = newString.length() - o;
        while (i < s) {
            if (result.charAt(i++) == oldString.charAt(j++)) {
                if (j == o) {
                    result = result.substring(0, i - o) + newString + result.substring(i);
                    s += d;
                    i += d;
                    j = 0;
                }
            } else {
                j = 0;
            }
        }
        return result;
    }
    static int foundSubstringIndex;

    public static int firstIndexOf(String string, String[] substringArray, int fromIndex) {
        char[] firstChar = new char[substringArray.length];
        for (int i = 0; i < substringArray.length; ++i) {
            try {
                firstChar[i] = substringArray[i].charAt(0);
            } catch (StringIndexOutOfBoundsException ex) {
                foundSubstringIndex = i;
                return fromIndex;
            }
        }
        int length = string.length();
        for (int index = fromIndex; index < length; ++index) {
            for (int i = 0; i < substringArray.length; ++i) {
                if (string.charAt(index) == firstChar[i]) {
                    int subIndex, subLength = substringArray[i].length();
                    if (index + subLength > length) {
                        continue;
                    }
                    for (subIndex = 1; subIndex < subLength; ++subIndex) {
                        if (string.charAt(index + subIndex) != substringArray[i].charAt(subIndex)) {
                            break;
                        }
                    }
                    if (subIndex == subLength) {
                        foundSubstringIndex = i;
                        return index;
                    }
                }
            }
        }
        foundSubstringIndex = -1;
        return -1;
    }

    public static int foundSubstringIndex() {
        return foundSubstringIndex;
    }

    public static boolean parseBoolean(String s, boolean defaultValue) {
        if (s == null) {
            return defaultValue;
        }
        String string = s.trim();
        if (defaultValue) {
            return (string.equals("1") || string.equalsIgnoreCase("true") || string.equalsIgnoreCase("yes"));
        } else {
            return !(string.equals("0") || string.equalsIgnoreCase("false") || string.equalsIgnoreCase("no"));
        }
    }

    public static List<String> stringToVector(String string) {
        return stringToVector(string, new SeparatorIndicator());
    }

    private static List<String> stringToVector(String string, SeparatorIndicator s) {
        if (string == null) {
            return null;
        }
        int pos, n = string.length(), start = -1;
        List<String> result = new ArrayList<>();
        for (pos = 0; pos < n; ++pos) {
            if (s.isSeparator(string.charAt(pos))) {
                if (start >= 0) {
                    result.add(string.substring(start, pos));
                    start = -1;
                }
            } else if (start < 0) {
                start = pos;
            }
        }
        if (start >= 0) {
            result.add(string.substring(start, pos));
        }
        return result;
    }

    public static String[] stringToArray(String string) {
        return stringToArray(string, new SeparatorIndicator());
    }

    private static String[] stringToArray(String string, SeparatorIndicator s) {
        return stringToVector(string, s).toArray(new String[0]);
    }

    public static String makeCmdLine(String[][] cmd) {
        StringBuffer result = new StringBuffer();
        String separator = "", quote = "'";
        for (int i = 0; i < cmd.length; ++i) {
            result.append(separator);
            separator = "";
            for (int j = 0; j < cmd[i].length; ++j) {
                String arg = cmd[i][j];
                result.append(separator);
                if (arg.indexOf(' ') > 0) {
                    result.append(quote);
                    result.append(arg);
                    result.append(quote);
                } else {
                    result.append(arg);
                }
                separator = " ";
            }
            separator = " | ";
        }
        return result.toString();
    }

    public static String[][] parseCmdLine(String cmdLine) {
        String pipedCmdLine = cmdLine.replace('|', '\n') + "\n";
        StreamTokenizer t = new StreamTokenizer(new StringReader(pipedCmdLine));
        t.resetSyntax();
        t.whitespaceChars('\u0000', '\u0020');
        t.wordChars('\u0021', '\uffff');
        t.quoteChar('"');
        t.quoteChar('\'');
        t.eolIsSignificant(true);
        List<String> cmd = new ArrayList<>();
        List<List<String>> pipe = new ArrayList<>();
        try {
            while (t.nextToken() != StreamTokenizer.TT_EOF) {
                if (t.ttype == StreamTokenizer.TT_EOL) {
                    if (cmd.size() > 0) {
                        pipe.add(cmd);
                        cmd = new ArrayList<>();
                    }
                } else {
                    cmd.add(t.sval);
                }
            }
        } catch (IOException e) {
        }
        int pipeSize = pipe.size();
        String[][] result = new String[pipeSize][];
        for (int i = 0; i < pipeSize; ++i) {
            cmd = pipe.get(i);
            result[i] = cmd.toArray(new String[cmd.size()]);
        }
        return result;
    }

    public static BufferedFDOutputStream createOutputStream(String cmdOrFilename)
            throws Exception {
        return createOutputStream(cmdOrFilename, null, false);
    }

    public static BufferedFDOutputStream createOutputStream(String cmdOrFilename, String rootDir)
            throws Exception {
        return createOutputStream(cmdOrFilename, rootDir, false);
    }

    public static BufferedFDOutputStream createOutputStream(String cmdOrFilename, String rootDir, boolean append)
            throws Exception {
        if (cmdOrFilename.trim().startsWith("|")) {
            Pipe pipe = new Pipe(parseCmdLine(cmdOrFilename));
            pipe.setRunDir(rootDir);
            pipe.start();
            return pipe.getOutputStream();
        } else {
            String prefix = "";
            if (rootDir != null && rootDir.length() > 0) {
                if (!cmdOrFilename.startsWith(File.separator)) {
                    prefix = rootDir + File.separator;
                }
            }
            String filename = prefix + cmdOrFilename;
            return new BufferedFDOutputStream(filename, append);
        }
    }

    public static Object[] stringsToBytes(Object[] strings) {
        int n = strings.length;
        Object result[] = new Object[n];
        for (int i = 0; i < n; ++i) {
            Object o = strings[i];
            if (o instanceof String) {
                result[i] = ((String) o).getBytes();
            } else {
                result[i] = stringsToBytes((Object[]) o);
            }
        }
        return result;
    }

    public static void printBytes(Object[] bytes) {
        printBytes(bytes, 0);
    }

    static void printBytes(Object[] bytes, int indent) {
        int n = bytes.length;
        for (int i = 0; i < n; ++i) {
            Object o = bytes[i];
            if (o instanceof byte[]) {
                System.out.print(multiply(" ", indent));
                System.out.println(new String((byte[]) o));
            } else {
                printBytes((Object[]) o, indent + 2);
            }
        }
    }

    public static boolean canCreat(File file) {
        File testFile = file.exists() ? file : new File(new File(file.getAbsolutePath()).getParent());
        return testFile.canWrite();
    }

    public static String getStackTrace(Exception e) {
        StringWriter traceWriter = new StringWriter();
        e.printStackTrace(new PrintWriter(traceWriter));
        return traceWriter.toString();
    }

    public static String getFileContent(String filename) {
        return getFileContent(filename, false);
    }

    public static String getFileContent(String filename, boolean delete) {
        File file = null;
        InputStream fileInput = null;
        StringWriter content = new StringWriter();
        try {
            file = new File(filename);
            fileInput = new BufferedInputStream(new FileInputStream(file));
            int c;
            while ((c = fileInput.read()) >= 0) {
                content.write(c);
            }
        } catch (IOException ex) {
        }
        try {
            fileInput.close();
        } catch (Exception ex) {
        }
        if (delete && file != null) {
            file.delete();
        }
        return content.toString();
    }

    public static void lowerPriority(Thread thread, int offset) {
        try {
            thread.setPriority(Thread.currentThread().getPriority() - offset);
        } catch (IllegalArgumentException ex) {
            thread.setPriority(Thread.MIN_PRIORITY);
        }
    }

    private static class SeparatorIndicator {

        public boolean isSeparator(char c) {
            return Character.isWhitespace(c);
        }
    }
}
