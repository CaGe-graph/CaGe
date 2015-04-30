/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package cage.writer.scad;

import cage.CaGeResult;
import cage.EdgeIterator;

/**
 * Super class for all ScadTypes that are based on vertices and edges.
 * @author nvcleemp
 */
public abstract class VertexEdgeType implements ScadType {
    
    private int minAngle; //the minimum angle of a fragment
    private int minSize;  //the minimum size of a fragment times 100

    @Override
    public String processResult(CaGeResult result) {
        StringBuilder sb = new StringBuilder();
        sb
            .append("module vertex(p, $fa=").append(minAngle)
            .append(", $fs=").append(minSize*0.01).append(") {\n")
            .append("    translate(p) sphere(r=")
            .append(getVertexRadius()).append(");\n")
            .append("}\n\n")
            .append("module edge(p1, p2, $fa=").append(minAngle)
            .append(", $fs=").append(minSize*0.01).append(") {\n")
            .append("    edge_length = sqrt(pow(p2[0]-p1[0], 2) + pow(p2[1]-p1[1], 2) + pow(p2[2]-p1[2], 2));\n")
            .append("    b = acos((p2[2]-p1[2])/edge_length);\n")
            .append("    c = (p2[0]==p1[0]) ? sign(p2[1]-p1[1])*90 : ((p2[0]>p1[0]) ? atan((p2[1]-p1[1])/(p2[0]-p1[0])) : atan((p2[1]-p1[1])/(p2[0]-p1[0]))+180);\n")
            .append("    translate(p1) rotate([0, b, c]) cylinder(h=edge_length, r=")
            .append(getEdgeRadius()).append(");\n")
            .append("}\n\n");

        
        for (int i = 1; i <= result.getGraph().getSize(); i++) {
            sb.append("v").append(i).append(" = [")
                .append(result.getGraph().get3DCoordinates(i)[0]*10).append(", ")
                .append(result.getGraph().get3DCoordinates(i)[1]*10).append(", ")
                .append(result.getGraph().get3DCoordinates(i)[2]*10).append("];\n");
        }
        
        sb.append("\n");
        
        for (int i = 1; i <= result.getGraph().getSize(); i++) {
            sb
                .append("vertex(v").append(i).append(");\n");
        }
        
        sb.append("\n");
        
        for (int i = 1; i <= result.getGraph().getSize(); i++) {
            EdgeIterator it = result.getGraph().getEdgeIterator(i);
            while(it.hasNext()){
                int n = it.nextEdge();
                if(i < n){
                    sb.append("edge(v").append(i).append(", v").append(n).append(");\n");
                }
            }
        }
        
        return sb.toString();
    }

    @Override
    public boolean hasResolution() {
        return true;
    }

    @Override
    public void setMinimumAngle(int minAngle) {
        this.minAngle = minAngle;
    }

    @Override
    public void setMinimumSize(int minSize) {
        this.minSize = minSize;
    }
    
    abstract protected int getVertexRadius();
    
    abstract protected int getEdgeRadius();
}
