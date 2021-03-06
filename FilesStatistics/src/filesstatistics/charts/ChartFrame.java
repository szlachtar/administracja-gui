/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package filesstatistics.charts;

/**
 *
 * @author n
 */
import java.awt.Dimension;
import java.util.Map;
import org.jfree.chart.ChartPanel;
import org.jfree.ui.ApplicationFrame;

/**
 * A simple demonstration application showing how to create a bar chart.
 */
public class ChartFrame extends ApplicationFrame {

    /**
     * Creates a new demo instance.
     *
     * @param title the frame title.
     */
    public ChartFrame(String fileName,String operation,Map<String,Map<String,Object>> stats) {
        super(fileName + " - " + operation);
        ChartPanel chartPanel = new ChartPanel(new SimpleChart(stats, fileName, operation).createChart(), false);
        chartPanel.setPreferredSize(new Dimension(500, 270));
        setContentPane(chartPanel);
    }
}
