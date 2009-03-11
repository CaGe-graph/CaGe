package cage;

import javax.swing.JPanel;

public abstract class GeneratorPanel extends JPanel {

    public abstract GeneratorInfo getGeneratorInfo();

    public abstract void showing();
}
