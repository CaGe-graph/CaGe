package cage;

import java.util.Arrays;
import java.util.List;
import javax.swing.AbstractButton;

public class Utils {

    public static void addIfSelected(List<String> l, AbstractButton button, String string) {
        addIfSelected(l, button, new String[]{string});
    }

    public static void addIfSelected(List<String> l, AbstractButton button, String[] strings) {
        if (button.isSelected()) {
            l.addAll(Arrays.asList(strings));
        }
    }
}

