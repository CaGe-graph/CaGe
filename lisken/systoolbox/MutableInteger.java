package lisken.systoolbox;

public class MutableInteger implements Comparable {

    private int value;

    public MutableInteger(int n) {
        value = n;
    }

    @Override
    public int compareTo(Object o) {
        return value - ((MutableInteger) o).intValue();
    }

    public int intValue() {
        return value;
    }

    public void setValue(int n) {
        value = n;
    }

    @Override
    public boolean equals(Object o) {
        if (o instanceof MutableInteger) {
            return value == ((MutableInteger) o).intValue();
        } else if (o instanceof Integer) {
            return value == ((Integer) o).intValue();
        } else {
            return false;
        }
    }

    @Override
    public String toString() {
        return "lisken.systoolbox.Integer2(" + value + ")";
    }

    @Override
    public int hashCode() {
        return value;
    }
}

