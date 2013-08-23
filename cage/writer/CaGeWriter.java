package cage.writer;

import cage.CaGeOutlet;
import cage.GeneratorInfo;

import java.io.IOException;
import java.io.OutputStream;

public abstract class CaGeWriter implements CaGeOutlet {

    public abstract String getFormatName();

    public abstract String getFileExtension();

    public boolean usesInfo() {
        return false;
    }
    int dimension;
    OutputStream out;
    GeneratorInfo generatorInfo;
    IOException lastException;

    @Override
    public void setDimension(int dimension) {
        this.dimension = dimension;
    }

    @Override
    public int getDimension() {
        return dimension;
    }

    @Override
    public void setGeneratorInfo(GeneratorInfo generatorInfo) {
        this.generatorInfo = generatorInfo;
    }

    public void setOutputStream(OutputStream out) {
        this.out = out;
    }

    @Override
    public void stop() {
        lastException = null;
        if (out == null) {
            return;
        }
        try {
            out.close();
        } catch (IOException ex) {
            lastException = ex;
        }
    }

    boolean out(String output) {
        return out(output.getBytes());
    }

    boolean out(byte[] output) {
        lastException = null;
        try {
            out.write(output);
        } catch (IOException ex) {
            lastException = ex;
        }
        return lastException != null;
    }

    public boolean wasIOException() {
        return lastException != null;
    }

    public IOException lastIOException() {
        return lastException;
    }

    public void throwLastIOException()
            throws IOException {
        if (lastException != null) {
            throw lastException;
        }
    }

    public void flush() {
        lastException = null;
        if (out == null) {
            return;
        }
        try {
            out.flush();
        } catch (IOException ex) {
            lastException = ex;
        }
    }
}
