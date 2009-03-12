package cage;

public interface CaGeOutlet {

    void setGeneratorInfo(GeneratorInfo generatorInfo);

    void setDimension(int d);

    int getDimension();

    void outputResult(CaGeResult result);

    void stop();
}
